#include "Analyse.h"
#include "../NetManager/XuNetInterfaceManager.h"
#include <QFile>
#include<QIODevice>

#include <QByteArrayView>
#include <QDebug>
#include <QApplication>
void bigToLittleUnsignedInt(unsigned int & pararm)
{
	// 1111 1111 1111 1111 1111  1111  1111  1111
	// 1111 1111 1111 1111 0000 0000  0000 0000
	
	// 1111 1111  1111 1111 1111  1111  0000 0000
	// 0000 0000 1111 1111 1111  1111  1111  1111

	// 1111 1111  1111 1111 1111  1111  0000 0000
	// 0000 0000 1111 1111 1111  1111  1111  1111

	unsigned int first = (((pararm << 8) & 0XFF000000) | ( (pararm >> 8) & 0X00FF0000 ) );
	unsigned int second = (((pararm << 8) & 0X0000FF00) | ((pararm >> 8)  & 0X000000FF));
	pararm = (first >> 16) | (second << 16);
}
void bigToLittleUnsignedShortInt(unsigned short int & pararm)
{
	// 1111 1111
	// 0000 1111
	// 1111 0000
	unsigned short int first = pararm << 8;
	unsigned short int second = pararm >> 8;
	pararm = first | second;
}

void bigToLittle(unsigned char*pararm ,int size)
{
	unsigned char*tmp = new  unsigned char[size];
	int index = 0;
	for (int i = size-1; i >=0; i--)
	{
		tmp[index] = pararm[i];
		index++;
	}
	memcpy(pararm,tmp,size);
}


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



void Analyse::parseIpAndTcpData(IpData &ipdata, int& iplength, TcpData& tcpdata, int& tcplength, const QByteArray* bytedata)
{
    int size =sizeof(LinkLayerData);
	memcpy(&ipdata, bytedata->data() + sizeof(LinkLayerData), sizeof(IpData));
	ipDataToLittle(ipdata);
    iplength = (ipdata.versionAndHeaderLength & 0X0F) * 4;

    if(ipdata.protocol != 6)
    {
        return;
    }
	memcpy(&tcpdata, bytedata->data() + sizeof(LinkLayerData) + iplength, sizeof(TcpData));
	tcpDataToLittle(tcpdata);
    tcplength = (tcpdata.headerLengthAndFlag[0] >> 4) * 4;
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
	//addAnalyseTask(HttpPicCut,new HttpPicCutTask());
}

void Analyse::run()
{
	while (!isStop)
	{
        mRecSem.acquire();
        if(mRecDataList.isEmpty())
        {
            continue;
        }
		mRecMutex.lock();
        std::shared_ptr<QByteArray>tmp = mRecDataList[0];
		mRecMutex.unlock();
        if(!mTaskHash.empty())
        {
            QHash<int,AnalyseTask*>::iterator iter = mTaskHash.begin();
            while (iter != mTaskHash.end())
            {
                iter.value()->doTask(tmp);
                iter++;
            }
        }
		mRecMutex.lock();
        mRecDataList.pop_front();
		qDebug() << "Analyse size " << mRecDataList.size();
		mRecMutex.unlock();
	}
}

bool Analyse::receiveData(std::shared_ptr<QByteArray> data)
{
	mRecMutex.lock();
	mRecDataList.push_back(data);
	mRecMutex.unlock();
    mRecSem.release();
    return true;
}

void Analyse::addAnalyseTask(AnalyseTaskType tasktype, AnalyseTask*task)
{
    if(!mTaskHash.contains(tasktype))
	{
        mTaskHash.insert(tasktype,task);
	}
}

void Analyse::ipDataToLittle(IpData &data)
{
	bigToLittleUnsignedShortInt(data.totalLength);
	bigToLittleUnsignedShortInt(data.identification);
	bigToLittle(data.flagAndOffset,2);
	bigToLittle(data.headerChecksum,2);
	bigToLittle(data.sourceIp,4);
	bigToLittle(data.disIp,4);
}

void Analyse::tcpDataToLittle(TcpData &data)
{
	bigToLittleUnsignedShortInt(data.sourcePort);
	bigToLittleUnsignedShortInt(data.distPort);
	bigToLittleUnsignedInt(data.seqNum);
	bigToLittleUnsignedInt(data.ackNum);
	bigToLittleUnsignedShortInt(data.window);
	bigToLittleUnsignedShortInt(data.checksum);
	bigToLittleUnsignedShortInt(data.urgentPtr);
}




