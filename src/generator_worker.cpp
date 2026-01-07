// generator_worker.cpp - Background Generator Worker Implementation

#include "generator_worker.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

GeneratorWorker::GeneratorWorker(QObject* parent)
    : QObject(parent)
    , generator_process_(nullptr)
    , current_instance_(0)
    , cancel_requested_(false) {
}

GeneratorWorker::~GeneratorWorker() {
    if (generator_process_) {
        generator_process_->kill();
        generator_process_->waitForFinished(1000);
        delete generator_process_;
    }
}

void GeneratorWorker::SetConfig(const GeneratorConfig& config) {
    config_ = config;
}

QString GeneratorWorker::GetGeneratorExePath() const {
    // Look for the generator executable relative to GUI location
    QString app_dir = QCoreApplication::applicationDirPath();

    // Try common locations
    QStringList candidates = {
        app_dir + "/CS-2D-Data.exe",
        "D:/YM-Code/CS-2D-Data/build/release/Release/CS-2D-Data.exe",
        "D:/YM-Code/CS-2D-Data/build/release/Debug/CS-2D-Data.exe"
    };

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    return QString();
}

QStringList GeneratorWorker::BuildCommandArgs() const {
    QStringList args;

    // Use manual mode with all parameters
    args << "--manual";

    // Scale parameters
    args << "--num-types" << QString::number(config_.num_types);
    args << "-W" << QString::number(config_.stock_width);
    args << "-L" << QString::number(config_.stock_length);

    // Size parameters
    args << "--min-size-ratio" << QString::number(config_.min_size_ratio, 'f', 3);
    args << "--max-size-ratio" << QString::number(config_.max_size_ratio, 'f', 3);
    args << "--size-cv" << QString::number(config_.size_cv, 'f', 2);

    // Demand parameters
    args << "--min-demand" << QString::number(config_.min_demand);
    args << "--max-demand" << QString::number(config_.max_demand);
    args << "--demand-skew" << QString::number(config_.demand_skew, 'f', 2);

    // Advanced options
    if (config_.prime_offset) {
        args << "--prime-offset";
    }

    // Strategy
    args << "--strategy" << QString::number(config_.strategy);

    // Common settings
    if (config_.seed > 0) {
        args << "-s" << QString::number(config_.seed);
    }
    args << "-n" << QString::number(config_.count);
    args << "-o" << config_.output_path;

    return args;
}

void GeneratorWorker::RunGeneration() {
    cancel_requested_ = false;
    generated_files_.clear();
    current_instance_ = 0;

    QString exe_path = GetGeneratorExePath();
    if (exe_path.isEmpty()) {
        emit GenerationFinished(false,
            QString::fromUtf8("未找到生成器可执行文件 (CS-2D-Data.exe)"),
            QStringList());
        return;
    }

    emit GenerationStarted(config_.count);
    emit LogMessage(QString("Generator: %1").arg(exe_path));
    emit LogMessage(QString("Output: %1").arg(config_.output_path));
    emit LogMessage(QString("Config: types=%1 stock=%2x%3 size_ratio=[%4,%5] demand=[%6,%7] count=%8")
        .arg(config_.num_types)
        .arg(config_.stock_width).arg(config_.stock_length)
        .arg(config_.min_size_ratio, 0, 'f', 2).arg(config_.max_size_ratio, 0, 'f', 2)
        .arg(config_.min_demand).arg(config_.max_demand)
        .arg(config_.count));

    // Build command arguments
    QStringList args = BuildCommandArgs();
    emit LogMessage(QString("Args: %1").arg(args.join(" ")));

    generator_process_ = new QProcess(this);

    connect(generator_process_, &QProcess::readyReadStandardOutput,
            this, &GeneratorWorker::OnProcessOutput);
    connect(generator_process_, &QProcess::readyReadStandardError,
            this, &GeneratorWorker::OnProcessOutput);
    connect(generator_process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GeneratorWorker::OnProcessFinished);

    // Set working directory
    generator_process_->setWorkingDirectory("D:/YM-Code/CS-2D-Data");
    generator_process_->start(exe_path, args);

    if (!generator_process_->waitForStarted(5000)) {
        emit GenerationFinished(false,
            QString::fromUtf8("启动生成器进程失败"),
            QStringList());
        delete generator_process_;
        generator_process_ = nullptr;
        return;
    }
}

void GeneratorWorker::RequestCancel() {
    cancel_requested_ = true;
    if (generator_process_ && generator_process_->state() == QProcess::Running) {
        generator_process_->kill();
    }
}

void GeneratorWorker::OnProcessOutput() {
    if (!generator_process_) return;

    QString output = generator_process_->readAllStandardOutput();
    output += generator_process_->readAllStandardError();

    // Remove ANSI color codes
    output.remove(QRegularExpression("\\x1B\\[[0-9;]*m"));

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        emit LogMessage(line.trimmed());

        // Check for file output patterns
        // Pattern: "Generated: filename.csv" or "Saved: path/filename.csv"
        if (line.contains(".csv")) {
            QRegularExpression rx("([\\w/\\\\-]+\\.csv)");
            QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch()) {
                QString filename = match.captured(1);
                generated_files_.append(filename);
                emit InstanceGenerated(generated_files_.size(), filename);
            }
        }
    }
}

void GeneratorWorker::OnProcessFinished(int exitCode, QProcess::ExitStatus status) {
    bool success = (exitCode == 0 && status == QProcess::NormalExit && !cancel_requested_);

    QString message;
    if (cancel_requested_) {
        message = QString::fromUtf8("生成已取消");
    } else if (success) {
        message = QString::fromUtf8("已生成 %1 个算例").arg(generated_files_.size());
    } else {
        message = QString::fromUtf8("生成失败 (退出码: %1)").arg(exitCode);
    }

    emit GenerationFinished(success, message, generated_files_);

    delete generator_process_;
    generator_process_ = nullptr;
}
