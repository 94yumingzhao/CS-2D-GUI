// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// analysis_widget.cpp - 求解分析页面实现

#include "analysis_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QComboBox>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QProgressBar>

AnalysisWidget::AnalysisWidget(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
    SetupConnections();
}

void AnalysisWidget::SetupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(8, 8, 8, 8);

    // 文件加载区域
    main_layout->addWidget(CreateFileGroup());

    // 主分割器 (左: 概要+时间, 右: 收敛+节点表)
    QSplitter* main_splitter = new QSplitter(Qt::Horizontal, this);

    // 左侧面板
    QWidget* left_panel = new QWidget();
    QVBoxLayout* left_layout = new QVBoxLayout(left_panel);
    left_layout->setSpacing(8);
    left_layout->setContentsMargins(0, 0, 0, 0);
    left_layout->addWidget(CreateSummaryGroup());
    left_layout->addWidget(CreateTimeBreakdownGroup());
    left_layout->addWidget(CreateConvergenceGroup(), 1);
    main_splitter->addWidget(left_panel);

    // 右侧面板 (节点表格)
    main_splitter->addWidget(CreateNodeTableGroup());

    main_splitter->setStretchFactor(0, 1);
    main_splitter->setStretchFactor(1, 2);

    main_layout->addWidget(main_splitter, 1);
}

void AnalysisWidget::SetupConnections() {
    connect(load_file_button_, &QPushButton::clicked, this, &AnalysisWidget::OnLoadFile);
    connect(status_filter_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnalysisWidget::OnNodeFilterChanged);
    connect(branch_type_filter_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnalysisWidget::OnNodeFilterChanged);
    connect(node_table_, &QTableWidget::cellClicked, this, &AnalysisWidget::OnNodeTableClicked);
}

QGroupBox* AnalysisWidget::CreateFileGroup() {
    QGroupBox* group = new QGroupBox(QString::fromUtf8("JSON 解文件"));
    QHBoxLayout* layout = new QHBoxLayout(group);

    file_path_edit_ = new QLineEdit();
    file_path_edit_->setReadOnly(true);
    file_path_edit_->setPlaceholderText(QString::fromUtf8("选择解文件..."));

    load_file_button_ = new QPushButton(QString::fromUtf8("加载..."));
    load_file_button_->setFixedWidth(80);

    layout->addWidget(file_path_edit_, 1);
    layout->addWidget(load_file_button_);

    return group;
}

QGroupBox* AnalysisWidget::CreateSummaryGroup() {
    QGroupBox* group = new QGroupBox(QString::fromUtf8("求解概要"));
    QGridLayout* layout = new QGridLayout(group);
    layout->setSpacing(6);

    int row = 0;

    layout->addWidget(new QLabel(QString::fromUtf8("状态:")), row, 0);
    solve_status_label_ = new QLabel("-");
    solve_status_label_->setStyleSheet("font-weight: bold;");
    layout->addWidget(solve_status_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("目标值:")), row, 0);
    objective_label_ = new QLabel("-");
    layout->addWidget(objective_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("启发式上界:")), row, 0);
    heuristic_label_ = new QLabel("-");
    layout->addWidget(heuristic_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("改进:")), row, 0);
    improvement_label_ = new QLabel("-");
    improvement_label_->setStyleSheet("color: green;");
    layout->addWidget(improvement_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("根节点下界:")), row, 0);
    root_lb_label_ = new QLabel("-");
    layout->addWidget(root_lb_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("最终下界:")), row, 0);
    final_lb_label_ = new QLabel("-");
    layout->addWidget(final_lb_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("Gap:")), row, 0);
    gap_label_ = new QLabel("-");
    layout->addWidget(gap_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("总节点数:")), row, 0);
    total_nodes_label_ = new QLabel("-");
    layout->addWidget(total_nodes_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("剪枝节点:")), row, 0);
    pruned_nodes_label_ = new QLabel("-");
    layout->addWidget(pruned_nodes_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("不可行节点:")), row, 0);
    infeasible_nodes_label_ = new QLabel("-");
    layout->addWidget(infeasible_nodes_label_, row++, 1);

    layout->addWidget(new QLabel(QString::fromUtf8("整数解节点:")), row, 0);
    integer_nodes_label_ = new QLabel("-");
    layout->addWidget(integer_nodes_label_, row++, 1);

    layout->setColumnStretch(1, 1);
    return group;
}

