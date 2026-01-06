// main_window.cpp - 主窗口实现

#include "main_window.h"
#include "parameter_widget.h"
#include "results_widget.h"
#include "log_widget.h"
#include "cutting_view_widget.h"
#include "solver_worker.h"
#include "generator_widget.h"
#include "generator_worker.h"
#include "difficulty_mapper.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , solver_thread_(nullptr)
    , solver_worker_(nullptr)
    , generator_thread_(nullptr)
    , generator_worker_(nullptr)
    , is_running_(false)
    , current_stock_index_(0)
    , total_stock_count_(0) {
    SetupUi();
    SetupMenuBar();
    SetupConnections();
    UpdateSolverUiState(false);

    setWindowTitle(QString::fromUtf8("二维下料问题求解器"));
    resize(1000, 700);
}

MainWindow::~MainWindow() {
    if (solver_thread_) {
        solver_thread_->quit();
        solver_thread_->wait();
    }
    if (generator_thread_) {
        generator_thread_->quit();
        generator_thread_->wait();
    }
}

void MainWindow::SetupUi() {
    auto* central = new QWidget(this);
    auto* main_layout = new QVBoxLayout(central);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // Tab widget for 3 tabs
    tab_widget_ = new QTabWidget(this);
    tab_widget_->addTab(CreateSolverTab(), QString::fromUtf8("求解器"));
    tab_widget_->addTab(CreateGeneratorTab(), QString::fromUtf8("算例生成"));
    tab_widget_->addTab(CreateCuttingTab(), QString::fromUtf8("切割方案"));
    main_layout->addWidget(tab_widget_);

    setCentralWidget(central);

    // Status bar
    statusBar()->showMessage(QString::fromUtf8("就绪"));
}

QWidget* MainWindow::CreateSolverTab() {
    auto* tab = new QWidget();
    auto* splitter = new QSplitter(Qt::Horizontal, tab);

    // ========== Left Panel (Controls) ==========
    auto* left_panel = new QWidget();
    auto* left_layout = new QVBoxLayout(left_panel);
    left_layout->setSpacing(8);
    left_layout->setContentsMargins(8, 8, 8, 8);

    // File selection group
    file_group_ = new QGroupBox(QString::fromUtf8("文件选择"), left_panel);
    auto* file_layout = new QVBoxLayout(file_group_);

    auto* browse_layout = new QHBoxLayout();
    browse_button_ = new QPushButton(QString::fromUtf8("浏览..."), left_panel);
    browse_button_->setFixedWidth(80);
    file_path_edit_ = new QLineEdit(left_panel);
    file_path_edit_->setReadOnly(true);
    file_path_edit_->setPlaceholderText(QString::fromUtf8("选择算例文件..."));
    browse_layout->addWidget(browse_button_);
    browse_layout->addWidget(file_path_edit_);

    file_info_label_ = new QLabel(QString::fromUtf8("子板类型: --  母板: -- x --"), left_panel);
    file_info_label_->setStyleSheet("color: gray;");

    file_layout->addLayout(browse_layout);
    file_layout->addWidget(file_info_label_);
    left_layout->addWidget(file_group_);

    // Parameters widget
    param_widget_ = new ParameterWidget(left_panel);
    left_layout->addWidget(param_widget_);

    // Control buttons
    auto* control_group = new QGroupBox(left_panel);
    auto* control_layout = new QVBoxLayout(control_group);

    auto* button_layout = new QHBoxLayout();
    start_button_ = new QPushButton(QString::fromUtf8("开始求解"), left_panel);
    start_button_->setMinimumHeight(36);
    start_button_->setStyleSheet("font-weight: bold;");
    cancel_button_ = new QPushButton(QString::fromUtf8("取消"), left_panel);
    cancel_button_->setMinimumHeight(36);
    button_layout->addWidget(start_button_);
    button_layout->addWidget(cancel_button_);
    control_layout->addLayout(button_layout);

    status_label_ = new QLabel(QString::fromUtf8("状态: 就绪"), left_panel);
    status_label_->setAlignment(Qt::AlignCenter);
    control_layout->addWidget(status_label_);

    left_layout->addWidget(control_group);

    // Results widget
    results_widget_ = new ResultsWidget(left_panel);
    left_layout->addWidget(results_widget_);

    // Export button
    export_json_button_ = new QPushButton(QString::fromUtf8("导出 JSON..."), left_panel);
    left_layout->addWidget(export_json_button_);

    left_layout->addStretch();

    // ========== Right Panel (Log) ==========
    solver_log_widget_ = new LogWidget();

    // Add to splitter
    splitter->addWidget(left_panel);
    splitter->addWidget(solver_log_widget_);
    splitter->setStretchFactor(0, 0);  // Left: fixed
    splitter->setStretchFactor(1, 1);  // Right: stretch
    splitter->setSizes({320, 680});

    auto* tab_layout = new QVBoxLayout(tab);
    tab_layout->setContentsMargins(0, 0, 0, 0);
    tab_layout->addWidget(splitter);

    return tab;
}

