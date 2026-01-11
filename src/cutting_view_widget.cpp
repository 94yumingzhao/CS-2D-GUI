// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// cutting_view_widget.cpp - 切割方案可视化组件实现

#include "cutting_view_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <algorithm>
#include <set>

CuttingViewWidget::CuttingViewWidget(QWidget* parent)
    : QWidget(parent)
    , current_plate_index_(0)
    , stock_width_(0)
    , stock_length_(0) {
    SetupUi();
}

void CuttingViewWidget::SetupUi() {
    auto* main_layout = new QVBoxLayout(this);

    // 导航栏
    auto* nav_layout = new QHBoxLayout();

    prev_button_ = new QPushButton("<", this);
    prev_button_->setFixedWidth(40);
    prev_button_->setEnabled(false);
    connect(prev_button_, &QPushButton::clicked, this, &CuttingViewWidget::ShowPrevPlate);

    plate_combo_ = new QComboBox(this);
    plate_combo_->setMinimumWidth(120);
    plate_combo_->setEnabled(false);
    connect(plate_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CuttingViewWidget::OnPlateComboChanged);

    next_button_ = new QPushButton(">", this);
    next_button_->setFixedWidth(40);
    next_button_->setEnabled(false);
    connect(next_button_, &QPushButton::clicked, this, &CuttingViewWidget::ShowNextPlate);

    utilization_label_ = new QLabel(QString::fromUtf8("利用率: --"), this);
    utilization_label_->setAlignment(Qt::AlignRight);

    nav_layout->addWidget(prev_button_);
    nav_layout->addWidget(plate_combo_);
    nav_layout->addWidget(next_button_);
    nav_layout->addStretch();
    nav_layout->addWidget(utilization_label_);

    main_layout->addLayout(nav_layout);
    main_layout->addStretch();  // 绘图区域会占据剩余空间

    setMinimumHeight(250);
}

bool CuttingViewWidget::LoadSolution(const QString& json_path) {
    QFile file(json_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return false;
    }

    QJsonObject root = doc.object();

    // 读取母板尺寸
    QJsonObject stock = root["stock"].toObject();
    stock_width_ = stock["width"].toInt();
    stock_length_ = stock["length"].toInt();

    // 读取子板类型 (用于颜色映射)
    QJsonArray item_types = root["item_types"].toArray();
    color_map_.clear();
    for (int i = 0; i < item_types.size(); i++) {
        color_map_[i + 1] = GetItemColor(i + 1);
    }

    // 读取所有母板
    plates_.clear();
    QJsonArray plates_array = root["plates"].toArray();
    for (const auto& plate_val : plates_array) {
        QJsonObject plate_obj = plate_val.toObject();

        PlateData plate;
        plate.plate_id = plate_obj["plate_id"].toInt();
        plate.utilization = plate_obj["utilization"].toDouble();

        // 读取条带信息
        QJsonArray strips_array = plate_obj["strips"].toArray();
        for (const auto& strip_val : strips_array) {
            QJsonObject strip_obj = strip_val.toObject();

            StripRect strip;
            strip.strip_id = strip_obj["strip_id"].toInt();
            strip.y = strip_obj["y"].toInt();
            strip.width = strip_obj["width"].toInt();

            plate.strips.push_back(strip);
        }

        // 读取子板信息
        QJsonArray items_array = plate_obj["items"].toArray();
        for (const auto& item_val : items_array) {
            QJsonObject item_obj = item_val.toObject();

            ItemRect item;
            item.item_type = item_obj["item_type"].toInt();
            item.x = item_obj["x"].toInt();
            item.y = item_obj["y"].toInt();
            item.width = item_obj["width"].toInt();
            item.length = item_obj["length"].toInt();
            item.strip_id = item_obj["strip_id"].toInt(-1);  // 兼容旧格式

            plate.items.push_back(item);
        }

        plates_.push_back(plate);
    }

    current_plate_index_ = 0;
    UpdateNavigation();
    update();
    return true;
}

void CuttingViewWidget::Clear() {
    plates_.clear();
    current_plate_index_ = 0;
    stock_width_ = 0;
    stock_length_ = 0;
    UpdateNavigation();
    update();
}

void CuttingViewWidget::ShowPrevPlate() {
    if (current_plate_index_ > 0) {
        current_plate_index_--;
        UpdateNavigation();
        update();
        emit PlateChanged(current_plate_index_, static_cast<int>(plates_.size()));
    }
}

void CuttingViewWidget::ShowNextPlate() {
    if (current_plate_index_ < static_cast<int>(plates_.size()) - 1) {
        current_plate_index_++;
        UpdateNavigation();
        update();
        emit PlateChanged(current_plate_index_, static_cast<int>(plates_.size()));
    }
}

