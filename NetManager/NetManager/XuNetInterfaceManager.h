
#ifndef XUNETINTERFACEMANAGER_H
#define XUNETINTERFACEMANAGER_H
#include <QObject>
#include <QHash>
#include <QMutex>
#include <QThread>
#include <functional>
#include <memory>
#include <QByteArray>
struct NetInterfaceInfo
{
	QString ip;
    QString mac;
	QString ipMask;
    QString gateway;
	QString adapterName;
	QString description;
};
typedef std::function<bool(std::shared_ptr<QByteArray>)>dataCallBackfun;
class XuNetInterfaceManager:public QThread
{
    Q_OBJECT
public:
    XuNetInterfaceManager(/* args */);
    virtual  ~XuNetInterfaceManager();

    static XuNetInterfaceManager *Instance();

    virtual bool init() = 0;

    virtual bool open(const QString &name) = 0;

    virtual bool close() = 0;

    virtual bool send(unsigned char * data , int len) = 0;

    virtual QStringList getNetInterfaceDesList() = 0;

    virtual NetInterfaceInfo getCurNetfaceInfo() = 0;

    bool attch(void *object,dataCallBackfun callback);

    bool dettch(void *object,dataCallBackfun callback);

    bool callBackData(const unsigned char *data, int len);

public:
    QMutex mCallBackMutex;
    QHash<void *,dataCallBackfun>mCallBackHash;
    bool isStop;
};

#endif
