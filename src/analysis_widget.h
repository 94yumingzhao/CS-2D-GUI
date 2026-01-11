// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// analysis_widget.h - 求解分析页面
// 包含: 求解概要、时间统计、收敛历史、节点列表

#ifndef ANALYSIS_WIDGET_H_
#define ANALYSIS_WIDGET_H_

#include <QWidget>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>

class QLabel;
class QPushButton;
class QTableWidget;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QProgressBar;

// 收敛事件数据
struct ConvergencePoint {
    double time;
    QString event;
    int node_id;
    double lb;
    double ub;
};

// 节点数据
struct NodeData {
    int id;
    int parent_id;
    int depth;
    double lower_bound;
    QString status;
    QString branch_type;
    QString branch_dir;
    int cg_iterations;
    int final_y_cols;
    int final_x_cols;
    bool is_integer;
};

class AnalysisWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisWidget(QWidget* parent = nullptr);

    // 加载JSON分析数据
    bool LoadAnalysisData(const QString& json_path);
    void ClearAnalysisData();

signals:
    void NodeSelected(int node_id);

private slots:
    void OnLoadFile();
    void OnNodeFilterChanged();
    void OnNodeTableClicked(int row, int column);

private:
    void SetupUi();
    void SetupConnections();

    // UI组件创建
    QGroupBox* CreateFileGroup();
    QGroupBox* CreateSummaryGroup();
    QGroupBox* CreateTimeBreakdownGroup();
    QGroupBox* CreateConvergenceGroup();
    QGroupBox* CreateNodeTableGroup();

    // 数据更新
    void UpdateSummary(const QJsonObject& summary, const QJsonObject& branch_stats);
    void UpdateTimeBreakdown(const QJsonObject& time_breakdown);
    void UpdateConvergenceTable(const QJsonArray& convergence);
    void UpdateNodeTable(const QJsonArray& nodes);
    void FilterNodeTable();

    // 文件加载
    QPushButton* load_file_button_;
    QLineEdit* file_path_edit_;

    // 求解概要
    QLabel* solve_status_label_;
    QLabel* objective_label_;
    QLabel* root_lb_label_;
    QLabel* final_lb_label_;
    QLabel* gap_label_;
    QLabel* total_nodes_label_;
    QLabel* pruned_nodes_label_;
    QLabel* infeasible_nodes_label_;
    QLabel* integer_nodes_label_;

    // 时间统计
    QTableWidget* time_table_;
    QLabel* total_time_label_;

    // 收敛历史表
    QTableWidget* convergence_table_;

    // 节点表格
    QTableWidget* node_table_;
    QComboBox* status_filter_combo_;
    QComboBox* branch_type_filter_combo_;

    // 数据存储
    std::vector<NodeData> all_nodes_;
    std::vector<ConvergencePoint> convergence_data_;
    QString current_file_path_;
};

#endif  // ANALYSIS_WIDGET_H_
