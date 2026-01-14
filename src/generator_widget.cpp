// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// generator_widget.cpp - Instance Generator Widget Implementation

#include "generator_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QToolButton>
#include <QMessageBox>

GeneratorWidget::GeneratorWidget(QWidget* parent)
    : QWidget(parent) {
    SetupUi();
    SetupConnections();
    UpdatePreview();
}

void GeneratorWidget::SetupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(8, 8, 8, 8);

    // Mode selector - two buttons
    auto* mode_layout = new QHBoxLayout();
    mode_layout->addWidget(new QLabel(QString::fromUtf8("模式:")));
    quick_mode_btn_ = new QPushButton(QString::fromUtf8("快速"));
    quick_mode_btn_->setCheckable(true);
    quick_mode_btn_->setChecked(true);
    quick_mode_btn_->setFixedWidth(60);
    manual_mode_btn_ = new QPushButton(QString::fromUtf8("手动"));
    manual_mode_btn_->setCheckable(true);
    manual_mode_btn_->setFixedWidth(60);
    mode_layout->addWidget(quick_mode_btn_);
    mode_layout->addWidget(manual_mode_btn_);
    mode_layout->addStretch();
    main_layout->addLayout(mode_layout);

    // Quick mode group
    quick_group_ = new QGroupBox(QString::fromUtf8("快速设置"));
    SetupQuickModeUi(quick_group_);
    main_layout->addWidget(quick_group_);

    // Manual mode group
    manual_group_ = new QGroupBox(QString::fromUtf8("手动设置"));
    SetupManualModeUi(manual_group_);
    manual_group_->setVisible(false);
    main_layout->addWidget(manual_group_);

    // Common settings group
    auto* common_group = new QGroupBox(QString::fromUtf8("输出设置"));
    auto* common_layout = new QFormLayout(common_group);
    common_layout->setSpacing(4);

    seed_spin_ = new QSpinBox();
    seed_spin_->setRange(0, 999999);
    seed_spin_->setValue(0);
    seed_spin_->setSpecialValueText(QString::fromUtf8("自动"));
    common_layout->addRow(QString::fromUtf8("随机种子:"), seed_spin_);

    count_spin_ = new QSpinBox();
    count_spin_->setRange(1, 100);
    count_spin_->setValue(1);
    common_layout->addRow(QString::fromUtf8("生成数量:"), count_spin_);

    auto* path_layout = new QHBoxLayout();
    output_edit_ = new QLineEdit();
    output_edit_->setText("D:/YM-Code/CS-2D-Data/data/");
    browse_button_ = new QPushButton("...");
    browse_button_->setFixedWidth(32);
    path_layout->addWidget(output_edit_);
    path_layout->addWidget(browse_button_);
    common_layout->addRow(QString::fromUtf8("输出路径:"), path_layout);

    main_layout->addWidget(common_group);

    // Preview label
    preview_label_ = new QLabel();
    preview_label_->setStyleSheet("background: #f0f0f0; padding: 8px; font-family: monospace; font-size: 9pt;");
    preview_label_->setWordWrap(true);
    main_layout->addWidget(preview_label_);

    // Generate button
    generate_button_ = new QPushButton(QString::fromUtf8("生成"));
    generate_button_->setMinimumHeight(32);
    generate_button_->setStyleSheet("font-weight: bold;");
    main_layout->addWidget(generate_button_);

    main_layout->addStretch();
}

void GeneratorWidget::SetupQuickModeUi(QGroupBox* group) {
    auto* layout = new QVBoxLayout(group);
    layout->setSpacing(8);

    // Use grid layout for alignment
    auto* grid = new QGridLayout();
    grid->setSpacing(8);

    // Difficulty row
    auto* diff_label = new QLabel(QString::fromUtf8("难度:"));
    grid->addWidget(diff_label, 0, 0, Qt::AlignRight);

    difficulty_button_group_ = new QButtonGroup(this);
    easy_radio_ = new QRadioButton(QString::fromUtf8("简单"));
    medium_radio_ = new QRadioButton(QString::fromUtf8("中等"));
    hard_radio_ = new QRadioButton(QString::fromUtf8("困难"));
    expert_radio_ = new QRadioButton(QString::fromUtf8("专家"));

    difficulty_button_group_->addButton(easy_radio_, 0);
    difficulty_button_group_->addButton(medium_radio_, 1);
    difficulty_button_group_->addButton(hard_radio_, 2);
    difficulty_button_group_->addButton(expert_radio_, 3);
    medium_radio_->setChecked(true);

    grid->addWidget(easy_radio_, 0, 1);
    grid->addWidget(medium_radio_, 0, 2);
    grid->addWidget(hard_radio_, 0, 3);
    grid->addWidget(expert_radio_, 0, 4);

    // Scale row
    auto* scale_label = new QLabel(QString::fromUtf8("规模:"));
    grid->addWidget(scale_label, 1, 0, Qt::AlignRight);

    scale_button_group_ = new QButtonGroup(this);
    small_radio_ = new QRadioButton(QString::fromUtf8("小型"));
    medium_scale_radio_ = new QRadioButton(QString::fromUtf8("中型"));
    large_radio_ = new QRadioButton(QString::fromUtf8("大型"));

    scale_button_group_->addButton(small_radio_, 0);
    scale_button_group_->addButton(medium_scale_radio_, 1);
    scale_button_group_->addButton(large_radio_, 2);
    medium_scale_radio_->setChecked(true);

    grid->addWidget(small_radio_, 1, 1);
    grid->addWidget(medium_scale_radio_, 1, 2);
    grid->addWidget(large_radio_, 1, 3);

    layout->addLayout(grid);
}

