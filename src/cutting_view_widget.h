// cutting_view_widget.h - 切割方案可视化组件

#ifndef CUTTING_VIEW_WIDGET_H_
#define CUTTING_VIEW_WIDGET_H_

#include <QWidget>
#include <QString>
#include <QColor>
#include <vector>
#include <map>

class QPushButton;
class QLabel;

// 子板绘制信息
struct ItemRect {
    int item_type;   // 子板类型 (从1开始)
    int x, y;        // 左下角位置
    int width;       // 宽度
    int length;      // 长度
};

// 母板数据
struct PlateData {
    int plate_id;
    double utilization;
    std::vector<ItemRect> items;
};

class CuttingViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit CuttingViewWidget(QWidget* parent = nullptr);

    // 加载 JSON 解文件
    bool LoadSolution(const QString& json_path);
    void Clear();

    // 导出当前母板为图片
    bool ExportCurrentPlateImage(const QString& path);

    // 获取当前状态
    int GetPlateCount() const { return static_cast<int>(plates_.size()); }
    int GetCurrentPlateIndex() const { return current_plate_index_; }

signals:
    void PlateChanged(int index, int total);

public slots:
    void ShowPrevPlate();
    void ShowNextPlate();
    void ShowPlate(int index);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void SetupUi();
    void UpdateNavigation();
    QColor GetItemColor(int item_type);
    void DrawPlate(QPainter& painter, const PlateData& plate, const QRect& rect);

    // UI 组件
    QPushButton* prev_button_;
    QPushButton* next_button_;
    QLabel* plate_index_label_;
    QLabel* utilization_label_;

    // 数据
    std::vector<PlateData> plates_;
    int current_plate_index_;
    int stock_width_;
    int stock_length_;

    // 颜色映射
    std::map<int, QColor> color_map_;
};

#endif  // CUTTING_VIEW_WIDGET_H_
