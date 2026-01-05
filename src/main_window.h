// main_window.h - 主窗口声明

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QThread>
#include <QString>

class ParameterWidget;
class ResultsWidget;
class LogWidget;
class CuttingViewDialog;
class SolverWorker;
class QLineEdit;
class QLabel;
class QPushButton;
class QProgressBar;
class QGroupBox;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void StartSolver();

private slots:
    void OnBrowseFile();
    void OnStartSolve();
    void OnCancelSolve();
    void OnExportJson();
    void OnViewCutting();

    // Worker 信号槽
    void OnDataLoaded(int numItemTypes, int stockWidth, int stockLength, int totalDemand);
    void OnStageStarted(int stage, const QString& name);
    void OnStageProgress(int stage, int percent, double elapsed);
    void OnStageCompleted(int stage, double value, double runtime);
    void OnSolveFinished(bool success, const QString& message);
    void OnLogMessage(const QString& message);
    void OnSolutionReady(const QString& jsonPath);
    void OnResultsReady(int optimalValue, double rootLB, double gap,
                        int nodeCount, double utilization);

private:
    void SetupUi();
    void SetupMenuBar();
    void SetupConnections();
    void UpdateUiState(bool is_running);
    void ResetProgress();

    // 文件选择
    QGroupBox* file_group_;
    QPushButton* browse_button_;
    QLineEdit* file_path_edit_;
    QLabel* file_info_label_;

    // 参数设置
    ParameterWidget* param_widget_;

    // 控制按钮
    QPushButton* start_button_;
    QPushButton* cancel_button_;
    QLabel* status_label_;

    // 进度显示 (5个阶段)
    QGroupBox* progress_group_;
    QLabel* stage_labels_[5];
    QProgressBar* progress_bars_[5];
    QLabel* time_labels_[5];

    // 结果和日志
    ResultsWidget* results_widget_;
    LogWidget* log_widget_;

    // 切割方案可视化对话框
    CuttingViewDialog* cutting_view_dialog_;

    // 底部按钮
    QPushButton* export_json_button_;
    QPushButton* view_cutting_button_;
    QLabel* total_time_label_;

    // Worker 线程
    QThread* worker_thread_;
    SolverWorker* solver_worker_;

    // 状态
    bool is_running_;
    QString current_file_path_;
    QString current_json_path_;
    double total_runtime_;
};

#endif  // MAIN_WINDOW_H_