void CommunicationPack::addHeader(std::shared_ptr<QByteArray>& pack)
{
	if (findHeader)
	{
		return;
	}
	// QByteArrayView y("\r\n\r\n");
	// int indexend = pack->indexOf(y);
	// if(indexend>= 0)
	// {
	// 	recHeadFinish = true;
	// 	if(indexend != (pack->size() - 5))
	// 	{
	// 		mHttpData->append(pack->mid(indexend+4));
	// 	}
	// }
	findHeader = true;
	IpData tmpIpData = { 0 };
    int iplength;
    TcpData tmpTcpData = { 0 };
    int tcplength;
    Analyse::Instance().parseIpAndTcpData(tmpIpData, iplength, tmpTcpData, tcplength, pack.get());
	QByteArray httpdata = pack->mid(sizeof(LinkLayerData) + iplength + tcplength);
	//qDebug() << "xcn  addHeader  ack " << acknum;
	int lastrn = httpdata.lastIndexOf("\r\n\r\n");
	if (lastrn < 0)
	{
		lastrn = httpdata.lastIndexOf("\r\n");
	}
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
		if(headerKeyandValue.contains("Content-Type") )
		{
			if (headerKeyandValue["Content-Type"].indexOf("image") > -1)
			{
				mIsImage = true;
			}
			mIsFoundType = true;
		}
		if (headerKeyandValue.contains("Content-Length"))
		{
			mHttpLength = headerKeyandValue["Content-Length"].replace(" ","").toInt();
		}
		else
		{
			if (headerKeyandValue.contains("Size"))
			{
				mHttpLength = headerKeyandValue["Size"].replace(" ", "").toInt();
			}
		}
	}
	appendHttdData(pack, iplength, tcplength);
	mSortData.insert(0, pack);
	//qDebug() << "xcn  addHeader  totalsize  "<< mHttpLength;
}


void CommunicationPack::addPack(std::shared_ptr<QByteArray>& pack)
{
	//qDebug() << "xcn  addPack  ack " << acknum << "findHeader:" << findHeader;
	if(mIsFoundType)
	{
		if(!isImage())
		{
			return;
		}
	}
	if (findHeader)
	{
		// 1为插入成功  -1 为已经有了，0为没有插入成功
		int insert = tryInsetNoSortPack(pack);
        switch (insert)
        {
        case -1:

            break;
        case 0:
            mNoSortData.push_back(pack);
            break;
        case 1:
        {
            for (int j = 0; j < mNoSortData.size(); j++)
            {
                int res =tryInsetNoSortPack(mNoSortData[j]);
                if(res == 1)
                {
                    mNoSortData.removeAt(j);
                    j--;
                }
            }
        }
            break;
        default:
            break;
        }
		bool isequal = true;
		if (findHeader  && mNoSortData.isEmpty())
		{
			for (int i = 0; i < mSortData.size() - 1; i++)
			{
				std::shared_ptr<QByteArray> first = mSortData[i];
				std::shared_ptr<QByteArray> second = mSortData[i+1];

				IpData firstpData = { 0 };
				int  firstiplength;
				TcpData firstTcpData = { 0 };
				int tcplength ;
				Analyse::Instance().parseIpAndTcpData(firstpData, firstiplength, firstTcpData, tcplength, first.get());
				int needSeq = firstTcpData.seqNum + firstpData.totalLength - firstiplength - tcplength;

				IpData secondipData = { 0 };
				int  secondiplength;

				TcpData secondTcpData = { 0 };
				int secondtcplength;
				Analyse::Instance().parseIpAndTcpData(secondipData, secondiplength, secondTcpData, secondtcplength, second.get());
				if (needSeq != secondTcpData.seqNum)
				{
                    isequal = false;
					break;
				}
			}
		}
		isSmoothly = isequal;
	}
	else
	{
		mNoSortData.push_back(pack);
	}
}

