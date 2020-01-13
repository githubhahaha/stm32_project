#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    client = new QTcpSocket(this);
    connect(client,SIGNAL(readyRead()),this,SLOT(read_data()));
    client->connectToHost("47.95.10.123",6667);
    if(!client->waitForConnected(30000))
    {
        ui->info->append(tr("连接，请重试\n"));
    }
    ui->info->append(tr("连接成功\n"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString data = ui->data->toPlainText();
    if(data.length() > 0)                //如果输入框不为空才可以发送
    {
        client->write(data.toUtf8());
        QString data1 = "已发送："+data;
        ui->info->append(data1);
    }
    else
    {
        ui->info->append("输入信息");
    }
}


void MainWindow::read_data()
{
    QByteArray msg = client->readAll();
    QString string  = msg;
    QStringList list = string.split(':');
    ui->info->append(string);
    if(list[0]=="loca_gps")
    {
        QString NS=list[1];
        float NS_num = (list[2].toFloat())/100000;
        QString NS_n=QString("%1").arg(NS_num);
        QString WE = list[3];
        float WE_num = (list[4].toFloat())/100000;
        QString WE_n=QString("%1").arg(WE_num);
        ui->GPS->setText(NS+':'+NS_n+';'+ WE +':'+WE_n);
    }
    if(list[0] == req_update){
        sendSize=0;
        QString fileName="/home/zhao/Desktop/ledqt.bin";
        QFileInfo *pcsfileInfo = new QFileInfo(fileName);
         binSize = pcsfileInfo->size ();
        name = pcsfileInfo->fileName();
        QString b = tr("%1").arg(binSize);
        //ui->info->setText(b);
        // QFile* file = new QFile;
        file->setFileName(fileName);
        bool ok = file->open(QIODevice::ReadOnly);

        if(ok)
        {
            ui->info->setText(b);
            QString head = QString("0:%1&%2").arg(name).arg(binSize);
            qint64 len_head = client->write(head.toUtf8(),head.length());
            client->flush();
            if(len_head < 0)
            {
                ui->info->append("头文件没有发送成功");
                file->close();
            }
            else
            {
                ui->info->append("头文件发送成功");
                client->flush();
                for(int i=0;i<1000;i++)
                    for(int j=0;j<100;j++);
                qint64 len = 0;
                do{
                    len = 0;
                    char buf[4*1024] = {0};//每次发送数据大小
                    len = file->read(buf,sizeof(buf));//读文件
                    len = client->write(buf,len);//发文件
                    sendSize += len;
                    ui->info->append("-");
                }while(len > 0);
                if(sendSize != binSize){
                    file->close();
                    ui->info->append("文件未发送完全");
                    return ;
                }
                ui->info->append("文件发送完毕");
                file->close();
            }


        }
    }

}


void MainWindow::on_pushButton_2_clicked()
{
    sendSize=0;
    QString fileName = QFileDialog::getOpenFileName(this,"open", QDir::currentPath());
    // ui->info->setText (fileName);
    QFileInfo *pcsfileInfo = new QFileInfo(fileName);
    binSize = pcsfileInfo->size ();
    name=pcsfileInfo->fileName();
    QString b=tr("%1").arg(binSize);
    //ui->info->setText(b);
    // QFile* file = new QFile;
    file->setFileName(fileName);
    bool ok = file->open(QIODevice::ReadOnly);

    if(ok)
    {
        ui->info->setText(b);
        QString head = QString("0:%1&%2").arg(name).arg(binSize);
        qint64 len_head = client->write(head.toUtf8(),head.length());
        client->flush();
        if(len_head < 0)
        {
            ui->info->append("头文件没有发送成功");
            file->close();
        }
        else
        {
            ui->info->append("头文件发送成功");
            client->flush();
            for(int i=0;i<1000;i++)
                for(int j=0;j<100;j++);
            qint64 len = 0;
            do{
                len = 0;
                char buf[4*1024] = {0};//每次发送数据大小
                len = file->read(buf,sizeof(buf));//读文件
                len = client->write(buf,len);//发文件
                sendSize += len;
                ui->info->append("-");
            }while(len > 0);
            if(sendSize != binSize){
                file->close();
                ui->info->append("文件未发送完全");
                return ;
            }
            ui->info->append("文件发送完毕");
            file->close();
        }


    }
}


void MainWindow::on_check_self_clicked()
{

}

void MainWindow::on_get_local_clicked()
{

}

void MainWindow::on_pushButton_3_clicked()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    // 字符串格式化
    QString timestamp = dateTime.toString("yyyy:MM-dd-hh-mm-ss-zzz\r\n");
    client->write(timestamp.toUtf8());
    QString data1 = "已发送："+ timestamp;
    ui->info->append(data1);

}
