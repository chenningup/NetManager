
#include "NetTransmit.h"
#include"../../NetManager/XuNetInterfaceManager.h"
#include"../arp/ArpBusiness.h"
#include <QDebug>
bool macStrToByte1(QByteArray& marray, const QString& str)
{
    marray.clear();
    QStringList maclist = str.split(':');
    for (size_t i = 0; i < maclist.size(); i++)
    {
        bool ok;
        int hexnum = maclist[i].toInt(&ok, 16);
        marray.push_back(hexnum);
    }
    return true;
}

bool ipStrToByte1(QByteArray& marray, const QString& str)
{
    marray.clear();
    QStringList maclist = str.split('.');
    for (size_t i = 0; i < maclist.size(); i++)
    {
        bool ok;
        int hexnum = maclist[i].toInt(&ok, 10);
        marray.push_back(hexnum);
    }
    return true;
}
NetTransmit::NetTransmit(/* args */) :isStop(false), isInit(false)
{
}

NetTransmit::~NetTransmit()
{
    isStop = true;
}

NetTransmit &NetTransmit::Instance()
{
    static NetTransmit mNetTransmit;
    return mNetTransmit;
}

void NetTransmit::run()
{
    while (!isStop)
    {
        mRecDataQSem.acquire();
        if (mRecDataList.isEmpty())
        {
            continue;
        }
        NetInterfaceInfo mNetinfo = XuNetInterfaceManager::Instance()->getCurNetfaceInfo();
        mRecDataMutex.lock();
        std::shared_ptr<QByteArray>tmp = mRecDataList[0];
        mRecDataMutex.unlock(); 
        QByteArray desmac = tmp->mid(0, 6);
        QByteArray sourmac = tmp->mid(6, 6);
        int type = tmp->at(12);
        bool ok;
        QByteArray dstportarray = tmp->mid(36, 2);
        int dstport = dstportarray.toHex().toInt(&ok, 16);

        QByteArray mMac;
        macStrToByte1(mMac, mNetinfo.mac);
        QByteArray mGateway;
        macStrToByte1(mGateway, ArpBusiness::Instance().getMacByIp(mNetinfo.gateway));
        QByteArray mIp;
        ipStrToByte1(mIp, mNetinfo.ip);
        if (desmac == mMac && sourmac != mGateway && type == 8)//
        {
            QByteArray sendbyte(tmp->data(), tmp->size());
            sendbyte.replace(0, 6, mGateway);
            sendbyte.replace(6, 6, mMac);//
            XuNetInterfaceManager::Instance()->send((unsigned char*)sendbyte.data(), sendbyte.size());
        }
        QByteArray devIp = tmp->mid(30, 4);
        if (desmac == mMac && sourmac == mGateway && type == 8 && tmp->size() > 34 && (devIp != mIp))
        {
            QByteArray sendbyte(tmp->data(), tmp->size());
            sendbyte.replace(0, 6, ArpBusiness::Instance().getMacByIp(devIp));//
            sendbyte.replace(6, 6, mMac);//
            XuNetInterfaceManager::Instance()->send((unsigned char*)sendbyte.data(), sendbyte.size());
        }
        mRecDataMutex.lock();
        mRecDataList.pop_front();
        qDebug() << "NetTransmit size " << mRecDataList.size();
        mRecDataMutex.unlock();
        
    }
}

void NetTransmit::init()
{
    if (isInit)
    {
        return;
    }
	XuNetInterfaceManager::Instance()->init();
	XuNetInterfaceManager::Instance()->attch(this,std::bind(&NetTransmit::receiveData,this,std::placeholders::_1));
    isInit = true;
}

bool NetTransmit::receiveData(std::shared_ptr<QByteArray> data)
{
    if (!isRunning())
    {
        return false;
    }
    mRecDataMutex.lock();
    mRecDataList.push_back(data);
    mRecDataMutex.unlock();
    mRecDataQSem.release();
    return true;
}


void NetTransmit::addTransmitDev(const TransmitInfo& info)
{
    mTransmitInfoMutex.lock();
    mTransmitInfoList.push_back(info);
    mTransmitInfoMutex.unlock();
}

void NetTransmit::startTransmit()
{
    if (!isRunning())
    {
        isStop = false;
        start();
    }
}

void NetTransmit::endTransmit()
{
    if (!isRunning())
    {
        isStop = false;
    }
}
