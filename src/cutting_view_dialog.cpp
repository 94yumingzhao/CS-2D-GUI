// cutting_view_dialog.cpp - 切割方案可视化独立窗口实现

#include "cutting_view_dialog.h"
#include "cutting_view_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

CuttingViewDialog::CuttingViewDialog(QWidget* parent)
    : QDialog(parent) {
    SetupUi();
    setWindowTitle(QString::fromUtf8("切割方案可视化"));
    resize(900, 700);
}

void CuttingViewDialog::SetupUi() {
    auto* main_layout = new QVBoxLayout(this);

    // 顶部工具栏
    auto* toolbar_layout = new QHBoxLayout();

    open_file_button_ = new QPushButton(QString::fromUtf8("打开结果文件..."), this);
    connect(open_file_button_, &QPushButton::clicked, this, &CuttingViewDialog::OnOpenFile);

    export_image_button_ = new QPushButton(QString::fromUtf8("导出图片..."), this);
    export_image_button_->setEnabled(false);
    connect(export_image_button_, &QPushButton::clicked, this, &CuttingViewDialog::OnExportImage);

    file_path_label_ = new QLabel(QString::fromUtf8("未选择文件"), this);
    file_path_label_->setStyleSheet("color: gray;");

    toolbar_layout->addWidget(open_file_button_);
    toolbar_layout->addWidget(export_image_button_);
    toolbar_layout->addStretch();
    toolbar_layout->addWidget(file_path_label_);

    main_layout->addLayout(toolbar_layout);

    // 切割可视化组件
    cutting_view_ = new CuttingViewWidget(this);
    cutting_view_->setMinimumSize(600, 500);
    main_layout->addWidget(cutting_view_, 1);
}

bool CuttingViewDialog::LoadSolution(const QString& json_path) {
    if (cutting_view_->LoadSolution(json_path)) {
        current_file_path_ = json_path;
        QFileInfo fi(json_path);
        file_path_label_->setText(fi.fileName());
        file_path_label_->setStyleSheet("color: black;");
        export_image_button_->setEnabled(true);
        return true;
    }
    return false;
}

void CuttingViewDialog::OnOpenFile() {
    QString path = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("选择结果文件"),
        "results",
        QString::fromUtf8("JSON 文件 (*.json);;所有文件 (*)"));

    if (!path.isEmpty()) {
        if (LoadSolution(path)) {
            // 成功加载
        } else {
            QMessageBox::warning(this, QString::fromUtf8("错误"),
                QString::fromUtf8("无法加载文件，请确认格式正确"));
        }
    }
}

void CuttingViewDialog::OnExportImage() {
    if (cutting_view_->GetPlateCount() == 0) {
        QMessageBox::warning(this, QString::fromUtf8("导出"),
            QString::fromUtf8("暂无可导出的切割方案"));
        return;
    }

    QString path = QFileDialog::getSaveFileName(this,
        QString::fromUtf8("导出图片"),
        QString("plate_%1.png").arg(cutting_view_->GetCurrentPlateIndex() + 1),
        QString::fromUtf8("PNG 图片 (*.png);;所有文件 (*)"));

    if (!path.isEmpty()) {
        if (cutting_view_->ExportCurrentPlateImage(path)) {
            QMessageBox::information(this, QString::fromUtf8("导出"),
                QString::fromUtf8("导出成功"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("导出错误"),
                QString::fromUtf8("导出失败"));
        }
    }
}