void GeneratorWidget::SetupManualModeUi(QGroupBox* group) {
    auto* layout = new QGridLayout(group);
    layout->setSpacing(4);
    layout->setColumnStretch(2, 1);

    int row = 0;

    // Scale parameters row
    auto* scale_layout = new QHBoxLayout();
    num_types_spin_ = new QSpinBox();
    num_types_spin_->setRange(5, 100);
    num_types_spin_->setValue(20);
    scale_layout->addWidget(new QLabel(QString::fromUtf8("种类数:")));
    scale_layout->addWidget(num_types_spin_);

    stock_width_spin_ = new QSpinBox();
    stock_width_spin_->setRange(50, 1000);
    stock_width_spin_->setValue(200);
    scale_layout->addWidget(new QLabel(QString::fromUtf8("母板宽:")));
    scale_layout->addWidget(stock_width_spin_);

    stock_length_spin_ = new QSpinBox();
    stock_length_spin_->setRange(50, 2000);
    stock_length_spin_->setValue(400);
    scale_layout->addWidget(new QLabel(QString::fromUtf8("母板长:")));
    scale_layout->addWidget(stock_length_spin_);

    layout->addWidget(new QLabel(QString::fromUtf8("规模")), row, 0, Qt::AlignRight);
    layout->addLayout(scale_layout, row, 1, 1, 2);
    row++;

    // Min size ratio
    layout->addWidget(new QLabel(QString::fromUtf8("最小尺寸比")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("子板最小面积相对于母板面积的比例"),
        QString::fromUtf8("最小尺寸比 (min_size_ratio)"),
        QString::fromUtf8(
            "定义: 子板最小面积/母板面积。\n\n"
            "取值范围: 0.03 ~ 0.25\n\n"
            "难度影响:\n"
            "  - 0.03: 极小件, 组合空间大, 困难\n"
            "  - 0.08: 中等小件, 平衡\n"
            "  - 0.15: 较大件, 简单\n\n"
            "推荐值: 0.08")), row, 1);
    min_size_ratio_spin_ = new QDoubleSpinBox();
    min_size_ratio_spin_->setRange(0.03, 0.25);
    min_size_ratio_spin_->setSingleStep(0.01);
    min_size_ratio_spin_->setValue(0.08);
    min_size_ratio_spin_->setDecimals(2);
    layout->addWidget(min_size_ratio_spin_, row, 2);
    row++;

    // Max size ratio
    layout->addWidget(new QLabel(QString::fromUtf8("最大尺寸比")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("子板最大面积相对于母板面积的比例"),
        QString::fromUtf8("最大尺寸比 (max_size_ratio)"),
        QString::fromUtf8(
            "定义: 子板最大面积/母板面积。\n\n"
            "取值范围: 0.15 ~ 0.60\n\n"
            "难度影响:\n"
            "  - 0.60: 大件多, 填充率高, 简单\n"
            "  - 0.35: 平衡配置\n"
            "  - 0.20: 小件多, 组合复杂, 困难\n\n"
            "推荐值: 0.35")), row, 1);
    max_size_ratio_spin_ = new QDoubleSpinBox();
    max_size_ratio_spin_->setRange(0.15, 0.60);
    max_size_ratio_spin_->setSingleStep(0.05);
    max_size_ratio_spin_->setValue(0.35);
    max_size_ratio_spin_->setDecimals(2);
    layout->addWidget(max_size_ratio_spin_, row, 2);
    row++;

    // Size CV
    layout->addWidget(new QLabel(QString::fromUtf8("尺寸变异系数")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("子板尺寸的离散程度"),
        QString::fromUtf8("尺寸变异系数 (size_cv)"),
        QString::fromUtf8(
            "定义: 尺寸分布的标准差/均值。\n\n"
            "取值范围: 0.0 ~ 0.8\n\n"
            "难度影响:\n"
            "  - 0.0: 尺寸均匀, 简单\n"
            "  - 0.30: 中等离散\n"
            "  - 0.60+: 高度离散, 困难\n\n"
            "推荐值: 0.30")), row, 1);
    size_cv_spin_ = new QDoubleSpinBox();
    size_cv_spin_->setRange(0.0, 0.8);
    size_cv_spin_->setSingleStep(0.10);
    size_cv_spin_->setValue(0.30);
    size_cv_spin_->setDecimals(2);
    layout->addWidget(size_cv_spin_, row, 2);
    row++;

    // Demand row
    auto* demand_layout = new QHBoxLayout();
    min_demand_spin_ = new QSpinBox();
    min_demand_spin_->setRange(1, 10);
    min_demand_spin_->setValue(1);
    demand_layout->addWidget(new QLabel(QString::fromUtf8("最小需求:")));
    demand_layout->addWidget(min_demand_spin_);

    max_demand_spin_ = new QSpinBox();
    max_demand_spin_->setRange(2, 50);
    max_demand_spin_->setValue(15);
    demand_layout->addWidget(new QLabel(QString::fromUtf8("最大需求:")));
    demand_layout->addWidget(max_demand_spin_);

    layout->addWidget(new QLabel(QString::fromUtf8("需求范围")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("每种子板的需求量范围"),
        QString::fromUtf8("需求范围 (min/max_demand)"),
        QString::fromUtf8(
            "定义: 每种子板的需求量最小/最大值。\n\n"
            "取值范围: 1-10 到 2-50\n\n"
            "难度影响:\n"
            "  - 高需求: 批量效应强, 简单\n"
            "  - 低需求: 组合约束严, 困难\n\n"
            "推荐值: 1-15")), row, 1);
    layout->addLayout(demand_layout, row, 2);
    row++;

    // Demand skew
    layout->addWidget(new QLabel(QString::fromUtf8("需求偏斜度")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("需求分布的偏态程度"),
        QString::fromUtf8("需求偏斜度 (demand_skew)"),
        QString::fromUtf8(
            "定义: 需求分布的偏态系数。\n\n"
            "取值范围: 0.0 ~ 1.0\n"
            "  - 0.0: 均匀分布\n"
            "  - 0.5: 中等偏态\n"
            "  - 1.0: 高度偏态(少数高需求)\n\n"
            "推荐值: 0.0-0.2")), row, 1);
    demand_skew_spin_ = new QDoubleSpinBox();
    demand_skew_spin_->setRange(0.0, 1.0);
    demand_skew_spin_->setSingleStep(0.1);
    demand_skew_spin_->setValue(0.0);
    demand_skew_spin_->setDecimals(2);
    layout->addWidget(demand_skew_spin_, row, 2);
    row++;

    // Peak ratio
    layout->addWidget(new QLabel(QString::fromUtf8("热门比例")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("高需求热门子板的比例"),
        QString::fromUtf8("热门比例 (peak_ratio)"),
        QString::fromUtf8(
            "定义: 需求量翻倍的'热门'子板比例。\n\n"
            "取值范围: 0.0 ~ 0.3\n"
            "  - 0.0: 无热门子板\n"
            "  - 0.1: 10%为热门\n"
            "  - 0.3: 30%为热门\n\n"
            "推荐值: 0.0-0.1")), row, 1);
    peak_ratio_spin_ = new QDoubleSpinBox();
    peak_ratio_spin_->setRange(0.0, 0.3);
    peak_ratio_spin_->setSingleStep(0.05);
    peak_ratio_spin_->setValue(0.0);
    peak_ratio_spin_->setDecimals(2);
    layout->addWidget(peak_ratio_spin_, row, 2);
    row++;

    // Num clusters
    layout->addWidget(new QLabel(QString::fromUtf8("尺寸聚类数")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("子板尺寸的聚类分组数"),
        QString::fromUtf8("尺寸聚类数 (num_clusters)"),
        QString::fromUtf8(
            "定义: 将子板尺寸聚集为几个规格群。\n\n"
            "取值范围: 0-5\n"
            "  - 0: 不使用聚类\n"
            "  - 2-3: 少量规格群\n"
            "  - 4-5: 多规格群\n\n"
            "影响: 聚类可能简化或复杂化问题。\n\n"
            "推荐值: 0")), row, 1);
    num_clusters_spin_ = new QSpinBox();
    num_clusters_spin_->setRange(0, 5);
    num_clusters_spin_->setValue(0);
    num_clusters_spin_->setSpecialValueText(QString::fromUtf8("不使用"));
    layout->addWidget(num_clusters_spin_, row, 2);
    row++;

    // Prime offset
    layout->addWidget(new QLabel(QString::fromUtf8("质数偏移")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("使用质数偏移增加填充难度"),
        QString::fromUtf8("质数偏移 (prime_offset)"),
        QString::fromUtf8(
            "定义: 对尺寸添加质数偏移,\n"
            "使其不易被母板尺寸整除。\n\n"
            "取值: 开/关\n\n"
            "难度影响:\n"
            "  - 关: 正常尺寸\n"
            "  - 开: 难以完美填充, 困难+\n\n"
            "推荐值: 关(一般), 开(专家)")), row, 1);
    prime_offset_check_ = new QCheckBox();
    prime_offset_check_->setChecked(false);
    layout->addWidget(prime_offset_check_, row, 2);
    row++;

    // Strategy
    layout->addWidget(new QLabel(QString::fromUtf8("生成策略")), row, 0, Qt::AlignRight);
    layout->addWidget(CreateHelpButton(
        QString::fromUtf8("算例生成的内部策略"),
        QString::fromUtf8("生成策略 (strategy)"),
        QString::fromUtf8(
            "定义: 内部生成算法的选择。\n\n"
            "可选项:\n"
            "  - 逆向: 已知最优解构造\n"
            "  - 随机: 参数化随机生成\n"
            "  - 聚类: 尺寸分群生成\n"
            "  - 残差: 难填充结构生成\n\n"
            "推荐值: 随机")), row, 1);
    strategy_combo_ = new QComboBox();
    strategy_combo_->addItem(QString::fromUtf8("逆向"), 0);
    strategy_combo_->addItem(QString::fromUtf8("随机"), 1);
    strategy_combo_->addItem(QString::fromUtf8("聚类"), 2);
    strategy_combo_->addItem(QString::fromUtf8("残差"), 3);
    strategy_combo_->setCurrentIndex(1);  // Default: random
    layout->addWidget(strategy_combo_, row, 2);
}

