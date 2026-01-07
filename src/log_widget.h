// log_widget.h - 日志输出面板

#ifndef LOG_WIDGET_H_
#define LOG_WIDGET_H_

#include <QGroupBox>
#include <QString>

class QTextEdit;
class QPushButton;

class LogWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = nullptr);

    void ClearLog();
    QString GetLogText() const;

public slots:
    void AppendLog(const QString& message);

private:
    QString GetTimestamp() const;

    QTextEdit* text_edit_;
    QPushButton* clear_btn_;
};

#endif  // LOG_WIDGET_H_
