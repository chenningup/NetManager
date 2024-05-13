#ifndef ARPBUSINESS_H
#define ARPBUSINESS_H

#include <QThread>
#include <QHash>
#include <QStringList>
#include "../NetManager/XuNetInterfaceManager.h"
#include <QMultiHash>
#include "Arp.h"
enum ArpTaskType
{
    ARP_Broadcast,
};
class ArpBusiness :public QThread
{
    Q_OBJECT
public:
    ArpBusiness(/* args */);
    ~ArpBusiness();

    static ArpBusiness &Instance();

    void init();
    void run();

    void addDeceive(const QString &ip,const ArpPack&info);//欺骗
    void deleteDeceive(const QString &ip);//欺骗

    void addDisconnectNet(const QString &ip);//断网
    void deleteDisconnectNet(const QString &ip);//断网

    void addOnitor(const QString &ip);
    void deleteOnitor(const QString &ip);

    void broadcast();

    bool receiveData(std::shared_ptr<QByteArray>data);

signals:
    void findDevice(QString ip,QString mac);
private:
    /* data */
    QHash<QString,QString>mDeviceInfo;
    QHash<QString, ArpPack>mDeceiveHash;
    QStringList mDiconnectNetList;//需要断网列表
    QStringList mOnitorList;//需要监听列表
    XuNetInterfaceManager * mNetmanager;
    bool isStop;
    bool isInit;
};



#endif