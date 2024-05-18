#ifndef ANALYSE_H
#define ANALYSE_H

#include <QObject>
#include <QThread>
#include<QMutex>
#include <QHash>
#include "../Protocol/Tcp/Tcp.h"
#include <QSemaphore>
#include <QTimer>
#include <QDateTime>
//
//
enum AnalyseTaskType
{
    HttpPicCut
};
class AnalyseTask;
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

    void addAnalyseTask(AnalyseTaskType tasktype, AnalyseTask*task);

    void ipDataToLittle(IpData & data);

    void tcpDataToLittle(TcpData & data);

    void parseIpAndTcpData(IpData& ipdata, int& iplength, TcpData& tcpdata, int& tcplength, const QByteArray* bytedata);

private:
    /* data */
    bool isInit;
    QMutex mRecMutex;
    QSemaphore mRecSem;
    QList<std::shared_ptr<QByteArray>>mRecDataList;
    bool isStop;
    QHash<int,AnalyseTask*>mTaskHash;
};


class CommunicationPack: public QObject
{
    Q_OBJECT
public:
    CommunicationPack()
    {
        findHeader = false;
        mHttpLength = -1;
        std::shared_ptr<QByteArray> pointer(new QByteArray());
        mHttpData = pointer;
        acknum = 0;
        mIsImage = false;
        mCreateTime = QDateTime::currentDateTime();
        isSmoothly = false;
        recHeadFinish = false;
        mIsFoundType = false;
    };
    ~CommunicationPack()
    {
        mNoSortData.clear();
        mSortData.clear();
    };
    void addHeader(std::shared_ptr<QByteArray>&pack);
    void addPack(std::shared_ptr<QByteArray>& pack);
    int tryInsetNoSortPack(std::shared_ptr<QByteArray>& pack);
    void appendHttdData(std::shared_ptr<QByteArray>& pack,int iplength,int tcplength);
    bool isRecFinish();
    bool isTimeOut();
    unsigned int acknum;
    QByteArray getHttpData() { return *mHttpData; };
    bool isImage() { return mIsImage; };
    bool isFoundType() { return mIsFoundType; };
private:
    bool findHeader;//
    bool recHeadFinish;//http头全部接收完毕
    QList<std::shared_ptr<QByteArray>>mNoSortData;//
    QList<std::shared_ptr<QByteArray>>mSortData;//
    std::shared_ptr<QByteArray>mHttpData;
    int mHttpLength;
    bool mIsImage;
    bool isSmoothly;
    bool mIsFoundType;
    QDateTime mCreateTime;
};

class AnalyseTask: public QObject
{
    Q_OBJECT
public:
    AnalyseTask(/* args */);
    virtual ~AnalyseTask();

    virtual void doTask(std::shared_ptr<QByteArray> &data) = 0;
};


class HttpPicCutTask :public AnalyseTask
{
    Q_OBJECT
public:
    HttpPicCutTask (/* args */);
    ~HttpPicCutTask ();
    void doTask(std::shared_ptr<QByteArray> &data);
public slots:
    void recTotalSlot();
private:
    /* data */
    QMutex mCommunicationPackMutex;
    QHash<int,CommunicationPack*>mCommunicationPackHash;
    QTimer mRecTotalTimer;
};























#endif