QGroupBox* AnalysisWidget::CreateTimeBreakdownGroup() {
    QGroupBox* group = new QGroupBox(QString::fromUtf8("时间统计"));
    QVBoxLayout* layout = new QVBoxLayout(group);

    time_table_ = new QTableWidget();
    time_table_->setColumnCount(3);
    time_table_->setHorizontalHeaderLabels({
        QString::fromUtf8("阶段"),
        QString::fromUtf8("耗时(秒)"),
        QString::fromUtf8("占比")
    });
    time_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    time_table_->setSelectionMode(QAbstractItemView::NoSelection);
    time_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    time_table_->setMaximumHeight(180);

    layout->addWidget(time_table_);

    total_time_label_ = new QLabel(QString::fromUtf8("总计: -"));
    total_time_label_->setStyleSheet("font-weight: bold;");
    layout->addWidget(total_time_label_);

    return group;
}

QGroupBox* AnalysisWidget::CreateConvergenceGroup() {
    QGroupBox* group = new QGroupBox(QString::fromUtf8("收敛历史"));
    QVBoxLayout* layout = new QVBoxLayout(group);

    convergence_table_ = new QTableWidget();
    convergence_table_->setColumnCount(5);
    convergence_table_->setHorizontalHeaderLabels({
        QString::fromUtf8("时间(秒)"),
        QString::fromUtf8("事件"),
        QString::fromUtf8("节点"),
        QString::fromUtf8("下界"),
        QString::fromUtf8("上界")
    });
    convergence_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    convergence_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    convergence_table_->setSelectionMode(QAbstractItemView::SingleSelection);
    convergence_table_->setAlternatingRowColors(true);
    convergence_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(convergence_table_);

    return group;
}

QGroupBox* AnalysisWidget::CreateNodeTableGroup() {
    QGroupBox* group = new QGroupBox(QString::fromUtf8("分支定价树节点"));
    QVBoxLayout* layout = new QVBoxLayout(group);

    // 筛选器
    QHBoxLayout* filter_layout = new QHBoxLayout();

    filter_layout->addWidget(new QLabel(QString::fromUtf8("状态:")));
    status_filter_combo_ = new QComboBox();
    status_filter_combo_->addItem(QString::fromUtf8("全部"), "");
    status_filter_combo_->addItem(QString::fromUtf8("已分支"), "branched");
    status_filter_combo_->addItem(QString::fromUtf8("已剪枝"), "pruned");
    status_filter_combo_->addItem(QString::fromUtf8("不可行"), "infeasible");
    status_filter_combo_->addItem(QString::fromUtf8("整数解"), "integer");
    status_filter_combo_->addItem(QString::fromUtf8("活跃"), "active");
    filter_layout->addWidget(status_filter_combo_);

    filter_layout->addSpacing(16);

    filter_layout->addWidget(new QLabel(QString::fromUtf8("分支类型:")));
    branch_type_filter_combo_ = new QComboBox();
    branch_type_filter_combo_->addItem(QString::fromUtf8("全部"), "");
    branch_type_filter_combo_->addItem(QString::fromUtf8("SP1"), "SP1");
    branch_type_filter_combo_->addItem(QString::fromUtf8("SP2"), "SP2");
    filter_layout->addWidget(branch_type_filter_combo_);

    filter_layout->addStretch();
    layout->addLayout(filter_layout);

    // 表格
    node_table_ = new QTableWidget();
    node_table_->setColumnCount(9);
    node_table_->setHorizontalHeaderLabels({
        QString::fromUtf8("ID"),
        QString::fromUtf8("父节点"),
        QString::fromUtf8("深度"),
        QString::fromUtf8("下界"),
        QString::fromUtf8("状态"),
        QString::fromUtf8("分支"),
        QString::fromUtf8("方向"),
        QString::fromUtf8("CG迭代"),
        QString::fromUtf8("列数(Y/X)")
    });
    node_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    node_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    node_table_->setSelectionMode(QAbstractItemView::SingleSelection);
    node_table_->setAlternatingRowColors(true);
    node_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(node_table_, 1);

    return group;
}

void AnalysisWidget::OnLoadFile() {
    QString file_path = QFileDialog::getOpenFileName(
        this, QString::fromUtf8("打开解文件"),
        "D:/YM-Code/CS-2D-BP-Arc/results",
        QString::fromUtf8("JSON 文件 (*.json);;所有文件 (*)"));

    if (!file_path.isEmpty()) {
        LoadAnalysisData(file_path);
    }
}

