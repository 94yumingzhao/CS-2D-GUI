// solver_worker.cpp - Background Solver Worker (Subprocess) Implementation

#include "solver_worker.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SolverWorker::SolverWorker(QObject* parent)
    : QObject(parent)
    , time_limit_(60)
    , sp1_method_(1)  // kArcFlow
    , sp2_method_(1)  // kArcFlow
    , solver_process_(nullptr)
    , log_reader_(nullptr)
    , log_file_pos_(0)
    , cancel_requested_(false)
    , current_stage_(-1)
    , stage_start_time_(0.0) {
}

SolverWorker::~SolverWorker() {
    if (solver_process_) {
        solver_process_->kill();
        solver_process_->waitForFinished(1000);
        delete solver_process_;
    }
    delete log_reader_;
}

void SolverWorker::SetDataPath(const QString& path) {
    data_path_ = path;
}

void SolverWorker::SetTimeLimit(int seconds) {
    time_limit_ = seconds;
}

void SolverWorker::SetSP1Method(int method) {
    sp1_method_ = method;
}

void SolverWorker::SetSP2Method(int method) {
    sp2_method_ = method;
}

QString SolverWorker::GetSolverExePath() const {
    QString app_dir = QCoreApplication::applicationDirPath();

    // Try relative paths from GUI build directory
    // GUI: D:/YM-Code/CS-2D-GUI/build/vs2022/bin/Release/
    // Solver: D:/YM-Code/CS-2D-BP-Arc/build/release/bin/Release/
    QStringList possible_paths = {
        app_dir + "/../../../../CS-2D-BP-Arc/build/release/bin/Release/CS-2D-BP-Arc.exe",
        app_dir + "/../../../CS-2D-BP-Arc/build/release/bin/Release/CS-2D-BP-Arc.exe",
        app_dir + "/../../CS-2D-BP-Arc/build/release/bin/Release/CS-2D-BP-Arc.exe",
        "D:/YM-Code/CS-2D-BP-Arc/build/release/bin/Release/CS-2D-BP-Arc.exe"
    };

    for (const QString& path : possible_paths) {
        QFileInfo fi(path);
        if (fi.exists()) {
            return fi.absoluteFilePath();
        }
    }

    return "D:/YM-Code/CS-2D-BP-Arc/build/release/bin/Release/CS-2D-BP-Arc.exe";
}

QString SolverWorker::GetLatestSolutionPath() const {
    QDir results_dir("results");
    if (!results_dir.exists()) return QString();

    QStringList filters;
    filters << "solution_*.json";
    QFileInfoList files = results_dir.entryInfoList(filters, QDir::Files, QDir::Time);

    if (files.isEmpty()) return QString();
    return files.first().absoluteFilePath();
}

void SolverWorker::RequestCancel() {
    cancel_requested_ = true;
    if (solver_process_ && solver_process_->state() != QProcess::NotRunning) {
        solver_process_->kill();
    }
}

void SolverWorker::RunSolver() {
    cancel_requested_ = false;
    current_stage_ = -1;
    stage_start_time_ = 0.0;

    QString exe_path = GetSolverExePath();
    QFileInfo exe_info(exe_path);

    if (!exe_info.exists()) {
        emit LogMessage(QString::fromUtf8("Error: Solver not found: %1").arg(exe_path));
        emit SolveFinished(false, QString::fromUtf8("Solver executable not found"));
        return;
    }

    emit LogMessage(QString::fromUtf8("Solver: %1").arg(exe_path));
    emit LogMessage(QString::fromUtf8("Data: %1").arg(data_path_));

    // Create results directory for solver output
    QDir().mkpath("results");
    QDir().mkpath("logs");

    // Build command line arguments
    QStringList args;
    args << "-f" << data_path_;
    if (time_limit_ > 0) {
        args << "-t" << QString::number(time_limit_);
    }

    emit LogMessage(QString::fromUtf8("Args: %1").arg(args.join(" ")));

    // Create and configure process
    if (solver_process_) {
        delete solver_process_;
    }
    solver_process_ = new QProcess(this);

    // Set working directory to solver's directory for output files
    solver_process_->setWorkingDirectory(exe_info.absolutePath());

    connect(solver_process_, &QProcess::readyReadStandardOutput,
            this, &SolverWorker::OnProcessOutput);
    connect(solver_process_, &QProcess::readyReadStandardError,
            this, &SolverWorker::OnProcessError);
    connect(solver_process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SolverWorker::OnProcessFinished);

    // Start log file reader
    log_file_path_ = exe_info.absolutePath() + "/logs";
    log_file_pos_ = 0;

    if (log_reader_) {
        log_reader_->stop();
        delete log_reader_;
    }
    log_reader_ = new QTimer(this);
    connect(log_reader_, &QTimer::timeout, this, &SolverWorker::OnReadLogFile);
    log_reader_->start(500);

    // Start the solver process
    solver_process_->start(exe_path, args);

    if (!solver_process_->waitForStarted(5000)) {
        emit LogMessage(QString::fromUtf8("Error: Failed to start solver process"));
        emit SolveFinished(false, QString::fromUtf8("Failed to start solver"));
        return;
    }

    emit LogMessage(QString::fromUtf8("Solver process started (PID: %1)")
                    .arg(solver_process_->processId()));

    // Emit initial stage
    emit StageStarted(0, QString::fromUtf8("Data Loading"));
}

