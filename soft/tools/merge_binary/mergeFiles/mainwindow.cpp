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
    else if(arg1.contains("HC32L136K8TA",Qt::CaseInsensitive))
    {
        ui->chipaddr->clear();
        ui->chipaddr->append("MCU:     HC32L136K8TA");
        ui->chipaddr->append("boot：   0x0000 0000 -- 0x0000 07FF");
        ui->chipaddr->append("app:     0x0000 0800 -- 0x0000 7FFF");
        ui->chipaddr->append("upgrade：0x0000 8000 -- 0x0000 F7FF");
    }
    else if(arg1.contains("HC32L176KATA",Qt::CaseInsensitive))
    {
        ui->chipaddr->clear();
        ui->chipaddr->append("MCU:     HC32L176KATA");
        ui->chipaddr->append("boot：   0x0000 0000 -- 0x0000 0FFF");
        ui->chipaddr->append("app:     0x0000 1000 -- 0x0000 FFFF");
        ui->chipaddr->append("upgrade：0x0001 0000 -- 0x0001 FFFF");
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
    unsigned char* pBootArray;
    unsigned char* pAppArray;
    unsigned int BootBinlength;
    unsigned int AppBinlength;
    unsigned int boot_max_sizeofbyte = 0;
    unsigned int app_max_sizeofbyte = 0;

    QByteArray FilePath=boot_file_name.toLocal8Bit();
    FILE* QReadBoot = fopen(FilePath.data(),"rb");

    if(QReadBoot)
    {
        fseek(QReadBoot, 0, SEEK_END);
        BootBinlength = ftell(QReadBoot);

        pBootArray =(unsigned char*)malloc(BootBinlength);
        memset(pBootArray,0,BootBinlength);
        fseek(QReadBoot, 0, SEEK_SET);
        if(BootBinlength!=fread(pBootArray,1,BootBinlength,QReadBoot))  //读取文件到内存
        {
            free(pBootArray);
            fclose(QReadBoot);
            QMessageBox msgBox(QMessageBox::Warning, QString::fromUtf8("Error"),QString::fromUtf8("Boot File sRead Error "), QMessageBox::Ok, this);
                        msgBox.exec();
            return;
        }
        else
        {
            fclose(QReadBoot);  //读取文件完毕，关闭文件
        }
    }
    else
    {
        fclose(QReadBoot);
        QMessageBox msgBox(QMessageBox::Warning, QString::fromUtf8("Error"),QString::fromUtf8("Do'not Open Boot File "), QMessageBox::Ok, this);
                    msgBox.exec();
        return;
    }
//-------------------------------------------------------------读取APP文件
    FilePath.clear();
    FilePath=app_file_name.toLocal8Bit();
    FILE* QReadApp = fopen(FilePath.data(),"rb");


    if(QReadApp)
    {
        fseek(QReadApp, 0, SEEK_END);
        AppBinlength = ftell(QReadApp);


        pAppArray =(unsigned char*)malloc(AppBinlength);
        memset(pAppArray,0,AppBinlength);
        fseek(QReadApp, 0, SEEK_SET);
        if(AppBinlength!=fread(pAppArray,1,AppBinlength,QReadApp))  //读取文件到内存
        {
            free(pAppArray);
            fclose(QReadApp);
            QMessageBox msgBox(QMessageBox::Warning, QString::fromUtf8("Error"),QString::fromUtf8("App File sRead Error "), QMessageBox::Ok, this);
                        msgBox.exec();
            return;
        }
        else
        {
            fclose(QReadApp);  //读取文件完毕，关闭文件
        }
    }
    else
    {
        fclose(QReadApp);
        QMessageBox msgBox(QMessageBox::Warning, QString::fromUtf8("Error"),QString::fromUtf8("Do'not Open App File "), QMessageBox::Ok, this);
                    msgBox.exec();
        return;
    }
//-------------------------------------------------------------------------------------
    unsigned int flash_addr_str = 0;   //根据MCU选择分区大小和保留数据
    unsigned char flash_resver_data = 0;

    rd_mcu_type = ui->comboBox->currentText();
    if(rd_mcu_type.contains("STM32F103RCT6",Qt::CaseInsensitive))
    {
        boot_max_sizeofbyte = 1024*8;
        app_max_sizeofbyte = 1024*108;
        flash_addr_str = 0x08000000;
        flash_resver_data = 0xff;//保留数据为0xff
    }
    else if(rd_mcu_type.contains("STM8L052R8T6",Qt::CaseInsensitive))
    {
        boot_max_sizeofbyte = 1024*2;
        app_max_sizeofbyte = 1024*31;
        flash_addr_str = 0x00008000;
        flash_resver_data = 0x00;//保留数据为0x00
    }
    else if(rd_mcu_type.contains("HC32L136K8TA",Qt::CaseInsensitive))
    {
        boot_max_sizeofbyte = 1024*2;
        app_max_sizeofbyte = 1024*30;
        flash_addr_str = 0x00000000;
        flash_resver_data = 0xff;//保留数据为0xff
    }
    else if(rd_mcu_type.contains("HC32L176KATA",Qt::CaseInsensitive))
    {
        boot_max_sizeofbyte = 1024*4;
        app_max_sizeofbyte = 1024*60;
        flash_addr_str = 0x00000000;
        flash_resver_data = 0xff;//保留数据为0xff
    }
//------------------------------------------------------------------------------------
    QByteArray MergeQbyte;
    mergeBinaryData theMergeBin;
    binaryTransfer_def the_binTransfer;
    bool mergeIsRight;

    the_binTransfer.app_max_size_byte = app_max_sizeofbyte;
    the_binTransfer.boot_max_size_byte = boot_max_sizeofbyte;
    the_binTransfer.in_bootArea_length = BootBinlength;
    the_binTransfer.in_pBootArea_data = pBootArray;
    the_binTransfer.in_appArea_length = AppBinlength;
    the_binTransfer.in_pAppArea_data = pAppArray;

    mergeIsRight = theMergeBin.mergeBinary_createMerge(the_binTransfer,&MergeQbyte,flash_resver_data);
    free(pAppArray);
    free(pBootArray);

    if(false == mergeIsRight)
    {
        QMessageBox msgBox(QMessageBox::Warning, QString::fromUtf8("Error"),QString::fromUtf8("File Length Error "), QMessageBox::Ok, this);
                    msgBox.exec();
        return;
    }
//-------------------------------------------------------------------------------------
    QString HexSting;
    binaryToHexString bToHexString;
    bToHexString.create_hexString(flash_addr_str,MergeQbyte,&HexSting);   //创建HEX格式的字符串
    QByteArray HexStrRaw = HexSting.toUtf8();

    QString MakeFileName;
    QFileInfo fileinf;

    fileinf = QFileInfo(app_file_name);
    MakeFileName = QDir::currentPath();
    MakeFileName.append("/");
    MakeFileName.append(fileinf.fileName());
    MakeFileName.remove(".bin");
    MakeFileName.append(".hex");
    QByteArray Make_Filepath=MakeFileName.toLocal8Bit();
    FILE* QMakebin = fopen(Make_Filepath.data(),"w+b");
    fwrite(HexStrRaw.data(),1,HexStrRaw.size(),QMakebin);
    fclose(QMakebin);
    if(MakeFileName.isNull())
    {
        ui->lineEdit_Merge->setText("This Make is Faild");
    }
    else
    {
        ui->lineEdit_Merge->setText(MakeFileName);
    }
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
