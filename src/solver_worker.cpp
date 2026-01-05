// solver_worker.cpp - 后台求解线程实现

#include "solver_worker.h"
#include "2DBP.h"

#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <filesystem>

SolverWorker::SolverWorker(QObject* parent)
    : QObject(parent)
    , time_limit_(60)
    , sp1_method_(kArcFlow)
    , sp2_method_(kArcFlow)
    , cancel_requested_(false)
    , params_(nullptr)
    , data_(nullptr)
    , root_node_(nullptr) {
}

SolverWorker::~SolverWorker() {
    delete params_;
    delete data_;
    delete root_node_;
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

void SolverWorker::RequestCancel() {
    cancel_requested_ = true;
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

void SolverWorker::RunSolver() {
    cancel_requested_ = false;

    // 清理并重新分配数据结构
    delete params_;
    delete data_;
    delete root_node_;
    params_ = new ProblemParams();
    data_ = new ProblemData();
    root_node_ = new BPNode();

    try {
        // 创建输出目录
        std::filesystem::create_directories("logs");
        std::filesystem::create_directories("results");

        // 设置参数
        params_->time_limit_ = time_limit_;
        params_->sp1_method_ = sp1_method_;
        params_->sp2_method_ = sp2_method_;
        params_->start_time_ = std::chrono::steady_clock::now();
        root_node_->id_ = 1;

        //===========================================
        // 阶段 0: 数据读取
        //===========================================
        emit StageStarted(0, QString::fromUtf8("数据读取"));
        emit LogMessage(QString::fromUtf8("开始读取数据文件..."));
        auto t0 = std::chrono::steady_clock::now();

        auto [status, num_items, num_strips] = LoadInput(*params_, *data_, data_path_.toStdString());
        if (status != 0) {
            emit LogMessage(QString::fromUtf8("数据读取失败"));
            emit SolveFinished(false, QString::fromUtf8("数据读取失败"));
            return;
        }

        // 生成 Arc Flow 网络 (如果需要)
        if (sp1_method_ == kArcFlow || sp2_method_ == kArcFlow) {
            emit LogMessage(QString::fromUtf8("生成 Arc Flow 网络..."));
            GenerateAllArcs(*data_, *params_);
        }

        auto t0_end = std::chrono::steady_clock::now();
        double t0_sec = std::chrono::duration<double>(t0_end - t0).count();

        emit DataLoaded(params_->num_item_types_,
                        params_->stock_width_,
                        params_->stock_length_,
                        num_items);
        emit LogMessage(QString::fromUtf8("子板类型: %1, 母板尺寸: %2 x %3")
            .arg(params_->num_item_types_)
            .arg(params_->stock_width_)
            .arg(params_->stock_length_));
        emit StageCompleted(0, num_items, t0_sec);

        if (cancel_requested_) {
            emit SolveFinished(false, QString::fromUtf8("用户取消"));
            return;
        }

        //===========================================
        // 阶段 1: 启发式生成初始解
        //===========================================
        emit StageStarted(1, QString::fromUtf8("启发式初始解"));
        emit LogMessage(QString::fromUtf8("生成启发式初始解..."));
        auto t1 = std::chrono::steady_clock::now();

        RunHeuristic(*params_, *data_, *root_node_);

        auto t1_end = std::chrono::steady_clock::now();
        double t1_sec = std::chrono::duration<double>(t1_end - t1).count();
        emit LogMessage(QString::fromUtf8("启发式完成, 初始解: %1 块母板")
            .arg(static_cast<int>(params_->global_best_int_)));
        emit StageCompleted(1, params_->global_best_int_, t1_sec);

        if (cancel_requested_) {
            emit SolveFinished(false, QString::fromUtf8("用户取消"));
            return;
        }

        //===========================================
        // 阶段 2: 根节点列生成
        //===========================================
        emit StageStarted(2, QString::fromUtf8("根节点列生成"));
        emit LogMessage(QString::fromUtf8("开始列生成求解..."));
        auto t2 = std::chrono::steady_clock::now();

        SolveRootCG(*params_, *data_, *root_node_);
        params_->root_lb_ = root_node_->lower_bound_;

        auto t2_end = std::chrono::steady_clock::now();
        double t2_sec = std::chrono::duration<double>(t2_end - t2).count();
        emit LogMessage(QString::fromUtf8("列生成收敛, LP下界: %1")
            .arg(root_node_->lower_bound_, 0, 'f', 4));
        emit StageCompleted(2, root_node_->lower_bound_, t2_sec);

        if (cancel_requested_) {
            emit SolveFinished(false, QString::fromUtf8("用户取消"));
            return;
        }

        //===========================================
        // 阶段 3: 整数性检查
        //===========================================
        emit StageStarted(3, QString::fromUtf8("整数性检查"));
        auto t3 = std::chrono::steady_clock::now();

        bool is_integer = IsIntegerSolution(root_node_->solution_);

        auto t3_end = std::chrono::steady_clock::now();
        double t3_sec = std::chrono::duration<double>(t3_end - t3).count();
        emit StageCompleted(3, is_integer ? 1.0 : 0.0, t3_sec);

        if (is_integer) {
            // LP解为整数，直接输出
            emit LogMessage(QString::fromUtf8("根节点解为整数解，无需分支"));
            params_->global_best_int_ = root_node_->solution_.obj_val_;
            params_->global_best_y_cols_ = root_node_->solution_.y_columns_;
            params_->global_best_x_cols_ = root_node_->solution_.x_columns_;
            params_->gap_ = 0.0;
            params_->node_counter_ = 1;

            // 跳过分支定价阶段
            emit StageCompleted(4, 0, 0);
        } else {
            //===========================================
            // 阶段 4: 分支定价
            //===========================================
            emit StageStarted(4, QString::fromUtf8("分支定价"));
            emit LogMessage(QString::fromUtf8("LP解非整数，开始分支定价..."));
            auto t4 = std::chrono::steady_clock::now();

            RunBranchAndPrice(*params_, *data_, root_node_);

            auto t4_end = std::chrono::steady_clock::now();
            double t4_sec = std::chrono::duration<double>(t4_end - t4).count();
            emit LogMessage(QString::fromUtf8("分支定价完成, 节点数: %1")
                .arg(params_->node_counter_));
            emit StageCompleted(4, params_->global_best_int_, t4_sec);
        }

        // 导出解到 JSON
        if (params_->global_best_int_ < 1e10) {
            ExportSolution(*params_, *data_);
            QString json_path = GetLatestSolutionPath();
            if (!json_path.isEmpty()) {
                emit SolutionReady(json_path);
                emit LogMessage(QString::fromUtf8("解已导出: %1").arg(json_path));
            }
        }

        // 计算总利用率
        double total_item_area = 0;
        for (int i = 0; i < params_->num_item_types_; i++) {
            total_item_area += data_->item_types_[i].width_ *
                              data_->item_types_[i].length_ *
                              data_->item_types_[i].demand_;
        }
        int num_plates = static_cast<int>(params_->global_best_int_);
        double total_stock_area = num_plates * params_->stock_width_ * params_->stock_length_;
        double utilization = (total_stock_area > 0) ? (total_item_area / total_stock_area) : 0.0;

        // 发送结果
        emit ResultsReady(
            num_plates,
            params_->root_lb_,
            params_->gap_,
            params_->node_counter_,
            utilization
        );

        // 计算总耗时
        double elapsed_sec = GetElapsedTime(*params_);
        emit LogMessage(QString::fromUtf8("总耗时: %1 秒").arg(elapsed_sec, 0, 'f', 3));

        // 完成
        if (params_->is_timeout_) {
            emit SolveFinished(true, QString::fromUtf8("求解完成 (超时终止)"));
        } else {
            emit SolveFinished(true, QString::fromUtf8("求解完成"));
        }

    } catch (const std::exception& e) {
        emit LogMessage(QString("Error: %1").arg(e.what()));
        emit SolveFinished(false, QString("Error: %1").arg(e.what()));
    } catch (...) {
        emit LogMessage("Unknown error");
        emit SolveFinished(false, "Unknown error");
    }
}
