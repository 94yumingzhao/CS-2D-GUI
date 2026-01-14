// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// log_widget.cpp - 日志输出面板实现

#include "log_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>

LogWidget::LogWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("运行日志"), parent)
    , elapsed_seconds_(0) {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(4);

    // Toolbar with timer and clear button
    auto* toolbar = new QHBoxLayout();

    // 计时器标签
    timer_label_ = new QLabel(QString::fromUtf8("用时: 00:00"), this);
    timer_label_->setFont(QFont("Consolas", 9));
    toolbar->addWidget(timer_label_);

    toolbar->addStretch();
    clear_btn_ = new QPushButton(QString::fromUtf8("清空"), this);
    clear_btn_->setFixedWidth(50);
    toolbar->addWidget(clear_btn_);
    layout->addLayout(toolbar);

    // Log text area
    text_edit_ = new QTextEdit(this);
    text_edit_->setReadOnly(true);
    text_edit_->setFont(QFont("Consolas", 9));
    text_edit_->setLineWrapMode(QTextEdit::NoWrap);
    layout->addWidget(text_edit_);

    // 定时器
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &LogWidget::UpdateTimerDisplay);

    connect(clear_btn_, &QPushButton::clicked, this, &LogWidget::ClearLog);
}

void LogWidget::ClearLog() {
    text_edit_->clear();
    ResetTimer();
}

QString LogWidget::GetLogText() const {
    return text_edit_->toPlainText();
}

void LogWidget::AppendLog(const QString& message) {
    text_edit_->append(message);

    // Auto-scroll to bottom
    QScrollBar* scrollbar = text_edit_->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

QString LogWidget::GetTimestamp() const {
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}

void LogWidget::StartTimer() {
    elapsed_seconds_ = 0;
    timer_label_->setText(FormatElapsedTime());
    timer_->start(1000);  // 每秒更新
}

void LogWidget::StopTimer() {
    timer_->stop();
}

void LogWidget::ResetTimer() {
    timer_->stop();
    elapsed_seconds_ = 0;
    timer_label_->setText(FormatElapsedTime());
}

void LogWidget::UpdateTimerDisplay() {
    elapsed_seconds_++;
    timer_label_->setText(FormatElapsedTime());
}

QString LogWidget::FormatElapsedTime() const {
    int minutes = elapsed_seconds_ / 60;
    int seconds = elapsed_seconds_ % 60;
    return QString::fromUtf8("用时: %1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}
