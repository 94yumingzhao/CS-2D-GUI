// parameter_widget.cpp - 参数设置面板实现

#include "parameter_widget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QMessageBox>

ParameterWidget::ParameterWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("参数设置"), parent) {
    SetupUi();
    ResetDefaults();
}

QToolButton* ParameterWidget::CreateHelpButton(const QString& tooltip,
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
    button->setProperty("detail_title", detail_title);
    button->setProperty("detail_content", detail_content);
    connect(button, &QToolButton::clicked, this, &ParameterWidget::OnHelpButtonClicked);
    return button;
}

void ParameterWidget::OnHelpButtonClicked() {
    auto* button = qobject_cast<QToolButton*>(sender());
    if (!button) return;
    QString title = button->property("detail_title").toString();
    QString content = button->property("detail_content").toString();
    QMessageBox::information(this, title, content);
}

void ParameterWidget::SetupUi() {
    auto* main_layout = new QVBoxLayout(this);

    auto* form_layout = new QFormLayout();

    // 时间限制
    auto* time_row = new QHBoxLayout();
    time_limit_spin_ = new QSpinBox(this);
    time_limit_spin_->setRange(0, 3600);
    time_limit_spin_->setSuffix(QString::fromUtf8(" 秒"));
    time_row->addWidget(time_limit_spin_);
    time_row->addWidget(CreateHelpButton(
        QString::fromUtf8("Branch-and-Price算法最大运行时间"),
        QString::fromUtf8("时间限制 (time_limit)"),
        QString::fromUtf8(
            "定义: Branch-and-Price算法的最大运行时间。\n\n"
            "取值范围: 0-3600秒\n"
            "  - 0 = 无限制，直到求解完成\n"
            "  - 推荐值: 60秒\n\n"
            "说明: 超时后返回当前最优解，Gap可能较大。")));
    time_row->addStretch();
    form_layout->addRow(QString::fromUtf8("时间限制:"), time_row);

    // SP1 方法
    auto* sp1_row = new QHBoxLayout();
    sp1_method_combo_ = new QComboBox(this);
    sp1_method_combo_->addItem("CPLEX IP", 0);
    sp1_method_combo_->addItem("Arc Flow", 1);
    sp1_method_combo_->addItem("DP", 2);
    sp1_row->addWidget(sp1_method_combo_);
    sp1_row->addWidget(CreateHelpButton(
        QString::fromUtf8("Stage1宽度方向一维背包求解方法"),
        QString::fromUtf8("SP1方法 (sp1_method)"),
        QString::fromUtf8(
            "定义: 宽度方向定价子问题(SP1)的求解方法。\n\n"
            "可选项:\n"
            "  - CPLEX IP: 通用整数规划，支持复杂约束，较慢\n"
            "  - Arc Flow: 弧流模型，支持弧分支约束，推荐\n"
            "  - DP: 动态规划，极快但不支持弧分支约束\n\n"
            "推荐: Arc Flow (速度与精度平衡)")));
    sp1_row->addStretch();
    form_layout->addRow(QString::fromUtf8("SP1 方法:"), sp1_row);

    // SP2 方法
    auto* sp2_row = new QHBoxLayout();
    sp2_method_combo_ = new QComboBox(this);
    sp2_method_combo_->addItem("CPLEX IP", 0);
    sp2_method_combo_->addItem("Arc Flow", 1);
    sp2_method_combo_->addItem("DP", 2);
    sp2_row->addWidget(sp2_method_combo_);
    sp2_row->addWidget(CreateHelpButton(
        QString::fromUtf8("Stage2长度方向一维背包求解方法"),
        QString::fromUtf8("SP2方法 (sp2_method)"),
        QString::fromUtf8(
            "定义: 长度方向定价子问题(SP2)的求解方法。\n\n"
            "可选项:\n"
            "  - CPLEX IP: 通用整数规划，支持复杂约束，较慢\n"
            "  - Arc Flow: 弧流模型，支持弧分支约束，推荐\n"
            "  - DP: 动态规划，极快但不支持弧分支约束\n\n"
            "推荐: Arc Flow (速度与精度平衡)")));
    sp2_row->addStretch();
    form_layout->addRow(QString::fromUtf8("SP2 方法:"), sp2_row);

    main_layout->addLayout(form_layout);

    // 重置按钮
    auto* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    reset_button_ = new QPushButton(QString::fromUtf8("恢复默认"), this);
    connect(reset_button_, &QPushButton::clicked, this, &ParameterWidget::ResetDefaults);
    button_layout->addWidget(reset_button_);

    main_layout->addLayout(button_layout);
}

void ParameterWidget::ResetDefaults() {
    time_limit_spin_->setValue(60);
    sp1_method_combo_->setCurrentIndex(1);  // Arc Flow
    sp2_method_combo_->setCurrentIndex(1);  // Arc Flow
}

int ParameterWidget::GetTimeLimit() const {
    return time_limit_spin_->value();
}

int ParameterWidget::GetSP1Method() const {
    return sp1_method_combo_->currentData().toInt();
}

int ParameterWidget::GetSP2Method() const {
    return sp2_method_combo_->currentData().toInt();
}
