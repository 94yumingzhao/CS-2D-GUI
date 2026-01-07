// log_widget.cpp - 日志输出面板实现

#include "log_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QDateTime>
#include <QScrollBar>

LogWidget::LogWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("运行日志"), parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(4);

    // Toolbar with clear button
    auto* toolbar = new QHBoxLayout();
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

    connect(clear_btn_, &QPushButton::clicked, this, &LogWidget::ClearLog);
}

void LogWidget::ClearLog() {
    text_edit_->clear();
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
