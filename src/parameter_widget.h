// parameter_widget.h - 参数设置面板

#ifndef PARAMETER_WIDGET_H_
#define PARAMETER_WIDGET_H_

#include <QGroupBox>

class QSpinBox;
class QComboBox;
class QPushButton;
class QToolButton;

class ParameterWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit ParameterWidget(QWidget* parent = nullptr);

    int GetTimeLimit() const;      // 时间限制 (秒)
    int GetSP1Method() const;      // SP1 求解方法 (0=CPLEX, 1=ArcFlow, 2=DP)
    int GetSP2Method() const;      // SP2 求解方法 (0=CPLEX, 1=ArcFlow, 2=DP)

public slots:
    void ResetDefaults();

private slots:
    void OnHelpButtonClicked();

private:
    void SetupUi();
    QToolButton* CreateHelpButton(const QString& tooltip,
                                  const QString& detail_title,
                                  const QString& detail_content);

    QSpinBox* time_limit_spin_;
    QComboBox* sp1_method_combo_;
    QComboBox* sp2_method_combo_;
    QPushButton* reset_button_;
};

#endif  // PARAMETER_WIDGET_H_
