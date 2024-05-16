#include "Analyse.h"
#include "../NetManager/XuNetInterfaceManager.h"
#include <QFile>
#include<QIODevice>
#include "../Protocol/Tcp/Tcp.h"
#include <QByteArrayView>

void paserIpData(IpData & ipdata,int &length, const QByteArray*bytedata)
{
	memcpy(&ipdata, bytedata->data() + sizeof(LinkLayerData), sizeof(IpData));
	length = ipdata.versionAndHeaderLength & 0X0F;
}

void paserTcpData(TcpData& tcpdata, int& length, int iplength,const QByteArray* bytedata)
{
	memcpy(&tcpdata, bytedata->data() + sizeof(LinkLayerData) + iplength, sizeof(TcpData));
	length = (tcpdata.headerLengthAndFlag[1] & 0XF0) >> 4;
}



void parseIpAndTcpData(IpData &ipdata, int& iplength, TcpData& tcpdata, int& tcplength, const QByteArray* bytedata)
{
	memcpy(&ipdata, bytedata->data() + sizeof(LinkLayerData), sizeof(IpData));
	iplength = ipdata.versionAndHeaderLength & 0X0F;

	memcpy(&tcpdata, bytedata->data() + sizeof(LinkLayerData) + iplength, sizeof(TcpData));
	tcplength = (tcpdata.headerLengthAndFlag[1] & 0XF0) >> 4;
}



void CommunicationPack::addHeader(std::shared_ptr<QByteArray>& pack,int httplength)
{
	if (findHeader)
	{
		return;
	}
	QByteArrayView y("\r\n\r\n");
	int indexend = pack->indexOf(y);
	if(indexend>= 0)
	{
		recHeadFinish = true;
		if(indexend != (pack->size() - 5))
		{
			mHttpData->append(pack->mid(indexend+4));
		}
	}
	mSortData.insert(0, pack);
	findHeader = true;
	mHttpLength = httplength;
}


void CommunicationPack::addPack(std::shared_ptr<QByteArray>& pack)
{
	if (findHeader)
	{
		IpData packIpData = { 0 };
		int packiplength;
		TcpData packTcpData = { 0 };
		int packtcplength;
		parseIpAndTcpData(packIpData, packiplength, packTcpData, packtcplength, pack.get());
		QByteArray tmphttpdata = 
		bool isinsert = false;
		for (int i = 0; i < mSortData.size(); i++)
		{
			std::shared_ptr<QByteArray> tmp = mSortData[i];
			IpData tmpIpData = { 0 };
			int iplength;
			TcpData tmpTcpData = { 0 };
			int tcplength;
			parseIpAndTcpData(tmpIpData, iplength, tmpTcpData, tcplength, tmp.get());
			int needseq = tmpIpData.totalLength - tcplength - iplength + tmpTcpData.seqNum;
			if (packTcpData.seqNum == needseq)
			{
				mSortData.insert(i+1, pack);
				handleFinishRecHttpHeader(pack);
				isinsert = true;
				break;
			}
		}
		if (!isinsert)
		{
			mNoSortData.push_back(pack);
		}
		else
		{
			for (int j = 0; j < mNoSortData.size(); j++)
			{
				IpData nosortIpData = { 0 };
				int nosortiplength;

				TcpData nosortTcpData = { 0 };
				int nosorttcplength;
				parseIpAndTcpData(nosortIpData, nosortiplength, nosortTcpData, nosorttcplength, mNoSortData[j].get());
				for (int i= 0; i < mSortData.size(); i++)
				{
					std::shared_ptr<QByteArray> tmp = mSortData[i];

					IpData tmpIpData = { 0 };
					int  tmpiplength;
					TcpData tmpTcpData = { 0 };
					int tmptcplength;
					parseIpAndTcpData(tmpIpData, tmpiplength, tmpTcpData, tmptcplength, tmp.get());
					int needseq = tmpIpData.totalLength - tmptcplength - tmpiplength + tmpTcpData.seqNum;
					if (nosortTcpData.seqNum == needseq)
					{
						mSortData.insert(i+1, mNoSortData[j]);
						handleFinishRecHttpHeader(mNoSortData[j]);
						mNoSortData.removeAt(j);
						j--;
						break;
					}
				}
			}
		}
		bool isequal = true;
		if (findHeader && findEnd && mNoSortData.isEmpty())
		{
			for (int i = 0; i < mSortData.size() - 1; i++)
			{
				std::shared_ptr<QByteArray> first = mSortData[i];
				std::shared_ptr<QByteArray> second = mSortData[i+1];

				IpData firstpData = { 0 };
				int  firstiplength;
				TcpData firstTcpData = { 0 };
				int tcplength ;
				parseIpAndTcpData(firstpData, firstiplength, firstTcpData, tcplength, first.get());
				int needSeq = firstTcpData.seqNum + firstpData.totalLength - firstiplength - tcplength;

				IpData secondipData = { 0 };
				int  secondiplength;

				TcpData secondTcpData = { 0 };
				int secondtcplength;
				parseIpAndTcpData(secondipData, secondiplength, secondTcpData, secondtcplength, second.get());
				if (needSeq != secondTcpData.seqNum)
				{
					isequal = false;
					break;
				}
			}
		}
		if (isequal)
		{
			if(mHttpLength == mHttpData->size())
			{
				dDebug()<< "receive ok";
				//mHttpData->save();
				ok = true;
			}
			if(mHttpData.mid(mHttpData.size() - 5))
			{

			}
		}
	}
	else
	{
		mNoSortData.push_back(pack);
	}
}

