// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// log_widget.h - 日志输出面板

#ifndef LOG_WIDGET_H_
#define LOG_WIDGET_H_

#include <QGroupBox>
#include <QString>

class QTextEdit;
class QPushButton;
class QLabel;
class QTimer;

class LogWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = nullptr);

    void ClearLog();
    QString GetLogText() const;

    // 计时器控制
    void StartTimer();
    void StopTimer();
    void ResetTimer();

public slots:
    void AppendLog(const QString& message);

private slots:
    void UpdateTimerDisplay();

private:
    QString GetTimestamp() const;
    QString FormatElapsedTime() const;

    QTextEdit* text_edit_;
    QPushButton* clear_btn_;
    QLabel* timer_label_;
    QTimer* timer_;
    int elapsed_seconds_;
};

#endif  // LOG_WIDGET_H_