bool AnalysisWidget::LoadAnalysisData(const QString& json_path) {
    QFile file(json_path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
            QString::fromUtf8("无法打开文件: ") + json_path);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
            QString::fromUtf8("JSON 解析错误: ") + error.errorString());
        return false;
    }

    if (!doc.isObject()) {
        QMessageBox::warning(this, QString::fromUtf8("错误"),
            QString::fromUtf8("无效的 JSON 格式"));
        return false;
    }

    QJsonObject root = doc.object();
    current_file_path_ = json_path;
    file_path_edit_->setText(json_path);

    // 更新各部分
    QJsonObject summary = root["summary"].toObject();
    QJsonObject branch_stats = root["branch_stats"].toObject();
    UpdateSummary(summary, branch_stats);

    if (root.contains("time_breakdown")) {
        UpdateTimeBreakdown(root["time_breakdown"].toObject());
    }

    if (root.contains("convergence")) {
        UpdateConvergenceTable(root["convergence"].toArray());
    }

    if (root.contains("bp_tree")) {
        QJsonObject bp_tree = root["bp_tree"].toObject();
        if (bp_tree.contains("nodes")) {
            UpdateNodeTable(bp_tree["nodes"].toArray());
        }
    }

    return true;
}

void AnalysisWidget::ClearAnalysisData() {
    file_path_edit_->clear();

    solve_status_label_->setText("-");
    objective_label_->setText("-");
    heuristic_label_->setText("-");
    improvement_label_->setText("-");
    root_lb_label_->setText("-");
    final_lb_label_->setText("-");
    gap_label_->setText("-");
    total_nodes_label_->setText("-");
    pruned_nodes_label_->setText("-");
    infeasible_nodes_label_->setText("-");
    integer_nodes_label_->setText("-");

    time_table_->setRowCount(0);
    total_time_label_->setText(QString::fromUtf8("总计: -"));

    convergence_table_->setRowCount(0);
    node_table_->setRowCount(0);

    all_nodes_.clear();
    convergence_data_.clear();
    current_file_path_.clear();
}

void AnalysisWidget::UpdateSummary(const QJsonObject& summary, const QJsonObject& branch_stats) {
    Q_UNUSED(branch_stats);

    QString status = summary["solve_status"].toString("unknown");
    solve_status_label_->setText(status);
    if (status == "optimal") {
        solve_status_label_->setStyleSheet("font-weight: bold; color: green;");
    } else if (status == "timeout") {
        solve_status_label_->setStyleSheet("font-weight: bold; color: orange;");
    } else if (status == "infeasible") {
        solve_status_label_->setStyleSheet("font-weight: bold; color: red;");
    } else {
        solve_status_label_->setStyleSheet("font-weight: bold;");
    }

    double objective = summary["objective_value"].toDouble();
    double heuristic = summary["heuristic_value"].toDouble();

    objective_label_->setText(QString::number(objective, 'f', 0));

    if (heuristic > 0 && heuristic < 1e9) {
        heuristic_label_->setText(QString::number(heuristic, 'f', 0));
        double improvement = heuristic - objective;
        double improvement_rate = (improvement / heuristic) * 100;
        improvement_label_->setText(QString::fromUtf8("%1 (%2%)")
            .arg(improvement, 0, 'f', 0)
            .arg(improvement_rate, 0, 'f', 1));
    } else {
        heuristic_label_->setText("-");
        improvement_label_->setText("-");
    }

    root_lb_label_->setText(QString::number(summary["root_lb"].toDouble(), 'f', 2));
    final_lb_label_->setText(QString::number(summary["final_lb"].toDouble(), 'f', 2));
    gap_label_->setText(QString::number(summary["gap"].toDouble() * 100, 'f', 2) + "%");

    total_nodes_label_->setText(QString::number(summary["total_nodes"].toInt()));
    pruned_nodes_label_->setText(QString::number(summary["pruned_nodes"].toInt()));
    infeasible_nodes_label_->setText(QString::number(summary["infeasible_nodes"].toInt()));
    integer_nodes_label_->setText(QString::number(summary["integer_nodes"].toInt()));
}