void CommunicationPack::handleFinishRecHttpHeader(std::shared_ptr<QByteArray> &pack)
{
	if(recHeadFinish)
	{
		retunr;
	}
	QByteArray tmphttpdata = pack->mid(sizeof(LinkLayerData)+packiplength+packtcplength);
	QByteArrayView y("\r\n\r\n");
	int indexend = tmphttpdata.indexOf(y);
	if(indexend>= 0)
	{
		recHeadFinish = true;
		if(indexend != (pack->size() - 5))
		{
			mHttpData->append(pack->mid(indexend+4));
		}
	}
}

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
            int tcplength = (tmpTcpData.headerLengthAndFlag[1] & 0XF0) >> 4;


            if(tmpTcpData.sourcePort == 80 )
            {
				QByteArray httpdata = tmplist[i]->mid(sizeof(LinkLayerData) + iplength + tcplength);
				int lastrn = httpdata.lastIndexOf("\r\n");
				if(lastrn != -1)
				{
					QString headParamStr = QString(httpdata.mid(0, lastrn));
					QStringList headParamList= headParamStr.split("\r\n");
					QHash<QString,QString>headerKeyandValue;
					for (size_t i = 0; i < headParamList.size(); i++)
					{
						if(headParamList[i].contains(":"))
						{
							QStringList param   = headParamList[i].split(":");
							if(param.size()==2)
							{
								headerKeyandValue.insert(QString(param[0]),QString(param[1]));
							}
						}
					}

					CommunicationPack* pack = nullptr;
					if (mCommunicationPackHash.contains(tmpTcpData.ackNum))
					{
						pack = mCommunicationPackHash[tmpTcpData.ackNum];
					}
					else
					{
						pack = new CommunicationPack();
						mCommunicationPackHash.insert(tmpTcpData.ackNum, pack);
					}
					//找到了头
					if(headerKeyandValue.contains("Content-Type") && headerKeyandValue["Content-Type"].indexOf("image") > -1)
					{
						int length = 0;
						if (headerKeyandValue.contains("Content-Length"))
						{
							length = headerKeyandValue["Content-Length"].replace(" ","").toInt();
						}
						else
						{
							if (headerKeyandValue.contains("Size"))
							{
								length = headerKeyandValue["Size"].replace(" ", "").toInt();
							}
						}
						pack->addHeader(tmplist[i], length);
					}
					else
					{
						pack->addPack(tmplist[i]);
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

