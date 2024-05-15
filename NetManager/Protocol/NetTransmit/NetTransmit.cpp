
#include "NetTransmit.h"
#include"../../NetManager/XuNetInterfaceManager.h"
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
        if (!mRecDataList.isEmpty())
        {
            mTransmitInfoMutex.lock();
            QList<TransmitInfo>tmpTransmitInfoList = mTransmitInfoList;
            mTransmitInfoMutex.unlock();
            if (tmpTransmitInfoList.isEmpty())
            {
                mRecDataMutex.lock();
                mRecDataList.pop_front();
                mRecDataMutex.unlock();
            }
            else
            {
                mRecDataMutex.lock();
                std::shared_ptr<QByteArray>tmp = mRecDataList[0];
                mRecDataMutex.unlock(); 
                for (size_t i = 0; i < tmpTransmitInfoList.size(); i++)
                {
                    QByteArray desmac = tmp->mid(0, 6);
                    QByteArray sourmac = tmp->mid(6, 6);
                    int type = tmp->at(12);
                    bool ok;
                    QByteArray dstportarray = tmp->mid(36, 2);
                    int dstport = dstportarray.toHex().toInt(&ok, 16);
                    if (desmac == tmpTransmitInfoList[i].mMac && sourmac == tmpTransmitInfoList[i].devMac && type == 8)//
                    {
                        QByteArray sendbyte(tmp->data(), tmp->size());
                        sendbyte.replace(0, 6, tmpTransmitInfoList[i].gatewayMac);
                        sendbyte.replace(6, 6, tmpTransmitInfoList[i].mMac);//
                        XuNetInterfaceManager::Instance()->send((unsigned char *)sendbyte.data(), sendbyte.size());
                    }
                    if (desmac == tmpTransmitInfoList[i].mMac && sourmac == tmpTransmitInfoList[i].gatewayMac && type == 8 && tmp->count() > 34 && (tmp->mid(30, 4) == tmpTransmitInfoList[i].devIP))
                    {
                        QByteArray sendbyte(tmp->data(), tmp->size());
                        sendbyte.replace(0, 6, tmpTransmitInfoList[i].devMac);//
                        sendbyte.replace(6, 6, tmpTransmitInfoList[i].mMac);//
                        XuNetInterfaceManager::Instance()->send((unsigned char*)sendbyte.data(), sendbyte.size());
                    }
                    mRecDataMutex.lock();
                    mRecDataList.pop_front();
                    mRecDataMutex.unlock();
                }
            }
        }
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
    start();
    isInit = true;
}

bool NetTransmit::receiveData(std::shared_ptr<QByteArray> data)
{
    mRecDataMutex.lock();
    mRecDataList.push_back(data);
    mRecDataMutex.unlock();
    return true;
}


void NetTransmit::addTransmitDev(const TransmitInfo& info)
{
    mTransmitInfoMutex.lock();
    mTransmitInfoList.push_back(info);
    mTransmitInfoMutex.unlock();
}
