// log_widget.cpp - 日志输出面板实现

#include "log_widget.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QDateTime>
#include <QScrollBar>

LogWidget::LogWidget(QWidget* parent)
    : QGroupBox(QString::fromUtf8("运行日志"), parent) {
    auto* layout = new QVBoxLayout(this);

    text_edit_ = new QTextEdit(this);
    text_edit_->setReadOnly(true);
    text_edit_->setFont(QFont("Consolas", 9));
    text_edit_->setLineWrapMode(QTextEdit::NoWrap);

    layout->addWidget(text_edit_);
}

void LogWidget::ClearLog() {
    text_edit_->clear();
}

QString LogWidget::GetLogText() const {
    return text_edit_->toPlainText();
}

void LogWidget::AppendLog(const QString& message) {
    QString timestamp = GetTimestamp();
    text_edit_->append(QString("[%1] %2").arg(timestamp, message));

    // Auto-scroll to bottom
    QScrollBar* scrollbar = text_edit_->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

QString LogWidget::GetTimestamp() const {
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}