void GeneratorWidget::SetupConnections() {
    connect(quick_mode_btn_, &QPushButton::clicked,
            this, &GeneratorWidget::OnModeChanged);
    connect(manual_mode_btn_, &QPushButton::clicked,
            this, &GeneratorWidget::OnModeChanged);

    connect(difficulty_button_group_, &QButtonGroup::idClicked,
            this, &GeneratorWidget::OnDifficultyChanged);
    connect(scale_button_group_, &QButtonGroup::idClicked,
            this, &GeneratorWidget::OnScaleChanged);

    connect(browse_button_, &QPushButton::clicked,
            this, &GeneratorWidget::OnBrowseOutput);
    connect(generate_button_, &QPushButton::clicked,
            this, &GeneratorWidget::OnGenerateClicked);

    // Manual mode value changes
    connect(num_types_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(stock_width_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(stock_length_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(min_size_ratio_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(max_size_ratio_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
    connect(size_cv_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GeneratorWidget::UpdatePreview);
}

void GeneratorWidget::OnModeChanged() {
    // Toggle buttons - ensure only one is checked
    QPushButton* sender_btn = qobject_cast<QPushButton*>(sender());
    if (sender_btn == quick_mode_btn_) {
        quick_mode_btn_->setChecked(true);
        manual_mode_btn_->setChecked(false);
    } else {
        quick_mode_btn_->setChecked(false);
        manual_mode_btn_->setChecked(true);
    }

    bool quick = quick_mode_btn_->isChecked();
    quick_group_->setVisible(quick);
    manual_group_->setVisible(!quick);
    UpdatePreview();
}

void GeneratorWidget::OnDifficultyChanged() {
    UpdatePreview();
}

void GeneratorWidget::OnScaleChanged() {
    UpdatePreview();
}

void GeneratorWidget::OnBrowseOutput() {
    QString path = QFileDialog::getExistingDirectory(this,
        QString::fromUtf8("选择输出目录"),
        output_edit_->text());

    if (!path.isEmpty()) {
        if (!path.endsWith('/') && !path.endsWith('\\')) {
            path += '/';
        }
        output_edit_->setText(path);
    }
}

void GeneratorWidget::OnGenerateClicked() {
    GeneratorConfig config = GetConfig();
    emit GenerateRequested(config);
}

void GeneratorWidget::UpdatePreview() {
    GeneratorConfig config = GetConfig();
    double score = DifficultyMapper::EstimateDifficultyScore(config);
    QString gap = DifficultyMapper::EstimateGap(config);
    double util = DifficultyMapper::EstimateUtilization(config);

    QString preview = QString::fromUtf8(
        "种类数 = %1  母板尺寸 = %2 x %3\n"
        "尺寸比例范围 = %4 ~ %5\n"
        "需求范围 = %6 ~ %7\n"
        "难度评分 = %8\n"
        "预估Gap = %9\n"
        "预估利用率下界 = %10%")
        .arg(config.num_types)
        .arg(config.stock_width)
        .arg(config.stock_length)
        .arg(config.min_size_ratio, 0, 'f', 2)
        .arg(config.max_size_ratio, 0, 'f', 2)
        .arg(config.min_demand)
        .arg(config.max_demand)
        .arg(score, 0, 'f', 1)
        .arg(gap)
        .arg(util * 100, 0, 'f', 1);

    preview_label_->setText(preview);
    emit ConfigChanged();
}

GeneratorConfig GeneratorWidget::GetConfig() const {
    GeneratorConfig config;

    if (quick_mode_btn_->isChecked()) {
        // Quick mode - use presets
        int diff_id = difficulty_button_group_->checkedId();
        int scale_id = scale_button_group_->checkedId();

        DifficultyLevel diff = static_cast<DifficultyLevel>(diff_id);
        ScaleLevel scale = static_cast<ScaleLevel>(scale_id);

        config = DifficultyMapper::GetPreset(diff, scale);
    } else {
        // Manual mode
        config.num_types = num_types_spin_->value();
        config.stock_width = stock_width_spin_->value();
        config.stock_length = stock_length_spin_->value();
        config.min_size_ratio = min_size_ratio_spin_->value();
        config.max_size_ratio = max_size_ratio_spin_->value();
        config.size_cv = size_cv_spin_->value();
        config.min_demand = min_demand_spin_->value();
        config.max_demand = max_demand_spin_->value();
        config.demand_skew = demand_skew_spin_->value();
        config.prime_offset = prime_offset_check_->isChecked();
        config.num_clusters = num_clusters_spin_->value();
        config.peak_ratio = peak_ratio_spin_->value();
        config.strategy = strategy_combo_->currentData().toInt();
    }

    // Common settings
    config.seed = seed_spin_->value();
    config.count = count_spin_->value();
    config.output_path = output_edit_->text();

    return config;
}

void GeneratorWidget::SetOutputPath(const QString& path) {
    output_edit_->setText(path);
}

void GeneratorWidget::ApplyPreset(const GeneratorConfig& config) {
    num_types_spin_->setValue(config.num_types);
    stock_width_spin_->setValue(config.stock_width);
    stock_length_spin_->setValue(config.stock_length);
    min_size_ratio_spin_->setValue(config.min_size_ratio);
    max_size_ratio_spin_->setValue(config.max_size_ratio);
    size_cv_spin_->setValue(config.size_cv);
    min_demand_spin_->setValue(config.min_demand);
    max_demand_spin_->setValue(config.max_demand);
    demand_skew_spin_->setValue(config.demand_skew);
    prime_offset_check_->setChecked(config.prime_offset);
    num_clusters_spin_->setValue(config.num_clusters);
    peak_ratio_spin_->setValue(config.peak_ratio);
    strategy_combo_->setCurrentIndex(config.strategy);
}

QToolButton* GeneratorWidget::CreateHelpButton(const QString& tooltip,
                                                const QString& detail_title,
                                                const QString& detail_content) {
    auto* button = new QToolButton(this);
    button->setText("?");
    button->setFixedSize(18, 18);
    button->setToolTip(tooltip);
    button->setStyleSheet(
        "QToolButton { border: 1px solid #999; border-radius: 9px; "
        "background: #f0f0f0; font-weight: bold; font-size: 10px; }"
        "QToolButton:hover { background: #e0e0e0; }");

    // Store detail info as dynamic properties
    button->setProperty("detail_title", detail_title);
    button->setProperty("detail_content", detail_content);

    connect(button, &QToolButton::clicked, this, &GeneratorWidget::OnHelpButtonClicked);

    return button;
}

void GeneratorWidget::OnHelpButtonClicked() {
    auto* button = qobject_cast<QToolButton*>(sender());
    if (!button) return;

    QString title = button->property("detail_title").toString();
    QString content = button->property("detail_content").toString();

    QMessageBox::information(this, title, content);
}
