#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include<QContextMenuEvent>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init();
    
    void contextMenuEvent(QContextMenuEvent* event);
public slots:
    void onRecArpInfo(QString ip,QString mac);

    void on_openNetInterfaceBtn_clicked();

    void on_arpScan_clicked();

    void stopNetActionSlot();
    void mOnitorActionSlot();
    void mStopActionSlot();
    void mIcmpActionSlot();
private slots:
    void on_globalMoitorBtn_clicked();

private:
    Ui::MainWindow *ui;
    QHash<QString,QString>mDeviceInfoHash;
    QMenu* mMenu;
	QAction* mStopNetAction;
	QAction* mMonitorAction;
	QAction* mStopAction;
	QAction* mIcmpAction;
};
#endif // MAINWINDOW_H
