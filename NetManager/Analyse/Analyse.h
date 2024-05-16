#ifndef ANALYSE_H
#define ANALYSE_H

#include <QObject>
#include <QThread>
#include<QMutex>
#include <QHash>
//
//
class CommunicationPack
{
public:
    CommunicationPack()
    {
        findHeader = false;
        findEnd = false;
        ok = false;
        mHttpLength = -1;
        mHttpData = std::shared_ptr<QByteArray> pointer(new QByteArray());
    };
    void addHeader(std::shared_ptr<QByteArray>&pack,int httplength = 0);
    void addPack(std::shared_ptr<QByteArray>& pack);
    void handleFinishRecHttpHeader(std::shared_ptr<QByteArray>& pack);
private:
    bool findHeader;//
    bool recHeadFinish;//http头全部接收完毕
    QList<std::shared_ptr<QByteArray>>mNoSortData;//
    QList<std::shared_ptr<QByteArray>>mSortData;//
    std::shared_ptr<QByteArray>mHttpData;
    bool ok;//
    int mHttpLength;
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