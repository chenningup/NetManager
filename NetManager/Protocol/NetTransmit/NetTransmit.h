
#ifndef NETTRANSMIT_H
#define NETTRANSMIT_H
#include <QThread>
#include <QList>
#include <QMutex>
#include <QSemaphore>

struct TransmitInfo//ת������Ҫ����Ϣ
{
    QByteArray devMac;//�����豸mac
    QByteArray gatewayMac;//�����豸mac
    QByteArray mMac;//����mac
    QByteArray devIP;//�����豸ip
    QByteArray mIP;//����IP
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
};







#endif