void AnalysisWidget::UpdateTimeBreakdown(const QJsonObject& time_breakdown) {
    time_table_->setRowCount(0);

    struct TimeItem {
        QString name;
        QString key;
    };

    std::vector<TimeItem> items = {
        {QString::fromUtf8("数据读取"), "data_loading"},
        {QString::fromUtf8("网络构建"), "network_building"},
        {QString::fromUtf8("启发式"), "heuristic"},
        {QString::fromUtf8("根节点CG"), "root_cg"},
        {QString::fromUtf8("分支定价"), "branch_and_price"},
        {QString::fromUtf8("导出"), "output"}
    };

    double total = time_breakdown["total"].toDouble();
    total_time_label_->setText(QString::fromUtf8("总计: %1 秒").arg(total, 0, 'f', 2));

    for (const auto& item : items) {
        double value = time_breakdown[item.key].toDouble();
        double percent = (total > 0) ? (value / total * 100) : 0;

        int row = time_table_->rowCount();
        time_table_->insertRow(row);

        time_table_->setItem(row, 0, new QTableWidgetItem(item.name));
        time_table_->setItem(row, 1, new QTableWidgetItem(QString::number(value, 'f', 2)));
        time_table_->setItem(row, 2, new QTableWidgetItem(QString::number(percent, 'f', 1) + "%"));
    }
}

void AnalysisWidget::UpdateConvergenceTable(const QJsonArray& convergence) {
    convergence_table_->setRowCount(0);
    convergence_data_.clear();

    for (const QJsonValue& val : convergence) {
        QJsonObject ev = val.toObject();

        int row = convergence_table_->rowCount();
        convergence_table_->insertRow(row);

        double time = ev["time"].toDouble();
        QString event = ev["event"].toString();
        int node_id = ev["node_id"].toInt(-1);
        double lb = ev["lb"].toDouble(-1);
        double ub = ev["ub"].toDouble(-1);

        convergence_table_->setItem(row, 0, new QTableWidgetItem(QString::number(time, 'f', 2)));

        // 事件名称中文化
        QString event_zh = event;
        if (event == "integer_found") event_zh = QString::fromUtf8("找到整数解");
        else if (event == "root_done") event_zh = QString::fromUtf8("根节点完成");
        else if (event == "new_ub") event_zh = QString::fromUtf8("更新上界");
        else if (event == "new_lb") event_zh = QString::fromUtf8("更新下界");
        else if (event == "pruned") event_zh = QString::fromUtf8("剪枝");
        else if (event == "start") event_zh = QString::fromUtf8("开始");
        else if (event == "end") event_zh = QString::fromUtf8("结束");
        convergence_table_->setItem(row, 1, new QTableWidgetItem(event_zh));
        convergence_table_->setItem(row, 2, new QTableWidgetItem(
            node_id >= 0 ? QString::number(node_id) : "-"));
        convergence_table_->setItem(row, 3, new QTableWidgetItem(
            lb >= 0 ? QString::number(lb, 'f', 2) : "-"));
        convergence_table_->setItem(row, 4, new QTableWidgetItem(
            (ub >= 0 && ub < 1e10) ? QString::number(ub, 'f', 2) : "-"));

        // 颜色编码
        QColor bg_color = Qt::white;
        if (event == "integer_found") {
            bg_color = QColor(200, 255, 200);  // 浅绿
        } else if (event == "root_done") {
            bg_color = QColor(200, 200, 255);  // 浅蓝
        } else if (event == "end") {
            bg_color = QColor(255, 255, 200);  // 浅黄
        }

        for (int col = 0; col < convergence_table_->columnCount(); col++) {
            if (auto* item = convergence_table_->item(row, col)) {
                item->setBackground(bg_color);
            }
        }

        ConvergencePoint point;
        point.time = time;
        point.event = event;
        point.node_id = node_id;
        point.lb = lb;
        point.ub = ub;
        convergence_data_.push_back(point);
    }
}

