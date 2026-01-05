// solver_worker.h - 后台求解线程

#ifndef SOLVER_WORKER_H_
#define SOLVER_WORKER_H_

#include <QObject>
#include <QString>
#include <atomic>
#include <chrono>

// 前向声明 (来自 CS-2D-BP-Arc)
struct ProblemParams;
struct ProblemData;
struct BPNode;

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
    // 数据加载完成
    void DataLoaded(int numItemTypes, int stockWidth, int stockLength, int totalDemand);

    // 阶段信号 (0-4 对应 5 个阶段)
    // 0: 数据读取, 1: 启发式, 2: 根节点CG, 3: 整数性检查, 4: 分支定价
    void StageStarted(int stage, const QString& name);
    void StageProgress(int stage, int percent, double elapsed);
    void StageCompleted(int stage, double value, double runtime);

    // 完成信号
    void SolveFinished(bool success, const QString& message);
    void LogMessage(const QString& message);

    // 解就绪 (传递 JSON 文件路径)
    void SolutionReady(const QString& jsonPath);

    // 结果信号
    void ResultsReady(int optimalValue, double rootLB, double gap,
                      int nodeCount, double utilization);

private:
    QString data_path_;
    int time_limit_;
    int sp1_method_;
    int sp2_method_;

    std::atomic<bool> cancel_requested_;

    // 求解器数据 (动态分配)
    ProblemParams* params_;
    ProblemData* data_;
    BPNode* root_node_;

    QString GetLatestSolutionPath() const;
};

#endif  // SOLVER_WORKER_H_
