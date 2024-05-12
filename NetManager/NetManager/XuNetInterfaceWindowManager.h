
#ifndef XUNETINTERFACEWINDOWMANAGER_H
#define XUNETINTERFACEWINDOWMANAGER_H
#include "XuNetInterfaceManager.h"
#include <QStringList>
#define WIN32 1

#ifdef WIN32
#include "pcap.h"

class XuNetInterfaceWindowManager : public XuNetInterfaceManager
{
    Q_OBJECT
public:
    XuNetInterfaceWindowManager(/* args */);
    ~XuNetInterfaceWindowManager();

    bool init();

    bool open(const QString &name);

    bool close();

    bool send(unsigned char * data , int len);

    QStringList getNetInterfaceDesList();

    NetInterfaceInfo getCurNetfaceInfo();

    void run();
private:
    void getNetInterfaceInfo();
private:
    pcap_t *mCap;
	QHash<QString, NetInterfaceInfo>mNetInterfaceInfoHash;
    NetInterfaceInfo mCurNetInterface;
    char *errbuf;
    bool isinit;
};

#else
#endif
#endif