QWidget* MainWindow::CreateGeneratorTab() {
    auto* tab = new QWidget();
    auto* splitter = new QSplitter(Qt::Horizontal, tab);

    // ========== Left Panel (Generator Controls) ==========
    generator_widget_ = new GeneratorWidget();

    // ========== Right Panel (Log) ==========
    generator_log_widget_ = new LogWidget();

    // Add to splitter
    splitter->addWidget(generator_widget_);
    splitter->addWidget(generator_log_widget_);
    splitter->setStretchFactor(0, 0);  // Left: fixed
    splitter->setStretchFactor(1, 1);  // Right: stretch
    splitter->setSizes({320, 680});

    auto* tab_layout = new QVBoxLayout(tab);
    tab_layout->setContentsMargins(0, 0, 0, 0);
    tab_layout->addWidget(splitter);

    return tab;
}

QWidget* MainWindow::CreateCuttingTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 8);

    // ========== Top Control Bar ==========
    auto* control_layout = new QHBoxLayout();

    load_solution_button_ = new QPushButton(QString::fromUtf8("加载方案..."), tab);
    control_layout->addWidget(load_solution_button_);

    solution_path_edit_ = new QLineEdit(tab);
    solution_path_edit_->setReadOnly(true);
    solution_path_edit_->setPlaceholderText(QString::fromUtf8("选择 JSON 解文件..."));
    control_layout->addWidget(solution_path_edit_, 1);

    layout->addLayout(control_layout);

    // ========== Bottom Visualization ==========
    cutting_view_widget_ = new CuttingViewWidget(tab);
    layout->addWidget(cutting_view_widget_, 1);

    return tab;
}

void MainWindow::SetupMenuBar() {
    auto* file_menu = menuBar()->addMenu(QString::fromUtf8("文件(&F)"));

    auto* open_action = new QAction(QString::fromUtf8("打开数据文件(&O)..."), this);
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::OnBrowseFile);
    file_menu->addAction(open_action);

    file_menu->addSeparator();

    auto* exit_action = new QAction(QString::fromUtf8("退出(&X)"), this);
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);
    file_menu->addAction(exit_action);

    auto* help_menu = menuBar()->addMenu(QString::fromUtf8("帮助(&H)"));
    auto* about_action = new QAction(QString::fromUtf8("关于(&A)"), this);
    connect(about_action, &QAction::triggered, [this]() {
        QMessageBox::about(this, QString::fromUtf8("关于"),
            QString::fromUtf8(
                "二维下料问题求解器 GUI\n\n"
                "版本 2.1.0\n\n"
                "基于 Qt6 的 CS-2D-BP-Arc 分支定价求解器界面\n\n"
                "算法: Branch and Price with Arc Flow"));
    });
    help_menu->addAction(about_action);
}

void MainWindow::SetupConnections() {
    // Solver tab connections
    connect(browse_button_, &QPushButton::clicked, this, &MainWindow::OnBrowseFile);
    connect(start_button_, &QPushButton::clicked, this, &MainWindow::OnStartSolve);
    connect(cancel_button_, &QPushButton::clicked, this, &MainWindow::OnCancelSolve);
    connect(export_json_button_, &QPushButton::clicked, this, &MainWindow::OnExportJson);

    // Cutting view tab connections
    connect(load_solution_button_, &QPushButton::clicked, this, &MainWindow::OnLoadSolution);

    // Setup solver worker thread
    solver_thread_ = new QThread(this);
    solver_worker_ = new SolverWorker();
    solver_worker_->moveToThread(solver_thread_);

    connect(this, &MainWindow::StartSolver, solver_worker_, &SolverWorker::RunSolver);
    connect(solver_worker_, &SolverWorker::DataLoaded, this, &MainWindow::OnDataLoaded);
    connect(solver_worker_, &SolverWorker::SolveFinished, this, &MainWindow::OnSolveFinished);
    connect(solver_worker_, &SolverWorker::LogMessage, this, &MainWindow::OnSolverLogMessage);
    connect(solver_worker_, &SolverWorker::SolutionReady, this, &MainWindow::OnSolutionReady);
    connect(solver_worker_, &SolverWorker::ResultsReady, this, &MainWindow::OnResultsReady);

    connect(solver_thread_, &QThread::finished, solver_worker_, &QObject::deleteLater);
    solver_thread_->start();

    // Setup generator worker thread
    generator_thread_ = new QThread(this);
    generator_worker_ = new GeneratorWorker();
    generator_worker_->moveToThread(generator_thread_);

    connect(generator_widget_, &GeneratorWidget::GenerateRequested,
            this, &MainWindow::OnGenerateRequested);
    connect(generator_worker_, &GeneratorWorker::GenerationStarted,
            this, &MainWindow::OnGenerationStarted);
    connect(generator_worker_, &GeneratorWorker::InstanceGenerated,
            this, &MainWindow::OnInstanceGenerated);
    connect(generator_worker_, &GeneratorWorker::GenerationFinished,
            this, &MainWindow::OnGenerationFinished);
    connect(generator_worker_, &GeneratorWorker::LogMessage,
            this, &MainWindow::OnGeneratorLogMessage);

    connect(generator_thread_, &QThread::finished, generator_worker_, &QObject::deleteLater);
    generator_thread_->start();
}

