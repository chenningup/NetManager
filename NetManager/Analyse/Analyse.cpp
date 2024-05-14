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
		for (size_t i = 0; i < tmplist.size(); i++)
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
                int contenttype = tmplist[i]->indexOf("Content-Type");
                if (contenttype > 0)
                {

                }
            }
			QByteArray desmac = tmplist[i]->mid(0, 6);
			QByteArray sourmac = tmplist[i]->mid(6, 6);
			int type = tmplist[i]->at(12);
			bool ok;
			QByteArray dstportarray = tmplist[i]->mid(34, 2);
			int dstport = dstportarray.toHex().toInt(&ok, 16);
			if (dstport == 80)
			{
				int contenttype = tmplist[i]->indexOf("Content-Type");
				if (contenttype > 0)
				{
					int jieshu = tmplist[i]->indexOf("\r\n", contenttype);
					if (jieshu)
					{
						QByteArray type = tmplist[i]->mid(contenttype, jieshu - contenttype);
						int typeindex = type.indexOf("image");
						if (typeindex > 0)
						{
							//qDebug() << QString(type);

							//QString houzhui = type.mid(typeindex + 6);
							//int httpend = tmplist[i]->indexOf("\r\n\r\n") + 4;
							//QByteArray picbyte = tmplist[i]->mid(httpend);
							////QString num = QString::number(AnalysisManager::getnum());
							//QFile file(num + "." + houzhui);
							//file.open(QIODevice::WriteOnly);
							//file.write(picbyte);        // write to stderr
							//file.close();
							//QFile file1(num + houzhui);
							//file1.open(QIODevice::WriteOnly);
							//file1.write(*receivedata);        // write to stderr
							//file1.close();
						}
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
