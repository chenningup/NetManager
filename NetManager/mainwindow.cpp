#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./Protocol/arp/ArpBusiness.h"
#include "./NetManager/XuNetInterfaceManager.h"
#include "./Analyse/Analyse.h"
#include<QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    XuNetInterfaceManager::Instance()->init();
    ui->netInterfaceComboBox->addItems(XuNetInterfaceManager::Instance()->getNetInterfaceDesList());
    connect(&ArpBusiness::Instance(),&ArpBusiness::findDevice,this,&MainWindow::onRecArpInfo);
    ArpBusiness::Instance().init();
    Analyse::Instance().init();
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
   	mMenu = new QMenu(ui->tableWidget);
	mStopNetAction = new QAction("断网攻击", ui->tableWidget);
	mMonitorAction = new QAction("监听",ui->tableWidget);
	mStopAction = new QAction("ֹͣ停止", ui->tableWidget);
	mIcmpAction = new QAction("Ping",ui->tableWidget);
	mMenu->addAction(mStopNetAction);
	mMenu->addAction(mMonitorAction);
	mMenu->addAction(mStopAction);
	mMenu->addAction(mIcmpAction);
	connect(mStopNetAction, SIGNAL(triggered()), this, SLOT(stopNetActionSlot()));
	connect(mMonitorAction, SIGNAL(triggered()), this, SLOT(mOnitorActionSlot()));
	connect(mStopAction, SIGNAL(triggered()), this, SLOT(mStopActionSlot()));
	connect(mIcmpAction, SIGNAL(triggered()), this, SLOT(mIcmpActionSlot())); 
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    mMenu->exec(QCursor::pos());
}

void MainWindow::onRecArpInfo(QString ip,QString mac)
{
    if (mDeviceInfoHash.contains(ip))
    {
        return;
    }
    mDeviceInfoHash.insert(ip, mac);
    int rowIndex = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(rowIndex + 1);
    QTableWidgetItem *item = new QTableWidgetItem(ip);
    ui->tableWidget->setItem(rowIndex, 0, item);
    item =new QTableWidgetItem(mac);
    ui->tableWidget->setItem(rowIndex, 1, item);
    qDebug()<<"onRecArpInfo"<<ip<<mac;
}
void MainWindow::on_openNetInterfaceBtn_clicked()
{
    XuNetInterfaceManager::Instance()->open(ui->netInterfaceComboBox->currentText());
}


void MainWindow::on_arpScan_clicked()
{
    mDeviceInfoHash.clear();
    ui->tableWidget->setRowCount(0);
    ArpBusiness::Instance().broadcast();
}

void MainWindow::stopNetActionSlot()
{
    int row = ui->tableWidget->currentRow();
    QTableWidgetItem* item = ui->tableWidget->item(row, 0);
    QString ip = item->text();
    ArpBusiness::Instance().addDisconnectNet(ip);
}

void MainWindow::mOnitorActionSlot()
{
    int row = ui->tableWidget->currentRow();
    QTableWidgetItem* item = ui->tableWidget->item(row, 0);
    QString ip = item->text();
    ArpBusiness::Instance().addOnitor(ip);
}

void MainWindow::mStopActionSlot()
{
    int row = ui->tableWidget->currentRow();
    QTableWidgetItem* item = ui->tableWidget->item(row, 0);
    QString ip = item->text();
    ArpBusiness::Instance().deleteOnitor(ip);
}

void MainWindow::mIcmpActionSlot()
{
    int row = ui->tableWidget->currentRow();
    QTableWidgetItem* item = ui->tableWidget->item(row, 0);
    QString ip = item->text();
}

void MainWindow::on_globalMoitorBtn_clicked()
{
    ArpBusiness::Instance().globalOnitor();
}

