
#ifndef NETTRANSMIT_H
#define NETTRANSMIT_H
#include <QThread>
#include <QList>
#include <QMutex>
#include <QSemaphore>

struct TransmitInfo//
{
    QByteArray devMac;//监听设备mac
    QByteArray gatewayMac;///网关设备mac
    QByteArray mMac;//本机mac
    QByteArray devIP;//监听设备ip
    QByteArray mIP;//本机IP
};

class NetTransmit : public QThread
{
    Q_OBJECT
public:
    NetTransmit(/* args */);
    ~NetTransmit();

    static NetTransmit&Instance();

    void init();

    void run();

    bool receiveData(std::shared_ptr<QByteArray> data);

    void addTransmitDev(const TransmitInfo&info);


private:
    QMutex mRecDataMutex;
    QSemaphore mRecDataQSem;
    QList<std::shared_ptr<QByteArray>>mRecDataList;
    QMutex mTransmitInfoMutex;
    QList<TransmitInfo>mTransmitInfoList;
    bool isStop;
    bool isInit;
};







#endif
