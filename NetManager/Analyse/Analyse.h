#ifndef ANALYSE_H
#define ANALYSE_H

#include <QObject>
#include <QThread>
#include<QMutex>
//因为一个http的请求 会分为很多包
struct CommunicationPack//一个通信包
{
    bool findHeader;//找到头了
    bool findEnd;//找到尾了
    QList<std::shared_ptr<QByteArray>>mNoSortData;//还没找到位置的包
    QList<std::shared_ptr<QByteArray>>mSortData;//排过序号的包
    CommunicationPack()
    {
        findHeader = false;
        findEnd = false;
    }
};

class Analyse :public QThread
{
    Q_OBJECT
public:
    Analyse(/* args */);
    ~Analyse();

    static Analyse & Instance();

    void init();

    void run();

    bool receiveData(std::shared_ptr<QByteArray> data);
private:
    /* data */
    bool isInit;
    QMutex mRecMutex;
    QList<std::shared_ptr<QByteArray>>mRecDataList;
    QMutex mCommunicationPackMutex;
    QHash<int,CommunicationPack>mCommunicationPackHash;
    bool isStop;
};


























#endif