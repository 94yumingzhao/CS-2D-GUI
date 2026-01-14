// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// cplex_param_widget.h - CPLEX Parameter Configuration Widget

#ifndef CPLEX_PARAM_WIDGET_H_
#define CPLEX_PARAM_WIDGET_H_

#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

// CPLEX parameter configuration widget
// Displays all CPLEX solver parameters in a compact horizontal layout
class CplexParamWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit CplexParamWidget(QWidget* parent = nullptr);
    ~CplexParamWidget() = default;

    // Master Problem (LP) parameters
    int GetMPThreads() const;
    int GetMPTimeLimit() const;

    // Subproblem (MIP) parameters
    int GetSPThreads() const;
    int GetSPTimeLimit() const;
    int GetSPNodeLimit() const;
    double GetSPMIPGap() const;

    // Node file parameters
    QString GetTempDir() const;
    int GetMemoryLimit() const;
    int GetNodeFileInd() const;

    // Set parameters (for loading saved settings)
    void SetMPThreads(int value);
    void SetMPTimeLimit(int value);
    void SetSPThreads(int value);
    void SetSPTimeLimit(int value);
    void SetSPNodeLimit(int value);
    void SetSPMIPGap(double value);
    void SetTempDir(const QString& path);
    void SetMemoryLimit(int value);
    void SetNodeFileInd(int value);

signals:
    void ParametersChanged();

private slots:
    void OnBrowseTempDir();
    void OnParameterChanged();

private:
    void SetupUI();

    // Master Problem controls
    QSpinBox* mp_threads_;
    QSpinBox* mp_time_limit_;

    // Subproblem controls
    QSpinBox* sp_threads_;
    QSpinBox* sp_time_limit_;
    QSpinBox* sp_node_limit_;
    QDoubleSpinBox* sp_mip_gap_;

    // Node file controls
    QLineEdit* temp_dir_;
    QPushButton* browse_btn_;
    QSpinBox* memory_limit_;
    QComboBox* node_file_strategy_;
};

#endif  // CPLEX_PARAM_WIDGET_H_
