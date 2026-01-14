// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// cplex_param_widget.cpp - CPLEX Parameter Configuration Widget Implementation

#include "cplex_param_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>

CplexParamWidget::CplexParamWidget(QWidget* parent)
    : QGroupBox(parent) {
    SetupUI();
}

void CplexParamWidget::SetupUI() {
    setTitle(QString::fromUtf8("CPLEX 参数"));

    // Use QGridLayout for strict alignment
    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(12, 8, 12, 8);
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(6);

    // Column indices:
    // 0: row label (主问题/子问题/节点文件)
    // 1: field1 label (线程/临时目录)
    // 2: field1 input
    // 3: field2 label (时限)
    // 4: field2 input
    // 5: field3 label (节点限制/内存上限)
    // 6: field3 input
    // 7: field4 label (相对Gap/存储策略)
    // 8: field4 input

    int row = 0;

    // ==================== Row 0: Master Problem (LP) ====================
    auto* mp_label = new QLabel(QString::fromUtf8("主问题 (LP)"));
    mp_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    grid->addWidget(mp_label, row, 0);

    auto* mp_thread_label = new QLabel(QString::fromUtf8("线程:"));
    mp_thread_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(mp_thread_label, row, 1);

    mp_threads_ = new QSpinBox();
    mp_threads_->setRange(1, 32);
    mp_threads_->setValue(2);
    mp_threads_->setFixedWidth(60);
    grid->addWidget(mp_threads_, row, 2);

    auto* mp_time_label = new QLabel(QString::fromUtf8("时限:"));
    mp_time_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(mp_time_label, row, 3);

    mp_time_limit_ = new QSpinBox();
    mp_time_limit_->setRange(1, 7200);
    mp_time_limit_->setValue(300);
    mp_time_limit_->setSuffix(" s");
    mp_time_limit_->setFixedWidth(80);
    grid->addWidget(mp_time_limit_, row, 4);

    row++;

    // ==================== Row 1: Subproblem (MIP) ====================
    auto* sp_label = new QLabel(QString::fromUtf8("子问题 (MIP)"));
    sp_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    grid->addWidget(sp_label, row, 0);

    auto* sp_thread_label = new QLabel(QString::fromUtf8("线程:"));
    sp_thread_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(sp_thread_label, row, 1);

    sp_threads_ = new QSpinBox();
    sp_threads_->setRange(1, 32);
    sp_threads_->setValue(1);
    sp_threads_->setFixedWidth(60);
    grid->addWidget(sp_threads_, row, 2);

    auto* sp_time_label = new QLabel(QString::fromUtf8("时限:"));
    sp_time_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(sp_time_label, row, 3);

    sp_time_limit_ = new QSpinBox();
    sp_time_limit_->setRange(1, 600);
    sp_time_limit_->setValue(30);
    sp_time_limit_->setSuffix(" s");
    sp_time_limit_->setFixedWidth(80);
    grid->addWidget(sp_time_limit_, row, 4);

    auto* sp_node_label = new QLabel(QString::fromUtf8("节点限制:"));
    sp_node_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(sp_node_label, row, 5);

    sp_node_limit_ = new QSpinBox();
    sp_node_limit_->setRange(0, 999999);
    sp_node_limit_->setValue(5000);
    sp_node_limit_->setFixedWidth(80);
    grid->addWidget(sp_node_limit_, row, 6);

    auto* sp_gap_label = new QLabel(QString::fromUtf8("相对Gap:"));
    sp_gap_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(sp_gap_label, row, 7);

    sp_mip_gap_ = new QDoubleSpinBox();
    sp_mip_gap_->setRange(0.0, 100.0);
    sp_mip_gap_->setValue(1.0);
    sp_mip_gap_->setDecimals(2);
    sp_mip_gap_->setSuffix(" %");
    sp_mip_gap_->setFixedWidth(80);
    grid->addWidget(sp_mip_gap_, row, 8);

    row++;

    // ==================== Row 2: Node File ====================
    auto* nf_label = new QLabel(QString::fromUtf8("节点文件"));
    nf_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    grid->addWidget(nf_label, row, 0);

    auto* nf_dir_label = new QLabel(QString::fromUtf8("临时目录:"));
    nf_dir_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(nf_dir_label, row, 1);

    // temp_dir spans columns 2-4 (aligns with thread input through time_limit input)
    auto* dir_widget = new QWidget();
    auto* dir_layout = new QHBoxLayout(dir_widget);
    dir_layout->setContentsMargins(0, 0, 0, 0);
    dir_layout->setSpacing(4);

    temp_dir_ = new QLineEdit();
    temp_dir_->setText("D:/CPLEX_Temp");
    dir_layout->addWidget(temp_dir_);

    browse_btn_ = new QPushButton("...");
    browse_btn_->setFixedWidth(28);
    dir_layout->addWidget(browse_btn_);

    grid->addWidget(dir_widget, row, 2, 1, 3);  // span columns 2,3,4

    auto* nf_mem_label = new QLabel(QString::fromUtf8("内存上限:"));
    nf_mem_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(nf_mem_label, row, 5);

    memory_limit_ = new QSpinBox();
    memory_limit_->setRange(256, 65536);
    memory_limit_->setValue(2048);
    memory_limit_->setSuffix(" MB");
    memory_limit_->setFixedWidth(80);
    grid->addWidget(memory_limit_, row, 6);

    auto* nf_strategy_label = new QLabel(QString::fromUtf8("存储策略:"));
    nf_strategy_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(nf_strategy_label, row, 7);

    node_file_strategy_ = new QComboBox();
    node_file_strategy_->addItem(QString::fromUtf8("仅内存"), 0);
    node_file_strategy_->addItem(QString::fromUtf8("压缩写盘"), 2);
    node_file_strategy_->addItem(QString::fromUtf8("直接写盘"), 3);
    node_file_strategy_->setCurrentIndex(1);  // Default: compressed
    node_file_strategy_->setFixedWidth(80);
    grid->addWidget(node_file_strategy_, row, 8);

    // Set column stretch
    grid->setColumnStretch(9, 1);  // Add stretch at the end

    // Connect signals
    connect(browse_btn_, &QPushButton::clicked,
            this, &CplexParamWidget::OnBrowseTempDir);

    connect(mp_threads_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CplexParamWidget::OnParameterChanged);
    connect(mp_time_limit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CplexParamWidget::OnParameterChanged);
    connect(sp_threads_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CplexParamWidget::OnParameterChanged);
    connect(sp_time_limit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CplexParamWidget::OnParameterChanged);
    connect(sp_node_limit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CplexParamWidget::OnParameterChanged);
    connect(sp_mip_gap_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CplexParamWidget::OnParameterChanged);
    connect(temp_dir_, &QLineEdit::textChanged,
            this, &CplexParamWidget::OnParameterChanged);
    connect(memory_limit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CplexParamWidget::OnParameterChanged);
    connect(node_file_strategy_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CplexParamWidget::OnParameterChanged);
}

