// main_window.cpp - 主窗口实现

#include "main_window.h"
#include "parameter_widget.h"
#include "results_widget.h"
#include "log_widget.h"
#include "cutting_view_dialog.h"
#include "solver_worker.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , worker_thread_(nullptr)
    , solver_worker_(nullptr)
    , is_running_(false)
    , total_runtime_(0.0) {
    SetupUi();
    SetupMenuBar();
    SetupConnections();
    UpdateUiState(false);

    setWindowTitle(QString::fromUtf8("二维下料问题求解器"));
    resize(900, 650);

    // 创建可视化对话框 (非模态)
    cutting_view_dialog_ = new CuttingViewDialog(this);
}

MainWindow::~MainWindow() {
    if (worker_thread_) {
        worker_thread_->quit();
        worker_thread_->wait();
    }
}

void MainWindow::SetupUi() {
    auto* central = new QWidget(this);
    auto* main_layout = new QVBoxLayout(central);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(10, 10, 10, 10);

    // Top row: File selection + Parameters
    auto* top_layout = new QHBoxLayout();

    // File selection group
    file_group_ = new QGroupBox(QString::fromUtf8("文件选择"), this);
    auto* file_layout = new QVBoxLayout(file_group_);

    auto* browse_layout = new QHBoxLayout();
    browse_button_ = new QPushButton(QString::fromUtf8("浏览..."), this);
    browse_button_->setFixedWidth(80);
    file_path_edit_ = new QLineEdit(this);
    file_path_edit_->setReadOnly(true);
    file_path_edit_->setPlaceholderText(QString::fromUtf8("选择算例数据文件..."));
    browse_layout->addWidget(browse_button_);
    browse_layout->addWidget(file_path_edit_);

    file_info_label_ = new QLabel(QString::fromUtf8("子板类型: --  母板尺寸: -- x --"), this);
    file_info_label_->setStyleSheet("color: gray;");

    file_layout->addLayout(browse_layout);
    file_layout->addWidget(file_info_label_);

    // Parameters widget
    param_widget_ = new ParameterWidget(this);

    top_layout->addWidget(file_group_, 1);
    top_layout->addWidget(param_widget_, 1);
    main_layout->addLayout(top_layout);

    // Run control row
    auto* run_layout = new QHBoxLayout();
    start_button_ = new QPushButton(QString::fromUtf8("开始求解"), this);
    start_button_->setMinimumHeight(36);
    start_button_->setStyleSheet("font-weight: bold;");
    cancel_button_ = new QPushButton(QString::fromUtf8("取消"), this);
    cancel_button_->setMinimumHeight(36);
    status_label_ = new QLabel(QString::fromUtf8("状态: 就绪"), this);

    run_layout->addWidget(start_button_);
    run_layout->addWidget(cancel_button_);
    run_layout->addStretch();
    run_layout->addWidget(status_label_);
    main_layout->addLayout(run_layout);

    // Progress group (5 stages)
    progress_group_ = new QGroupBox(QString::fromUtf8("求解进度"), this);
    auto* progress_layout = new QGridLayout(progress_group_);

    const char* stage_names[] = {
        "数据读取:",
        "启发式:",
        "根节点 CG:",
        "整数性检查:",
        "分支定价:"
    };
    for (int i = 0; i < 5; ++i) {
        stage_labels_[i] = new QLabel(QString::fromUtf8(stage_names[i]), this);
        progress_bars_[i] = new QProgressBar(this);
        progress_bars_[i]->setRange(0, 100);
        progress_bars_[i]->setValue(0);
        progress_bars_[i]->setTextVisible(true);
        time_labels_[i] = new QLabel("--", this);
        time_labels_[i]->setFixedWidth(70);
        time_labels_[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        progress_layout->addWidget(stage_labels_[i], i, 0);
        progress_layout->addWidget(progress_bars_[i], i, 1);
        progress_layout->addWidget(time_labels_[i], i, 2);
    }
    progress_layout->setColumnStretch(1, 1);
    main_layout->addWidget(progress_group_);

    // Results and Log (splitter)
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    results_widget_ = new ResultsWidget(this);
    log_widget_ = new LogWidget(this);

    splitter->addWidget(results_widget_);
    splitter->addWidget(log_widget_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    main_layout->addWidget(splitter, 1);

    // Bottom row: buttons + Total time
    auto* bottom_layout = new QHBoxLayout();
    export_json_button_ = new QPushButton(QString::fromUtf8("导出 JSON..."), this);
    view_cutting_button_ = new QPushButton(QString::fromUtf8("查看切割方案..."), this);
    total_time_label_ = new QLabel(QString::fromUtf8("总耗时: --"), this);
    total_time_label_->setStyleSheet("font-weight: bold;");

    bottom_layout->addWidget(export_json_button_);
    bottom_layout->addWidget(view_cutting_button_);
    bottom_layout->addStretch();
    bottom_layout->addWidget(total_time_label_);
    main_layout->addLayout(bottom_layout);

    setCentralWidget(central);

    // Status bar
    statusBar()->showMessage(QString::fromUtf8("就绪"));
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
                "版本 1.0.0\n\n"
                "基于 Qt6 的 CS-2D-BP-Arc 分支定价求解器界面\n\n"
                "算法: Branch and Price with Arc Flow"));
    });
    help_menu->addAction(about_action);
}