void MainWindow::UpdateSolverUiState(bool is_running) {
    is_running_ = is_running;
    browse_button_->setEnabled(!is_running);
    param_widget_->setEnabled(!is_running);
    start_button_->setEnabled(!is_running && !current_file_path_.isEmpty());
    cancel_button_->setEnabled(is_running);
    export_json_button_->setEnabled(!is_running && !current_json_path_.isEmpty());

    status_label_->setText(is_running ?
        QString::fromUtf8("状态: 运行中...") :
        QString::fromUtf8("状态: 就绪"));
}

void MainWindow::UpdateCuttingNavigation() {
    total_stock_count_ = cutting_view_widget_->GetPlateCount();
    current_stock_index_ = cutting_view_widget_->GetCurrentPlateIndex();
}

// ============================================================================
// Solver Tab Slots
// ============================================================================

void MainWindow::OnBrowseFile() {
    QString path = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("选择数据文件"),
        "D:/YM-Code/CS-2D-Data/data",
        QString::fromUtf8("CSV/TXT 文件 (*.csv *.txt);;所有文件 (*)"));

    if (!path.isEmpty()) {
        current_file_path_ = path;
        file_path_edit_->setText(path);
        file_info_label_->setText(QString::fromUtf8("已选择文件，等待求解..."));
        file_info_label_->setStyleSheet("color: gray;");

        start_button_->setEnabled(true);
        solver_log_widget_->AppendLog(QString::fromUtf8("已选择文件: ") + path);
    }
}

void MainWindow::OnStartSolve() {
    if (current_file_path_.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
            QString::fromUtf8("请先选择数据文件"));
        return;
    }

    // Reset state
    results_widget_->ClearResults();
    solver_log_widget_->ClearLog();
    current_json_path_.clear();

    UpdateSolverUiState(true);

    // Set parameters
    solver_worker_->SetDataPath(current_file_path_);
    solver_worker_->SetTimeLimit(param_widget_->GetTimeLimit());
    solver_worker_->SetSP1Method(param_widget_->GetSP1Method());
    solver_worker_->SetSP2Method(param_widget_->GetSP2Method());

    solver_log_widget_->AppendLog(QString::fromUtf8("开始求解..."));
    statusBar()->showMessage(QString::fromUtf8("正在求解..."));

    emit StartSolver();
}

void MainWindow::OnCancelSolve() {
    if (solver_worker_) {
        solver_worker_->RequestCancel();
        solver_log_widget_->AppendLog(QString::fromUtf8("正在取消..."));
    }
}

void MainWindow::OnExportJson() {
    if (current_json_path_.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("导出"),
            QString::fromUtf8("暂无可导出的解"));
        return;
    }

    QString dest = QFileDialog::getSaveFileName(this,
        QString::fromUtf8("导出 JSON"),
        "solution.json",
        QString::fromUtf8("JSON 文件 (*.json);;所有文件 (*)"));

    if (!dest.isEmpty()) {
        if (QFile::copy(current_json_path_, dest)) {
            solver_log_widget_->AppendLog(QString::fromUtf8("JSON 已导出: ") + dest);
            QMessageBox::information(this, QString::fromUtf8("导出"),
                QString::fromUtf8("导出成功"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("导出错误"),
                QString::fromUtf8("导出失败"));
        }
    }
}

