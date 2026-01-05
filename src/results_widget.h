// results_widget.h - 结果显示面板

#ifndef RESULTS_WIDGET_H_
#define RESULTS_WIDGET_H_

#include <QGroupBox>
#include <QString>

class QLabel;

class ResultsWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit ResultsWidget(QWidget* parent = nullptr);

    void ClearResults();
    void SetOptimalValue(int value);
    void SetRootLB(double lb);
    void SetGap(double gap);
    void SetNodeCount(int count);
    void SetUtilization(double util);
    bool HasResults() const;

private:
    void SetupUi();

    QLabel* optimal_value_label_;
    QLabel* root_lb_label_;
    QLabel* gap_label_;
    QLabel* node_count_label_;
    QLabel* utilization_label_;

    bool has_results_;
};

#endif  // RESULTS_WIDGET_H_