void AnalysisWidget::UpdateNodeTable(const QJsonArray& nodes) {
    all_nodes_.clear();

    for (const QJsonValue& val : nodes) {
        QJsonObject node_obj = val.toObject();
        NodeData node;
        node.id = node_obj["id"].toInt();
        node.parent_id = node_obj["parent_id"].toInt();
        node.depth = node_obj["depth"].toInt();
        node.lower_bound = node_obj["lower_bound"].toDouble(-1);
        node.status = node_obj["status"].toString();
        node.branch_type = node_obj["branch_type"].toString();
        node.branch_dir = node_obj["branch_dir"].toString();
        node.cg_iterations = node_obj["cg_iterations"].toInt();
        node.final_y_cols = node_obj["final_y_cols"].toInt();
        node.final_x_cols = node_obj["final_x_cols"].toInt();
        node.is_integer = node_obj["is_integer"].toBool();

        all_nodes_.push_back(node);
    }

    FilterNodeTable();
}

void AnalysisWidget::OnNodeFilterChanged() {
    FilterNodeTable();
}

void AnalysisWidget::FilterNodeTable() {
    QString status_filter = status_filter_combo_->currentData().toString();
    QString branch_filter = branch_type_filter_combo_->currentData().toString();

    node_table_->setRowCount(0);

    for (const auto& node : all_nodes_) {
        // 应用筛选器
        if (!status_filter.isEmpty() && node.status != status_filter) {
            continue;
        }
        if (!branch_filter.isEmpty() && node.branch_type != branch_filter) {
            continue;
        }

        int row = node_table_->rowCount();
        node_table_->insertRow(row);

        node_table_->setItem(row, 0, new QTableWidgetItem(QString::number(node.id)));
        node_table_->setItem(row, 1, new QTableWidgetItem(
            node.parent_id >= 0 ? QString::number(node.parent_id) : "-"));
        node_table_->setItem(row, 2, new QTableWidgetItem(QString::number(node.depth)));
        node_table_->setItem(row, 3, new QTableWidgetItem(
            node.lower_bound >= 0 ? QString::number(node.lower_bound, 'f', 2) : "-"));

        // 状态中文化
        QString status_zh = node.status;
        if (node.status == "branched") status_zh = QString::fromUtf8("已分支");
        else if (node.status == "pruned") status_zh = QString::fromUtf8("已剪枝");
        else if (node.status == "infeasible") status_zh = QString::fromUtf8("不可行");
        else if (node.status == "integer") status_zh = QString::fromUtf8("整数解");
        else if (node.status == "active") status_zh = QString::fromUtf8("活跃");
        else if (node.status == "processed") status_zh = QString::fromUtf8("已处理");
        node_table_->setItem(row, 4, new QTableWidgetItem(status_zh));

        // 分支类型中文化
        QString branch_zh = node.branch_type;
        if (node.branch_type == "sp1_arc") branch_zh = QString::fromUtf8("SP1弧");
        else if (node.branch_type == "sp2_arc") branch_zh = QString::fromUtf8("SP2弧");
        else if (node.branch_type == "none" || node.branch_type.isEmpty()) branch_zh = "-";
        node_table_->setItem(row, 5, new QTableWidgetItem(branch_zh));

        // 方向中文化
        QString dir_zh = node.branch_dir;
        if (node.branch_dir == "left") dir_zh = QString::fromUtf8("左");
        else if (node.branch_dir == "right") dir_zh = QString::fromUtf8("右");
        else if (node.branch_dir.isEmpty()) dir_zh = "-";
        node_table_->setItem(row, 6, new QTableWidgetItem(dir_zh));
        node_table_->setItem(row, 7, new QTableWidgetItem(QString::number(node.cg_iterations)));
        node_table_->setItem(row, 8, new QTableWidgetItem(
            QString("%1/%2").arg(node.final_y_cols).arg(node.final_x_cols)));

        // 状态颜色
        QColor bg_color = Qt::white;
        if (node.status == "integer") {
            bg_color = QColor(200, 255, 200);  // 浅绿
        } else if (node.status == "pruned") {
            bg_color = QColor(220, 220, 220);  // 灰色
        } else if (node.status == "infeasible") {
            bg_color = QColor(255, 200, 200);  // 浅红
        }

        for (int col = 0; col < node_table_->columnCount(); col++) {
            if (auto* item = node_table_->item(row, col)) {
                item->setBackground(bg_color);
            }
        }

        // 存储节点ID用于选择
        node_table_->item(row, 0)->setData(Qt::UserRole, node.id);
    }
}

void AnalysisWidget::OnNodeTableClicked(int row, int column) {
    Q_UNUSED(column);
    if (auto* item = node_table_->item(row, 0)) {
        int node_id = item->data(Qt::UserRole).toInt();
        emit NodeSelected(node_id);
    }
}
