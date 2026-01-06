// solver_worker.h - Background Solver Worker (Subprocess)

#ifndef SOLVER_WORKER_H_
#define SOLVER_WORKER_H_

#include <QObject>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <atomic>

class SolverWorker : public QObject {
    Q_OBJECT

public:
    explicit SolverWorker(QObject* parent = nullptr);
    ~SolverWorker() override;

    void SetDataPath(const QString& path);
    void SetTimeLimit(int seconds);
    void SetSP1Method(int method);
    void SetSP2Method(int method);

public slots:
    void RunSolver();
    void RequestCancel();

signals:
    // Data loaded
    void DataLoaded(int numItemTypes, int stockWidth, int stockLength, int totalDemand);

    // Stage signals (0-4 correspond to 5 stages)
    // 0: Data load, 1: Heuristic, 2: Root CG, 3: Integer check, 4: Branch & Price
    void StageStarted(int stage, const QString& name);
    void StageCompleted(int stage, double value, double runtime);

    // Completion signals
    void SolveFinished(bool success, const QString& message);
    void LogMessage(const QString& message);

    // Solution ready (JSON file path)
    void SolutionReady(const QString& jsonPath);

    // Results signal
    void ResultsReady(int optimalValue, double rootLB, double gap,
                      int nodeCount, double utilization);

private slots:
    void OnProcessOutput();
    void OnProcessError();
    void OnProcessFinished(int exitCode, QProcess::ExitStatus status);
    void OnReadLogFile();

private:
    void ParseProgressLine(const QString& line);
    QString GetSolverExePath() const;
    QString GetLatestSolutionPath() const;
    void ParseResultsFromJson(const QString& jsonPath);

    QString data_path_;
    int time_limit_;
    int sp1_method_;
    int sp2_method_;

    QProcess* solver_process_;
    QTimer* log_reader_;
    QString log_file_path_;
    qint64 log_file_pos_;

    std::atomic<bool> cancel_requested_;

    // Stage timing
    int current_stage_;
    double stage_start_time_;
};

#endif  // SOLVER_WORKER_H_
