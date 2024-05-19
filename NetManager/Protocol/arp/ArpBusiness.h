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

    void addDeceive(const QString &ip,const ArpPack&info);//欺骗终端
    void deleteDeceive(const QString &ip);//欺骗终端

    void addDeceiveGateWay(const QString& ip, const ArpPack& info);//欺骗gateway
    void deleteDeceiveGateWay(const QString& ip);//欺骗终端


    void addDisconnectNet(const QString &ip);//断网
    void deleteDisconnectNet(const QString &ip);//断网

    void addOnitor(const QString &ip);
    void deleteOnitor(const QString &ip);
    void globalOnitor();
    void stopGlobalOnitor();

    void broadcast();

    bool receiveData(std::shared_ptr<QByteArray>data);

    QString  getMacByIp(const QString &ip);

    QByteArray  getMacByIp(const QByteArray& ip);
signals:
    void findDevice(QString ip,QString mac);
private:
    /* data */
    QHash<QString,QString>mDeviceInfo;
    QHash<QByteArray, QByteArray>mDeviceInfoByte;
    QHash<QString, ArpPack>mDeceiveHash;
    QHash<QString, ArpPack>mDeceiveGateWayHash;
    QStringList mDiconnectNetList;//需要断网列表
    QStringList mOnitorList;//需要监听列表
    XuNetInterfaceManager * mNetmanager;
    bool isStop;
    bool isInit;
};



#endif