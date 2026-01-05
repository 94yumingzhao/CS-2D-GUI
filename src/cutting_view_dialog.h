// cutting_view_dialog.h - 切割方案可视化独立窗口

#ifndef CUTTING_VIEW_DIALOG_H_
#define CUTTING_VIEW_DIALOG_H_

#include <QDialog>
#include <QString>

class CuttingViewWidget;
class QPushButton;
class QLabel;

class CuttingViewDialog : public QDialog {
    Q_OBJECT

public:
    explicit CuttingViewDialog(QWidget* parent = nullptr);

    // 加载解文件
    bool LoadSolution(const QString& json_path);

public slots:
    void OnOpenFile();
    void OnExportImage();

private:
    void SetupUi();

    CuttingViewWidget* cutting_view_;
    QPushButton* open_file_button_;
    QPushButton* export_image_button_;
    QLabel* file_path_label_;
    QString current_file_path_;
};

#endif  // CUTTING_VIEW_DIALOG_H_