void SolverWorker::OnProcessOutput() {
    // stdout is typically not used for progress in CS-2D-BP-Arc
    if (!solver_process_) return;

    QByteArray data = solver_process_->readAllStandardOutput();
    QString output = QString::fromUtf8(data);

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        emit LogMessage(QString::fromUtf8("[stdout] %1").arg(line.trimmed()));
    }
}

void SolverWorker::OnProcessError() {
    if (!solver_process_) return;

    QByteArray data = solver_process_->readAllStandardError();
    QString output = QString::fromUtf8(data);

    // Parse progress messages from stderr
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        ParseProgressLine(line.trimmed());
    }
}

void SolverWorker::OnProcessFinished(int exitCode, QProcess::ExitStatus status) {
    // Stop log reader
    if (log_reader_) {
        log_reader_->stop();
    }

    // Read any remaining log content
    OnReadLogFile();

    if (cancel_requested_) {
        emit SolveFinished(false, QString::fromUtf8("Cancelled by user"));
        return;
    }

    if (status == QProcess::CrashExit) {
        emit LogMessage(QString::fromUtf8("Solver process crashed"));
        emit SolveFinished(false, QString::fromUtf8("Solver crashed"));
        return;
    }

    if (exitCode != 0) {
        emit LogMessage(QString::fromUtf8("Solver exited with code %1").arg(exitCode));
        emit SolveFinished(false, QString::fromUtf8("Solver failed (exit code %1)").arg(exitCode));
        return;
    }

    // Find and parse the solution JSON
    QString solver_dir = QFileInfo(GetSolverExePath()).absolutePath();
    QDir results_dir(solver_dir + "/results");

    if (results_dir.exists()) {
        QStringList filters;
        filters << "solution_*.json";
        QFileInfoList files = results_dir.entryInfoList(filters, QDir::Files, QDir::Time);

        if (!files.isEmpty()) {
            QString json_path = files.first().absoluteFilePath();
            emit SolutionReady(json_path);
            emit LogMessage(QString::fromUtf8("Solution exported: %1").arg(json_path));
            ParseResultsFromJson(json_path);
        }
    }

    emit LogMessage(QString::fromUtf8("Solver completed successfully"));
    emit SolveFinished(true, QString::fromUtf8("Completed"));
}

void SolverWorker::OnReadLogFile() {
    // Find latest log file in logs directory
    QString solver_dir = QFileInfo(GetSolverExePath()).absolutePath();
    QDir logs_dir(solver_dir + "/logs");

    if (!logs_dir.exists()) return;

    QStringList filters;
    filters << "log_2DBP_Arc_*.log";
    QFileInfoList files = logs_dir.entryInfoList(filters, QDir::Files, QDir::Time);

    if (files.isEmpty()) return;

    QString log_path = files.first().absoluteFilePath();

    // Track if this is a new file
    static QString last_log_path;
    if (log_path != last_log_path) {
        last_log_path = log_path;
        log_file_pos_ = 0;
    }

    QFile file(log_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    file.seek(log_file_pos_);

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty()) {
            emit LogMessage(line);
        }
    }

    log_file_pos_ = file.pos();
    file.close();
}