void CuttingViewWidget::ShowPlate(int index) {
    if (index >= 0 && index < static_cast<int>(plates_.size())) {
        current_plate_index_ = index;
        UpdateNavigation();
        update();
        emit PlateChanged(current_plate_index_, static_cast<int>(plates_.size()));
    }
}

void CuttingViewWidget::UpdateNavigation() {
    int total = static_cast<int>(plates_.size());

    prev_button_->setEnabled(current_plate_index_ > 0);
    next_button_->setEnabled(current_plate_index_ < total - 1);

    // 更新下拉框 (阻止信号循环)
    plate_combo_->blockSignals(true);
    if (plate_combo_->count() != total) {
        plate_combo_->clear();
        for (int i = 0; i < total; i++) {
            plate_combo_->addItem(QString::fromUtf8("母板 %1/%2").arg(i + 1).arg(total));
        }
    }
    if (total > 0) {
        plate_combo_->setCurrentIndex(current_plate_index_);
        plate_combo_->setEnabled(true);
        utilization_label_->setText(QString::fromUtf8("利用率: %1%")
            .arg(plates_[current_plate_index_].utilization * 100, 0, 'f', 1));
    } else {
        plate_combo_->setEnabled(false);
        utilization_label_->setText(QString::fromUtf8("利用率: --"));
    }
    plate_combo_->blockSignals(false);
}

void CuttingViewWidget::OnPlateComboChanged(int index) {
    if (index >= 0 && index < static_cast<int>(plates_.size())) {
        current_plate_index_ = index;
        prev_button_->setEnabled(current_plate_index_ > 0);
        next_button_->setEnabled(current_plate_index_ < static_cast<int>(plates_.size()) - 1);
        utilization_label_->setText(QString::fromUtf8("利用率: %1%")
            .arg(plates_[current_plate_index_].utilization * 100, 0, 'f', 1));
        update();
        emit PlateChanged(current_plate_index_, static_cast<int>(plates_.size()));
    }
}

QColor CuttingViewWidget::GetItemColor(int item_type) {
    // 使用预定义的调色板
    static const QColor palette[] = {
        QColor(255, 179, 186),  // 粉红
        QColor(255, 223, 186),  // 橙色
        QColor(255, 255, 186),  // 黄色
        QColor(186, 255, 201),  // 浅绿
        QColor(186, 225, 255),  // 浅蓝
        QColor(218, 186, 255),  // 紫色
        QColor(255, 186, 255),  // 粉紫
        QColor(186, 255, 255),  // 青色
        QColor(255, 200, 200),  // 浅红
        QColor(200, 255, 200),  // 浅绿2
        QColor(200, 200, 255),  // 浅蓝2
        QColor(255, 230, 200),  // 浅橙
    };
    int idx = (item_type - 1) % 12;
    return palette[idx];
}

void CuttingViewWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘图区域 (排除导航栏)
    QRect draw_rect = rect();
    draw_rect.setTop(40);  // 留出导航栏空间
    draw_rect.adjust(10, 10, -10, -10);  // 边距

    // 绘制背景
    painter.fillRect(draw_rect, QColor(250, 250, 250));
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(draw_rect);

    if (plates_.empty() || stock_width_ <= 0 || stock_length_ <= 0) {
        // 无数据时显示提示
        painter.setPen(Qt::gray);
        painter.drawText(draw_rect, Qt::AlignCenter,
            QString::fromUtf8("暂无切割方案"));
        return;
    }

    // 绘制当前母板
    DrawPlate(painter, plates_[current_plate_index_], draw_rect);
}

