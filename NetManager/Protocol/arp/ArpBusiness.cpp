#include "ArpBusiness.h"
#include "Arp.h"
#include <QDebug>
#include"../NetTransmit/NetTransmit.h"

void createcharip(const QString &ipstr, char *str)
{

	if (ipstr != "")
	{
		sscanf(ipstr.toLocal8Bit().data(), "%d.%d.%d.%d", &str[0], &str[1], &str[2], &str[3]);
	}

}

void createcharmac(const QString & macip, char *str)
{
	if (macip != "")
	{
		sscanf(macip.toLocal8Bit().data(), "%x:%x:%x:%x:%x:%x", &str[0], &str[1], &str[2], &str[3], &str[4], &str[5]);
	}
}

QString getThreeIp(const QString &ip)
{
	QStringList iplist = ip.split('.');
	if (iplist.count() == 4)
	{
		return iplist.at(0) + "." + iplist.at(1) + "." + iplist.at(2) + ".";
	}
	return "";
}
ArpBusiness::ArpBusiness():isStop(false)
{

}

ArpBusiness::~ArpBusiness()
{
	isStop = true;
}

ArpBusiness &ArpBusiness::Instance()
{
	static ArpBusiness mArpBusiness;
	return mArpBusiness;
    // TODO: 在此处插入 return 语句
}

void ArpBusiness::init()
{
	mNetmanager = XuNetInterfaceManager::Instance();
	if(mNetmanager)
	{
		mNetmanager->init();
		mNetmanager->attch(this,std::bind(&ArpBusiness::receiveData,this,std::placeholders::_1));
	}
}

void ArpBusiness::run()
{
	while (!isStop)
	{
		if(!mOnitorList.isEmpty())
		{
			
		}
	}
}

 void ArpBusiness::addDeceive(const QString& ip)
 {
 	if (!mDeceiveList.contains(ip))
 	{
 		mDeceiveList.push_back(ip);
 	}
 }

 void ArpBusiness::deleteDeceive(const QString& ip)
 {
 	if (mDeceiveList.contains(ip))
 	{
 		mDeceiveList.removeOne(ip);
 	}
 }

void ArpBusiness::addDisconnectNet(const QString& ip)
{
    if(!mDiconnectNetList.contains(ip))
    {
        mDiconnectNetList.push_back(ip);
    }
}

void ArpBusiness::deleteDisconnectNet(const QString& ip)
{
    if (mDiconnectNetList.contains(ip))
    {
        mDiconnectNetList.removeOne(ip);
    }
}

void ArpBusiness::addOnitor(const QString& ip)
{
	if (!mOnitorList.contains(ip))
	{
		mOnitorList.push_back(ip);
	}
}

void ArpBusiness::deleteOnitor(const QString& ip)
{
	if (mOnitorList.contains(ip))
	{
		mOnitorList.removeOne(ip);
	}
}

void ArpBusiness::broadcast()
{
	NetInterfaceInfo tmpinfo = XuNetInterfaceManager::Instance()->getCurNetfaceInfo();
	ArpPack tmpPck = {0};
	memset(tmpPck.disMac,0XFF,6);
	memset(tmpPck.targetMac,0,6);
	
	createcharmac(tmpinfo.mac, (char*)tmpPck.surMac);
	createcharmac(tmpinfo.mac, (char*)tmpPck.sendMac);
	createcharip(tmpinfo.ip, (char*)tmpPck.sendIp);
	tmpPck.type = 1;
	for (int i = 1; i <= 255; i++)
	{
		QString tmptargetIp = getThreeIp(tmpinfo.ip) + QString::number(i);
		createcharip(tmptargetIp, (char*)tmpPck.targetIp);
		std::shared_ptr<QByteArray> arpdata = Arp::Instance().createPack(tmpPck);
		mNetmanager->send((unsigned char *)arpdata->data(),arpdata->size());
		QThread::msleep(2);
	}
}

bool ArpBusiness::receiveData(std::shared_ptr<QByteArray> data)
{
	if (data->size() == 42)
	{
		if ((data->at(12) == (char)0X08) &&
			(data->at(13) == (char)0X06) &&
			(data->at(21) == (char)0X02)
			)
		{
			QByteArray macArray = data->mid(22, 6);
			QByteArray ipArray = data->mid(28, 4);
			QString ip = QString("%1.%2.%3.%4").arg(QString::number((unsigned char)ipArray[0])).arg(QString::number((unsigned char)ipArray[1])).arg(QString::number((unsigned char)ipArray[2])).arg(QString::number((unsigned char)ipArray[3]));
			QString mac = QString("%1:%2:%3:%4:%5:%6").arg(QString::number((unsigned char)macArray[0],16)).arg(QString::number((unsigned char)macArray[1],16)).arg(QString::number((unsigned char)macArray[2],16)).arg(QString::number((unsigned char)macArray[3],16)).arg(QString::number((unsigned char)macArray[4],16)).arg(QString::number((unsigned char)macArray[5],16));
			if(!mDeviceInfo.contains(ip))
			{
				mDeviceInfo.insert(ip,mac);
			}
			emit findDevice(ip,mac);
		}
	}
    return true;
}