void SolverWorker::ParseProgressLine(const QString& line) {
    // Parse PROGRESS output from CS-2D-BP-Arc stderr
    // Format: "[elapsed] message"
    // Examples:
    // "[  0.001s] Start | CS-2D-BP-Arc | inst_xxx.csv | time:60s"
    // "[  0.010s] Data | 5 types | stock:500x300"
    // "[  0.050s] CG   | converged LP=3.45 (fractional)"
    // "[ 15.234s] Done | optimal=4 Gap=0.0% nodes=12"

    emit LogMessage(line);

    // Extract elapsed time and message
    static QRegularExpression re_progress(R"(\[\s*(\d+\.?\d*)s?\]\s*(.+))");
    QRegularExpressionMatch match = re_progress.match(line);

    if (!match.hasMatch()) return;

    double elapsed = match.captured(1).toDouble();
    QString message = match.captured(2);

    // Parse different message types
    if (message.contains(QString::fromUtf8("Data")) || message.contains(QString::fromUtf8("数据"))) {
        // Data loaded
        static QRegularExpression re_data(R"((\d+)\s*(?:种子板|types).*?(\d+)\s*x\s*(\d+))");
        QRegularExpressionMatch data_match = re_data.match(message);
        if (data_match.hasMatch()) {
            int num_types = data_match.captured(1).toInt();
            int width = data_match.captured(2).toInt();
            int length = data_match.captured(3).toInt();
            emit DataLoaded(num_types, width, length, 0);
            emit StageCompleted(0, num_types, elapsed);
            emit StageStarted(1, QString::fromUtf8("Heuristic"));
        }
    } else if (message.contains("CG") || message.contains(QString::fromUtf8("列生成"))) {
        // Column generation progress
        if (message.contains("LP=") || message.contains(QString::fromUtf8("收敛"))) {
            static QRegularExpression re_lp(R"(LP=(\d+\.?\d*))");
            QRegularExpressionMatch lp_match = re_lp.match(message);
            if (lp_match.hasMatch()) {
                double lp_value = lp_match.captured(1).toDouble();
                emit StageCompleted(2, lp_value, elapsed);

                if (message.contains(QString::fromUtf8("分数")) || message.contains("fractional")) {
                    emit StageStarted(4, QString::fromUtf8("Branch & Price"));
                } else {
                    emit StageCompleted(3, 1.0, 0);
                    emit StageCompleted(4, 0.0, 0);
                }
            }
        }
    } else if (message.contains(QString::fromUtf8("完成")) || message.contains("Done")) {
        // Completed
        static QRegularExpression re_done(R"((?:最优|optimal)=(\d+).*?Gap=(\d+\.?\d*)%.*?nodes=(\d+))");
        QRegularExpressionMatch done_match = re_done.match(message);
        if (done_match.hasMatch()) {
            int optimal = done_match.captured(1).toInt();
            double gap = done_match.captured(2).toDouble() / 100.0;
            int nodes = done_match.captured(3).toInt();
            emit StageCompleted(4, optimal, elapsed);
        }
    } else if (message.contains(QString::fromUtf8("启发式")) || message.contains("Heuristic")) {
        emit StageCompleted(1, 0, elapsed);
        emit StageStarted(2, QString::fromUtf8("Root Node CG"));
    }
}

void SolverWorker::ParseResultsFromJson(const QString& jsonPath) {
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit LogMessage(QString::fromUtf8("Cannot open solution file: %1").arg(jsonPath));
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        emit LogMessage(QString::fromUtf8("JSON parse error: %1").arg(error.errorString()));
        return;
    }

    QJsonObject root = doc.object();

    // Parse summary
    if (root.contains("summary")) {
        QJsonObject summary = root["summary"].toObject();

        int num_plates = summary["num_plates"].toInt();
        double root_lb = summary["root_lb"].toDouble();
        double gap = summary["gap"].toDouble();
        int node_count = summary["node_count"].toInt(1);
        double utilization = summary["total_utilization"].toDouble();

        emit ResultsReady(num_plates, root_lb, gap, node_count, utilization);
    }
}
