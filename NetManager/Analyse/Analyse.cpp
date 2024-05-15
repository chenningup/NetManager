#include "Analyse.h"
#include "../NetManager/XuNetInterfaceManager.h"
#include <QFile>
#include<QIODevice>
#include "../Protocol/Tcp/Tcp.h"
Analyse::Analyse() :isInit(false), isStop(false)
{
}

Analyse::~Analyse()
{
	isStop = true;
}

Analyse &Analyse::Instance()
{
    static Analyse mAnalyse;
    return mAnalyse;
}

void Analyse::init()
{
    if (isInit)
	{
		return;
	}	
	XuNetInterfaceManager::Instance()->init();
	XuNetInterfaceManager::Instance()->attch(this,std::bind(&Analyse::receiveData,this,std::placeholders::_1));
	start();
	isInit = true;
}

void Analyse::run()
{
	while (!isStop)
	{
		mRecMutex.lock();
		QList<std::shared_ptr<QByteArray>>tmplist = mRecDataList;
		mRecMutex.unlock();
		for (int i = 0; i < tmplist.size(); i++)
		{
            LinkLayerData tmpLinkLayerData = {0};
            memcpy(&tmpLinkLayerData,tmplist[i]->data(),sizeof(LinkLayerData));

            IpData tmpIpData = {0};
            memcpy(&tmpIpData,tmplist[i]->data()+sizeof(LinkLayerData),sizeof(IpData));
            unsigned char iplength = tmpIpData.versionAndHeaderLength & 0X0F;

            TcpData tmpTcpData = {0};
            memcpy(&tmpIpData,tmplist[i]->data()+sizeof(LinkLayerData)+iplength,sizeof(TcpData));
            int tcplength = (tmpIpData.headerChecksum[1] & 0XF0) >> 4; 


            if(tmpTcpData.sourcePort == 80 )
            {
				QByteArray httpdata = tmplist[i]->mid(sizeof(LinkLayerData) + iplength + tcplength);
				int lastrn = httpdata.lastIndexOf("\r\n");
				if(lastrn != -1)
				{
					QByteArray headParam = httpdata.mid(0,lastrn);
					QList<QByteArray>headerlist = headParam.split("\r\n");
					QHash<QString,QString>headerKeyandValue;
					for (size_t i = 0; i < headerlist.size(); i++)
					{
						if(headerlist[i].contains(":"))
						{
							QList<QByteArray>param   = headerlist[i].split(":");
							if(param.size()==2)
							{
								headerKeyandValue.insert(QString(param[0]),QString(param[1]));
							}
						}
					}

					//找到了头
					if(headerKeyandValue.contains("Content-Type") && headerKeyandValue["Content-Type"].indexOf("image") > -1)
					{
						if(mCommunicationPackHash.contains(tmpTcpData.ackNum))
						{
							
						}
					}
					else
					{
						
					}
				}

            }
		}
		mRecMutex.lock();
		mRecDataList.remove(0, tmplist.size());
		mRecMutex.unlock();
	}
}

bool Analyse::receiveData(std::shared_ptr<QByteArray> data)
{
	mRecMutex.lock();
	mRecDataList.push_back(data);
	mRecMutex.unlock();
    return true;
}
