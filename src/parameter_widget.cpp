// parameter_widget.cpp - 参数设置面板实现

#include "parameter_widget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

ParameterWidget::ParameterWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("参数设置"), parent) {
    SetupUi();
    ResetDefaults();
}

void ParameterWidget::SetupUi() {
    auto* main_layout = new QVBoxLayout(this);

    auto* form_layout = new QFormLayout();

    // 时间限制
    time_limit_spin_ = new QSpinBox(this);
    time_limit_spin_->setRange(0, 3600);
    time_limit_spin_->setSuffix(QString::fromUtf8(" 秒"));
    time_limit_spin_->setToolTip(QString::fromUtf8("求解时间限制，0表示无限制"));
    form_layout->addRow(QString::fromUtf8("时间限制:"), time_limit_spin_);

    // SP1 方法
    sp1_method_combo_ = new QComboBox(this);
    sp1_method_combo_->addItem("CPLEX IP", 0);
    sp1_method_combo_->addItem("Arc Flow", 1);
    sp1_method_combo_->addItem("DP", 2);
    sp1_method_combo_->setToolTip(QString::fromUtf8("宽度方向子问题求解方法"));
    form_layout->addRow(QString::fromUtf8("SP1 方法:"), sp1_method_combo_);

    // SP2 方法
    sp2_method_combo_ = new QComboBox(this);
    sp2_method_combo_->addItem("CPLEX IP", 0);
    sp2_method_combo_->addItem("Arc Flow", 1);
    sp2_method_combo_->addItem("DP", 2);
    sp2_method_combo_->setToolTip(QString::fromUtf8("长度方向子问题求解方法"));
    form_layout->addRow(QString::fromUtf8("SP2 方法:"), sp2_method_combo_);

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