int  CommunicationPack::tryInsetNoSortPack(std::shared_ptr<QByteArray>& pack)
{
	IpData packIpData = { 0 };
	int packiplength;
	TcpData packTcpData = { 0 };
	int packtcplength;
	Analyse::Instance().parseIpAndTcpData(packIpData, packiplength, packTcpData, packtcplength, pack.get());
    int isinsert = 0;
	for (int i = 0; i < mSortData.size(); i++)
	{
		std::shared_ptr<QByteArray> tmp = mSortData[i];
		IpData tmpIpData = { 0 };
		int iplength;
		TcpData tmpTcpData = { 0 };
		int tcplength;
		Analyse::Instance().parseIpAndTcpData(tmpIpData, iplength, tmpTcpData, tcplength, tmp.get());
		int needseq = tmpIpData.totalLength - tcplength - iplength + tmpTcpData.seqNum;
		if (packTcpData.seqNum == needseq)
		{
            if(i == mSortData.size() - 1)
            {
                mSortData.insert(i+1, pack);
              //  qDebug() << "xcn  addPack ack  sort " << acknum << "seq"  << packTcpData.seqNum << "index " << i+1;
                appendHttdData(pack, packiplength, packtcplength);
                isinsert = 1;
            }
            else
            {
                IpData nextIpData = { 0 };
                int nextiplength;
                TcpData nextTcpData = { 0 };
                int nexttcplength;
                Analyse::Instance().parseIpAndTcpData(nextIpData, nextiplength, nextTcpData, nexttcplength, mSortData[i+1].get());
                if(nextTcpData.seqNum == needseq)
                {
                    return -1;
                }
				else
				{
					mSortData.insert(i + 1, pack);
					//qDebug() << "xcn  addPack ack  sort " << acknum << "seq" << packTcpData.seqNum << "index " << i + 1;
					appendHttdData(pack, packiplength, packtcplength);
					isinsert = 1;
				}
            }
			break;
		}
	}
	return isinsert;
}
void CommunicationPack::appendHttdData(std::shared_ptr<QByteArray> &pack,int iplength, int tcplength)
{

	if(recHeadFinish)
	{
		QByteArray tmphttpdata = pack->mid(sizeof(LinkLayerData) + iplength + tcplength);
		mHttpData->append(tmphttpdata);
		//qDebug() << "xcn  addPack body data size " << tmphttpdata.size();
	}
	else
	{
		QByteArray tmphttpdata = pack->mid(sizeof(LinkLayerData) + iplength + tcplength);
		QByteArrayView y("\r\n\r\n");
		int indexend = tmphttpdata.indexOf(y);
		if(indexend>= 0)
		{
			recHeadFinish = true;
			if(indexend != (tmphttpdata.size() - 5))
			{
				QByteArray data = tmphttpdata.mid(indexend + 4);
				mHttpData->append(data);
				//qDebug() << "xcn  addPack header data size " << data.size();
			}
		}
	}
}
bool CommunicationPack::isRecFinish()
{
	if (!isSmoothly)
	{
		return false;
	}
	if (mHttpLength == mHttpData->size())
	{
		qDebug() << "xcn receive ok" << acknum;
		return true;
	}
	else
	{
		QByteArray byte("\r\n0\r\n");
		if (mHttpData->mid(mHttpData->size() - 5) == byte)
		{
			return true;
		}
	}
	return false;
}
bool CommunicationPack::isTimeOut()
{
	int sec = mCreateTime.secsTo(QDateTime::currentDateTime());
	return sec > 10;
}
AnalyseTask::AnalyseTask(/* args */)
{
}

AnalyseTask::~AnalyseTask()
{
}

HttpPicCutTask ::HttpPicCutTask (/* args */)
{
	connect(&mRecTotalTimer, &QTimer::timeout, this, &HttpPicCutTask::recTotalSlot);
	mRecTotalTimer.start(100);
}

HttpPicCutTask ::~HttpPicCutTask ()
{
}

void HttpPicCutTask::doTask(std::shared_ptr<QByteArray> &data)
{
    IpData tmpIpData = { 0 };
    int iplength;
    TcpData tmpTcpData = { 0 };
    int tcplength;
    Analyse::Instance().parseIpAndTcpData(tmpIpData, iplength, tmpTcpData, tcplength, data.get());
    //qDebug() << "receive ack " << tmpTcpData.ackNum << "protocol" << tmpIpData.protocol << "sourcePort " << tmpTcpData.sourcePort;
    if(tmpIpData.protocol != 6)
    {
        return;
    }
    if(tmpTcpData.sourcePort == 80 )
    {
        CommunicationPack* pack = nullptr;
		mCommunicationPackMutex.lock();
        if (mCommunicationPackHash.contains(tmpTcpData.ackNum))
        {
            pack = mCommunicationPackHash[tmpTcpData.ackNum];
        }
        else
        {
            pack = new CommunicationPack();
            pack->acknum = tmpTcpData.ackNum;
            mCommunicationPackHash.insert(tmpTcpData.ackNum, pack);
        }
		
        QByteArray httpdata = data->mid(sizeof(LinkLayerData) + iplength + tcplength,4);
		if(httpdata == QByteArray("HTTP"))
		{
			pack->addHeader(data);
		}
		else
		{
			pack->addPack(data);
		}
		mCommunicationPackMutex.unlock();
    }
}
void HttpPicCutTask::recTotalSlot()
{
	mCommunicationPackMutex.lock();
	QHash<int, CommunicationPack*>::iterator iter = mCommunicationPackHash.begin();
	QList<QByteArray> bytelist;

	while (iter != mCommunicationPackHash.end())
	{
		if (iter.value()->isRecFinish())
		{
			bytelist.append(iter.value()->getHttpData());
			delete iter.value();
			iter = mCommunicationPackHash.erase(iter);
		}
		else
		{
			bool isNeedDelete = false;
			if (iter.value() ->isFoundType())
			{
				if (!iter.value()->isImage())
				{
					isNeedDelete = true;
				}
			}
			if (iter.value()->isTimeOut())
			{
				isNeedDelete = true;
			}
			if (isNeedDelete)
			{
				delete iter.value();
				iter = mCommunicationPackHash.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
	mCommunicationPackMutex.unlock();

	for (size_t i = 0; i < bytelist.size(); i++)
	{
		QString filename = QApplication::applicationDirPath()+"/"+QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + ".jpg";
		//QString filename = QApplication::applicationDirPath() +"/123123.jpg";
		QFile file(filename);
		if (file.open(QIODevice::WriteOnly))
		{
			file.write(bytelist[i]);
		}
		else
		{
			QString asdf = file.errorString();
		}
		file.close();
	}
}