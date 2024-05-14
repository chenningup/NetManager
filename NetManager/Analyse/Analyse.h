#ifndef ANALYSE_H
#define ANALYSE_H

#include <QObject>
#include <QThread>
#include<QMutex>
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
    bool isStop;
};


























#endif