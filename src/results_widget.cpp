// results_widget.cpp - 结果显示面板实现

#include "results_widget.h"

#include <QFormLayout>
#include <QLabel>

ResultsWidget::ResultsWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("求解结果"), parent)
    , has_results_(false) {
    SetupUi();
    ClearResults();
}

void ResultsWidget::SetupUi() {
    auto* layout = new QFormLayout(this);

    optimal_value_label_ = new QLabel("--", this);
    optimal_value_label_->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addRow(QString::fromUtf8("最优母板数:"), optimal_value_label_);

    root_lb_label_ = new QLabel("--", this);
    layout->addRow(QString::fromUtf8("根节点下界:"), root_lb_label_);

    gap_label_ = new QLabel("--", this);
    layout->addRow(QString::fromUtf8("最优性 Gap:"), gap_label_);

    node_count_label_ = new QLabel("--", this);
    layout->addRow(QString::fromUtf8("分支节点数:"), node_count_label_);

    utilization_label_ = new QLabel("--", this);
    layout->addRow(QString::fromUtf8("总利用率:"), utilization_label_);
}

void ResultsWidget::ClearResults() {
    optimal_value_label_->setText("--");
    root_lb_label_->setText("--");
    gap_label_->setText("--");
    node_count_label_->setText("--");
    utilization_label_->setText("--");
    has_results_ = false;
}

void ResultsWidget::SetOptimalValue(int value) {
    optimal_value_label_->setText(QString::number(value));
    has_results_ = true;
}

void ResultsWidget::SetRootLB(double lb) {
    root_lb_label_->setText(QString::number(lb, 'f', 4));
}

void ResultsWidget::SetGap(double gap) {
    gap_label_->setText(QString("%1%").arg(gap * 100, 0, 'f', 2));
}

void ResultsWidget::SetNodeCount(int count) {
    node_count_label_->setText(QString::number(count));
}

void ResultsWidget::SetUtilization(double util) {
    utilization_label_->setText(QString("%1%").arg(util * 100, 0, 'f', 2));
}

bool ResultsWidget::HasResults() const {
    return has_results_;
}
