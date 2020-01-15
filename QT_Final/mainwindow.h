#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFileDialog>
#include <QFileInfo>
#include <QDataStream>
#include <QDebug>
#include <QStringList>
#include <QDateTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void read_data();

    void on_pushButton_2_clicked();


    void on_get_local_clicked();



    void on_get_version_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *client;
    QFile* file = new QFile;
    qint64 binSize;
    QString name;
    qint64 sendSize=0;
    QString req_update="update";
    QString version_num="1";
};

#endif // MAINWINDOW_H
