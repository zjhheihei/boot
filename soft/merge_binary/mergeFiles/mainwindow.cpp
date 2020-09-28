#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if(arg1.contains("STM32F103RCT6",Qt::CaseInsensitive))
    {
        ui->chipaddr->clear();
        ui->chipaddr->append("MCU:     STM32F103RCT6");
        ui->chipaddr->append("boot：   0x0800 0000 -- 0x0800 1FFF");
        ui->chipaddr->append("app:     0x0800 2000 -- 0x0801 CFFF");
        ui->chipaddr->append("upgrade：0x0801 D000 -- 0x0803 7FFF");
    }
    else if(arg1.contains("STM8L052R8T6",Qt::CaseInsensitive))
    {
        ui->chipaddr->clear();
        ui->chipaddr->append("MCU:     STM8L052R8T6");
        ui->chipaddr->append("boot：   0x00 8000 -- 0x00 07FF");
        ui->chipaddr->append("app:     0x00 8800 -- 0x01 03FF");
        ui->chipaddr->append("upgrade：0x01 0400 -- 0x01 7FFF");
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//创建合并后的hex文件
//-------------------------------------------------------------------------------------
void MainWindow::on_pushButton_CreatMerge_clicked()
{
    QString rd_mcu_type;
//-------------------------------------------------------------------------------------
    if(boot_file_name.isNull())
    {
        QMessageBox msgBox(QMessageBox::Warning, QString::fromUtf8("Error"),QString::fromUtf8("Please select Boot BinFile"), QMessageBox::Ok, this);
                    msgBox.exec();
        return;
    }
    if(app_file_name.isNull())
    {
        QMessageBox msgBox(QMessageBox::Warning, QString::fromUtf8("Error"),QString::fromUtf8("Please select App BinFile"), QMessageBox::Ok, this);
                    msgBox.exec();
        return;
    }
//-------------------------------------------------------------------------------------
    rd_mcu_type = ui->comboBox->currentText();
    if(rd_mcu_type.contains("STM32F103RCT6",Qt::CaseInsensitive))
    {
       ui->lineEdit_Merge->setText("STM32F103RCT6");
    }
    else if(rd_mcu_type.contains("STM8L052R8T6",Qt::CaseInsensitive))
    {
        ui->lineEdit_Merge->setText("STM8L052R8T6");
    }
//------------------------------------------------------------------------------------
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::on_pushButton_OpenBoot_clicked()
{
    boot_file_name = QFileDialog::getOpenFileName(this,
                                                "Open File",
                                                "",
                                                "Binary Files (*.bin);;All Files (*.*?)");
    if(boot_file_name.isNull())
    {
        return;
    }
    ui->lineEdit_Boot->setText(boot_file_name);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MainWindow::on_pushButton_OpenApp_clicked()
{
    app_file_name = QFileDialog::getOpenFileName(this,
                                                "Open File",
                                                "",
                                                "Binary Files (*.bin);;All Files (*.*?)");
    if(app_file_name.isNull())
    {
        return;
    }
    ui->lineEdit_App->setText(app_file_name);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::on_pushButton_OpenFolder_clicked()
{
    QString CurrentDir;
    CurrentDir = QDir::currentPath();
    QDesktopServices::openUrl(CurrentDir);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MainWindow::on_pushButton_ClearMerge_clicked()
{
    QFile::remove(ui->lineEdit_Merge->text());
    ui->lineEdit_Merge->clear();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