void MainWindow::OnDataLoaded(int numItemTypes, int stockWidth, int stockLength, int totalDemand) {
    QString info = QString::fromUtf8("子板类型: %1  母板: %2 x %3  总需求: %4")
        .arg(numItemTypes).arg(stockWidth).arg(stockLength).arg(totalDemand);
    file_info_label_->setText(info);
    file_info_label_->setStyleSheet("color: black;");
}

void MainWindow::OnSolveFinished(bool success, const QString& message) {
    UpdateSolverUiState(false);

    if (success) {
        statusBar()->showMessage(QString::fromUtf8("求解完成"));
        solver_log_widget_->AppendLog(message);
    } else {
        statusBar()->showMessage(QString::fromUtf8("求解已停止"));
        solver_log_widget_->AppendLog(QString::fromUtf8("求解已停止: ") + message);
        if (!message.contains(QString::fromUtf8("取消"))) {
            QMessageBox::warning(this, QString::fromUtf8("求解"), message);
        }
    }
}

void MainWindow::OnSolverLogMessage(const QString& message) {
    solver_log_widget_->AppendLog(message);
}

void MainWindow::OnSolutionReady(const QString& jsonPath) {
    current_json_path_ = jsonPath;
    export_json_button_->setEnabled(true);

    // Auto-load solution to cutting view
    cutting_view_widget_->LoadSolution(jsonPath);
    solution_path_edit_->setText(jsonPath);
    UpdateCuttingNavigation();
}

void MainWindow::OnResultsReady(int optimalValue, double rootLB, double gap,
                                int nodeCount, double utilization) {
    results_widget_->SetOptimalValue(optimalValue);
    results_widget_->SetRootLB(rootLB);
    results_widget_->SetGap(gap);
    results_widget_->SetNodeCount(nodeCount);
    results_widget_->SetUtilization(utilization);
}

// ============================================================================
// Generator Tab Slots
// ============================================================================

void MainWindow::OnGenerateRequested(const GeneratorConfig& config) {
    generator_log_widget_->ClearLog();
    generator_log_widget_->AppendLog(QString::fromUtf8("开始生成算例..."));

    generator_worker_->SetConfig(config);

    // Use QMetaObject to invoke across threads
    QMetaObject::invokeMethod(generator_worker_, "RunGeneration", Qt::QueuedConnection);

    statusBar()->showMessage(QString::fromUtf8("生成中..."));
}

void MainWindow::OnGenerationStarted(int count) {
    generator_log_widget_->AppendLog(QString::fromUtf8("正在生成 %1 个算例...").arg(count));
}

void MainWindow::OnInstanceGenerated(int index, const QString& filename) {
    generator_log_widget_->AppendLog(QString::fromUtf8("[%1] 已生成: %2").arg(index).arg(filename));
}

void MainWindow::OnGenerationFinished(bool success, const QString& message,
                                       const QStringList& files) {
    if (success) {
        statusBar()->showMessage(QString::fromUtf8("生成完成"));
        generator_log_widget_->AppendLog(QString::fromUtf8("生成完成: ") + message);

        if (!files.isEmpty()) {
            QString first_file = files.first();
            generator_log_widget_->AppendLog(QString::fromUtf8("算例已生成，可用于求解: ") + first_file);
        }
    } else {
        statusBar()->showMessage(QString::fromUtf8("生成失败"));
        generator_log_widget_->AppendLog(QString::fromUtf8("生成失败: ") + message);
    }
}

void MainWindow::OnGeneratorLogMessage(const QString& message) {
    generator_log_widget_->AppendLog(message);
}

// ============================================================================
// Cutting View Tab Slots
// ============================================================================

void MainWindow::OnLoadSolution() {
    QString path = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("加载切割方案"),
        "D:/YM-Code/CS-2D-BP-Arc",
        QString::fromUtf8("JSON 文件 (*.json);;所有文件 (*)"));

    if (!path.isEmpty()) {
        if (cutting_view_widget_->LoadSolution(path)) {
            solution_path_edit_->setText(path);
            UpdateCuttingNavigation();
            statusBar()->showMessage(QString::fromUtf8("方案已加载"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("加载错误"),
                QString::fromUtf8("无法加载切割方案文件"));
        }
    }
}

void MainWindow::OnPrevStock() {
    cutting_view_widget_->ShowPrevPlate();
    UpdateCuttingNavigation();
}

void MainWindow::OnNextStock() {
    cutting_view_widget_->ShowNextPlate();
    UpdateCuttingNavigation();
}

void MainWindow::OnZoomChanged(int index) {
    // Reserved for zoom functionality
    Q_UNUSED(index);
}