void CplexParamWidget::OnBrowseTempDir() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        QString::fromUtf8("选择CPLEX临时目录"),
        temp_dir_->text(),
        QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) {
        temp_dir_->setText(dir);
    }
}

void CplexParamWidget::OnParameterChanged() {
    emit ParametersChanged();
}

// Getters
int CplexParamWidget::GetMPThreads() const {
    return mp_threads_->value();
}

int CplexParamWidget::GetMPTimeLimit() const {
    return mp_time_limit_->value();
}

int CplexParamWidget::GetSPThreads() const {
    return sp_threads_->value();
}

int CplexParamWidget::GetSPTimeLimit() const {
    return sp_time_limit_->value();
}

int CplexParamWidget::GetSPNodeLimit() const {
    return sp_node_limit_->value();
}

double CplexParamWidget::GetSPMIPGap() const {
    return sp_mip_gap_->value();
}

QString CplexParamWidget::GetTempDir() const {
    return temp_dir_->text();
}

int CplexParamWidget::GetMemoryLimit() const {
    return memory_limit_->value();
}

int CplexParamWidget::GetNodeFileInd() const {
    return node_file_strategy_->currentData().toInt();
}

// Setters
void CplexParamWidget::SetMPThreads(int value) {
    mp_threads_->setValue(value);
}

void CplexParamWidget::SetMPTimeLimit(int value) {
    mp_time_limit_->setValue(value);
}

void CplexParamWidget::SetSPThreads(int value) {
    sp_threads_->setValue(value);
}

void CplexParamWidget::SetSPTimeLimit(int value) {
    sp_time_limit_->setValue(value);
}

void CplexParamWidget::SetSPNodeLimit(int value) {
    sp_node_limit_->setValue(value);
}

void CplexParamWidget::SetSPMIPGap(double value) {
    sp_mip_gap_->setValue(value);
}

void CplexParamWidget::SetTempDir(const QString& path) {
    temp_dir_->setText(path);
}

void CplexParamWidget::SetMemoryLimit(int value) {
    memory_limit_->setValue(value);
}

void CplexParamWidget::SetNodeFileInd(int value) {
    int index = node_file_strategy_->findData(value);
    if (index >= 0) {
        node_file_strategy_->setCurrentIndex(index);
    }
}
