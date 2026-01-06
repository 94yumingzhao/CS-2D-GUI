// main_window.h - 主窗口声明

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QThread>
#include <QString>

struct GeneratorConfig;
class ParameterWidget;
class ResultsWidget;
class LogWidget;
class GeneratorWidget;
class CuttingViewWidget;
class SolverWorker;
class GeneratorWorker;
class QLineEdit;
class QLabel;
class QPushButton;
class QGroupBox;
class QTabWidget;
class QSpinBox;
class QComboBox;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void StartSolver();
    void StartGeneration();

private slots:
    // Solver tab
    void OnBrowseFile();
    void OnStartSolve();
    void OnCancelSolve();
    void OnExportJson();

    // Cutting view tab
    void OnLoadSolution();
    void OnPrevStock();
    void OnNextStock();
    void OnZoomChanged(int index);

    // Solver worker signals
    void OnDataLoaded(int numItemTypes, int stockWidth, int stockLength, int totalDemand);
    void OnSolveFinished(bool success, const QString& message);
    void OnSolverLogMessage(const QString& message);
    void OnSolutionReady(const QString& jsonPath);
    void OnResultsReady(int optimalValue, double rootLB, double gap,
                        int nodeCount, double utilization);

    // Generator widget signal
    void OnGenerateRequested(const GeneratorConfig& config);

    // Generator worker signals
    void OnGenerationStarted(int count);
    void OnInstanceGenerated(int index, const QString& filename);
    void OnGenerationFinished(bool success, const QString& message,
                              const QStringList& files);
    void OnGeneratorLogMessage(const QString& message);

private:
    void SetupUi();
    void SetupMenuBar();
    void SetupConnections();
    void UpdateSolverUiState(bool is_running);
    void UpdateCuttingNavigation();

    QWidget* CreateSolverTab();
    QWidget* CreateGeneratorTab();
    QWidget* CreateCuttingTab();

    // Tab widget
    QTabWidget* tab_widget_;

    // ========== Solver Tab ==========
    QGroupBox* file_group_;
    QPushButton* browse_button_;
    QLineEdit* file_path_edit_;
    QLabel* file_info_label_;

    ParameterWidget* param_widget_;

    QPushButton* start_button_;
    QPushButton* cancel_button_;
    QLabel* status_label_;

    ResultsWidget* results_widget_;
    QPushButton* export_json_button_;

    LogWidget* solver_log_widget_;

    // ========== Generator Tab ==========
    GeneratorWidget* generator_widget_;
    LogWidget* generator_log_widget_;

    // ========== Cutting View Tab ==========
    QPushButton* load_solution_button_;
    QLineEdit* solution_path_edit_;
    QPushButton* prev_stock_button_;
    QLabel* stock_nav_label_;
    QPushButton* next_stock_button_;
    QComboBox* zoom_combo_;
    CuttingViewWidget* cutting_view_widget_;

    // ========== Workers ==========
    QThread* solver_thread_;
    SolverWorker* solver_worker_;
    QThread* generator_thread_;
    GeneratorWorker* generator_worker_;

    // ========== State ==========
    bool is_running_;
    QString current_file_path_;
    QString current_json_path_;
    int current_stock_index_;
    int total_stock_count_;
};

#endif  // MAIN_WINDOW_H_