void CuttingViewWidget::DrawPlate(QPainter& painter, const PlateData& plate, const QRect& rect) {
    // 计算缩放比例 (保持宽高比)
    double scale_x = static_cast<double>(rect.width()) / stock_length_;
    double scale_y = static_cast<double>(rect.height()) / stock_width_;
    double scale = std::min(scale_x, scale_y) * 0.95;  // 留 5% 边距

    int offset_x = rect.x() + (rect.width() - static_cast<int>(stock_length_ * scale)) / 2;
    int offset_y = rect.y() + (rect.height() - static_cast<int>(stock_width_ * scale)) / 2;

    // 绘制母板背景
    QRect stock_rect(offset_x, offset_y,
                     static_cast<int>(stock_length_ * scale),
                     static_cast<int>(stock_width_ * scale));
    painter.fillRect(stock_rect, QColor(220, 220, 220));
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(stock_rect);

    // 绘制每个子板
    for (const auto& item : plate.items) {
        int draw_x = offset_x + static_cast<int>(item.x * scale);
        // Y 坐标翻转 (屏幕 Y 轴向下，切割 Y 轴向上)
        int draw_y = offset_y + static_cast<int>((stock_width_ - item.y - item.width) * scale);
        int draw_w = static_cast<int>(item.length * scale);
        int draw_h = static_cast<int>(item.width * scale);

        QRect item_rect(draw_x, draw_y, draw_w, draw_h);

        // 填充颜色
        QColor color = GetItemColor(item.item_type);
        painter.fillRect(item_rect, color);

        // 边框
        painter.setPen(QPen(Qt::black, 1));
        painter.drawRect(item_rect);

        // 子板类型标签 (仅当足够大时显示)
        if (draw_w > 30 && draw_h > 20) {
            painter.setPen(Qt::black);
            QFont font = painter.font();
            font.setPointSize(8);
            painter.setFont(font);
            QString label = QString("T%1").arg(item.item_type);
            painter.drawText(item_rect, Qt::AlignCenter, label);
        }
    }

    // ===== 绘制两阶段切割轨迹 =====
    // 关闭抗锯齿以获得锐利的像素级线条
    painter.setRenderHint(QPainter::Antialiasing, false);

    constexpr int kLineWidth = 3;
    constexpr int kLineHalfWidth = (kLineWidth + 1) / 2;  // = 2

    // 收集红线Y位置 (用于蓝线避让)
    std::set<int> red_line_model_y;

    // 第一阶段切割线 (水平红线，分隔条带)
    if (!plate.strips.empty()) {
        QPen red_pen(QColor(200, 50, 50), kLineWidth, Qt::SolidLine);
        red_pen.setCapStyle(Qt::FlatCap);
        painter.setPen(red_pen);

        for (const auto& strip : plate.strips) {
            int strip_top = strip.y + strip.width;
            if (strip_top < stock_width_) {
                red_line_model_y.insert(strip_top);
                int draw_y = offset_y + static_cast<int>((stock_width_ - strip_top) * scale);
                painter.drawLine(stock_rect.left(), draw_y, stock_rect.right(), draw_y);
            }
        }
    }

    // 第二阶段切割线 (垂直蓝线，分隔同一条带内的子板)
    QPen blue_pen(QColor(50, 100, 180), kLineWidth, Qt::SolidLine);
    blue_pen.setCapStyle(Qt::FlatCap);
    painter.setPen(blue_pen);

    // 按条带分组收集子板右边界
    std::map<int, std::vector<int>> strip_item_boundaries;
    for (const auto& item : plate.items) {
        int right_x = item.x + item.length;
        if (right_x < stock_length_) {
            strip_item_boundaries[item.strip_id].push_back(right_x);
        }
    }

    // 对每个条带绘制垂直切割线
    for (const auto& strip : plate.strips) {
        auto it = strip_item_boundaries.find(strip.strip_id);
        if (it == strip_item_boundaries.end()) continue;

        // Y范围 (屏幕坐标)
        int strip_top_y = offset_y + static_cast<int>((stock_width_ - strip.y - strip.width) * scale);
        int strip_bottom_y = offset_y + static_cast<int>((stock_width_ - strip.y) * scale);

        // 像素级避让: 蓝线端点内缩，不与红线相交
        bool has_red_at_top = red_line_model_y.count(strip.y + strip.width) > 0;
        bool has_red_at_bottom = red_line_model_y.count(strip.y) > 0;

        if (has_red_at_top) {
            strip_top_y += kLineHalfWidth;
        }
        if (has_red_at_bottom) {
            strip_bottom_y -= kLineHalfWidth;
        }

        for (int x : it->second) {
            int draw_x = offset_x + static_cast<int>(x * scale);
            painter.drawLine(draw_x, strip_top_y, draw_x, strip_bottom_y);
        }
    }

    // 恢复抗锯齿 (用于后续文字绘制)
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制尺寸标注
    painter.setPen(Qt::darkGray);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    // 底部: 长度
    QString length_text = QString("%1").arg(stock_length_);
    painter.drawText(stock_rect.x(), stock_rect.bottom() + 15,
                     stock_rect.width(), 15, Qt::AlignCenter, length_text);

    // 右侧: 宽度
    painter.save();
    painter.translate(stock_rect.right() + 15, stock_rect.center().y());
    painter.rotate(-90);
    QString width_text = QString("%1").arg(stock_width_);
    painter.drawText(-30, -5, 60, 15, Qt::AlignCenter, width_text);
    painter.restore();
}

bool CuttingViewWidget::ExportCurrentPlateImage(const QString& path) {
    if (plates_.empty()) return false;

    // 创建图片
    QPixmap pixmap(800, 600);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect draw_rect(20, 20, 760, 560);
    DrawPlate(painter, plates_[current_plate_index_], draw_rect);

    return pixmap.save(path);
}