void MainWindow::SetupConnections() {
    connect(browse_button_, &QPushButton::clicked, this, &MainWindow::OnBrowseFile);
    connect(start_button_, &QPushButton::clicked, this, &MainWindow::OnStartSolve);
    connect(cancel_button_, &QPushButton::clicked, this, &MainWindow::OnCancelSolve);
    connect(export_json_button_, &QPushButton::clicked, this, &MainWindow::OnExportJson);
    connect(view_cutting_button_, &QPushButton::clicked, this, &MainWindow::OnViewCutting);

    // Setup worker thread
    worker_thread_ = new QThread(this);
    solver_worker_ = new SolverWorker();
    solver_worker_->moveToThread(worker_thread_);

    connect(this, &MainWindow::StartSolver, solver_worker_, &SolverWorker::RunSolver);
    connect(solver_worker_, &SolverWorker::DataLoaded, this, &MainWindow::OnDataLoaded);
    connect(solver_worker_, &SolverWorker::StageStarted, this, &MainWindow::OnStageStarted);
    connect(solver_worker_, &SolverWorker::StageProgress, this, &MainWindow::OnStageProgress);
    connect(solver_worker_, &SolverWorker::StageCompleted, this, &MainWindow::OnStageCompleted);
    connect(solver_worker_, &SolverWorker::SolveFinished, this, &MainWindow::OnSolveFinished);
    connect(solver_worker_, &SolverWorker::LogMessage, this, &MainWindow::OnLogMessage);
    connect(solver_worker_, &SolverWorker::SolutionReady, this, &MainWindow::OnSolutionReady);
    connect(solver_worker_, &SolverWorker::ResultsReady, this, &MainWindow::OnResultsReady);

    connect(worker_thread_, &QThread::finished, solver_worker_, &QObject::deleteLater);

    worker_thread_->start();
}

void MainWindow::UpdateUiState(bool is_running) {
    is_running_ = is_running;
    browse_button_->setEnabled(!is_running);
    param_widget_->setEnabled(!is_running);
    start_button_->setEnabled(!is_running && !current_file_path_.isEmpty());
    cancel_button_->setEnabled(is_running);
    export_json_button_->setEnabled(!is_running && !current_json_path_.isEmpty());
    view_cutting_button_->setEnabled(!is_running);

    status_label_->setText(is_running ? QString::fromUtf8("状态: 运行中...") : QString::fromUtf8("状态: 就绪"));
}

void MainWindow::ResetProgress() {
    for (int i = 0; i < 5; ++i) {
        progress_bars_[i]->setValue(0);
        time_labels_[i]->setText("--");
    }
    results_widget_->ClearResults();
    log_widget_->ClearLog();
    current_json_path_.clear();
    total_runtime_ = 0.0;
    total_time_label_->setText(QString::fromUtf8("总耗时: --"));
}

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
        log_widget_->AppendLog(QString::fromUtf8("已选择文件: ") + path);
    }
}

