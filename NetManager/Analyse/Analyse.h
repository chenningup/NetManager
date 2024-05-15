#ifndef ANALYSE_H
#define ANALYSE_H

#include <QObject>
#include <QThread>
#include<QMutex>
#include <QHash>
//因为一个http的请求 会分为很多包
//一个通信包
class CommunicationPack
{
public:
    CommunicationPack()
    {
        findHeader = false;
        findEnd = false;
        ok = false;
        mHttpLength = 0;
    };
    void addHeader(std::shared_ptr<QByteArray>&pack,int httplength = 0);
    //void addEnd(std::shared_ptr<QByteArray>& pack);
    void addPack(std::shared_ptr<QByteArray>& pack);
private:
    bool findHeader;//找到头了
    bool findEnd;//找到尾了
    QList<std::shared_ptr<QByteArray>>mNoSortData;//还没找到位置的包
    QList<std::shared_ptr<QByteArray>>mSortData;//排过序号的包
    bool ok;//全部找齐了
    bool mHttpLength;
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
    QHash<int,CommunicationPack*>mCommunicationPackHash;
    bool isStop;
};


























#endif