void MainWindow::OnStartSolve() {
    if (current_file_path_.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
            QString::fromUtf8("请先选择数据文件"));
        return;
    }

    ResetProgress();
    UpdateUiState(true);

    // Set parameters
    solver_worker_->SetDataPath(current_file_path_);
    solver_worker_->SetTimeLimit(param_widget_->GetTimeLimit());
    solver_worker_->SetSP1Method(param_widget_->GetSP1Method());
    solver_worker_->SetSP2Method(param_widget_->GetSP2Method());

    log_widget_->AppendLog(QString::fromUtf8("开始求解..."));
    statusBar()->showMessage(QString::fromUtf8("正在求解..."));

    emit StartSolver();
}

void MainWindow::OnCancelSolve() {
    if (solver_worker_) {
        solver_worker_->RequestCancel();
        log_widget_->AppendLog(QString::fromUtf8("正在取消..."));
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
            log_widget_->AppendLog(QString::fromUtf8("JSON 已导出: ") + dest);
            QMessageBox::information(this, QString::fromUtf8("导出"),
                QString::fromUtf8("导出成功"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("导出错误"),
                QString::fromUtf8("导出失败"));
        }
    }
}

void MainWindow::OnViewCutting() {
    // 如果有当前解，先加载到对话框
    if (!current_json_path_.isEmpty()) {
        cutting_view_dialog_->LoadSolution(current_json_path_);
    }
    cutting_view_dialog_->show();
    cutting_view_dialog_->raise();
    cutting_view_dialog_->activateWindow();
}

void MainWindow::OnDataLoaded(int numItemTypes, int stockWidth, int stockLength, int totalDemand) {
    QString info = QString::fromUtf8("子板类型: %1  母板尺寸: %2 x %3  总需求: %4")
        .arg(numItemTypes).arg(stockWidth).arg(stockLength).arg(totalDemand);
    file_info_label_->setText(info);
    file_info_label_->setStyleSheet("color: black;");
}

void MainWindow::OnStageStarted(int stage, const QString& name) {
    if (stage >= 0 && stage < 5) {
        progress_bars_[stage]->setValue(0);
        time_labels_[stage]->setText("...");
    }
    log_widget_->AppendLog(QString::fromUtf8("开始: ") + name);
}

void MainWindow::OnStageProgress(int stage, int percent, double elapsed) {
    if (stage >= 0 && stage < 5) {
        progress_bars_[stage]->setValue(percent);
        time_labels_[stage]->setText(QString("%1s").arg(elapsed, 0, 'f', 1));
    }
}

void MainWindow::OnStageCompleted(int stage, double value, double runtime) {
    if (stage >= 0 && stage < 5) {
        progress_bars_[stage]->setValue(100);
        time_labels_[stage]->setText(QString("%1s").arg(runtime, 0, 'f', 2));
    }

    total_runtime_ += runtime;
    total_time_label_->setText(QString::fromUtf8("总耗时: %1s").arg(total_runtime_, 0, 'f', 2));
}

void MainWindow::OnSolveFinished(bool success, const QString& message) {
    UpdateUiState(false);

    if (success) {
        statusBar()->showMessage(QString::fromUtf8("求解完成"));
        log_widget_->AppendLog(message);
    } else {
        statusBar()->showMessage(QString::fromUtf8("求解已停止"));
        log_widget_->AppendLog(QString::fromUtf8("求解已停止: ") + message);
        if (!message.contains(QString::fromUtf8("取消"))) {
            QMessageBox::warning(this, QString::fromUtf8("求解"), message);
        }
    }
}

void MainWindow::OnLogMessage(const QString& message) {
    log_widget_->AppendLog(message);
}

void MainWindow::OnSolutionReady(const QString& jsonPath) {
    current_json_path_ = jsonPath;
    export_json_button_->setEnabled(true);
}

void MainWindow::OnResultsReady(int optimalValue, double rootLB, double gap,
                                int nodeCount, double utilization) {
    results_widget_->SetOptimalValue(optimalValue);
    results_widget_->SetRootLB(rootLB);
    results_widget_->SetGap(gap);
    results_widget_->SetNodeCount(nodeCount);
    results_widget_->SetUtilization(utilization);
}
