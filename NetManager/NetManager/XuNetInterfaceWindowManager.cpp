#include "XuNetInterfaceWindowManager.h"
#ifdef WIN32
#include <Packet32.h>
#include <ntddndis.h>
#include <conio.h>
#include <Iphlpapi.h>
#include<QDebug>
#pragma comment(lib,"Iphlpapi.lib") //需要添加Iphlpapi.lib库
XuNetInterfaceWindowManager::XuNetInterfaceWindowManager():
    mCap(nullptr),
    errbuf(nullptr),
	isinit(false)
{    
	
}

XuNetInterfaceWindowManager::~XuNetInterfaceWindowManager()
{
}

bool XuNetInterfaceWindowManager::init()
{
	if(isinit)
	{
		return true;
	}
    getNetInterfaceInfo();
	isinit = true;
	start();
    return true;
}

bool XuNetInterfaceWindowManager::open(const QString &name)
{
    if(!mNetInterfaceInfoHash.contains(name))
    {
        return false;
    }
    if(!errbuf)
    {
        errbuf = new char(PCAP_ERRBUF_SIZE);
    }
	NetInterfaceInfo& tmp = mNetInterfaceInfoHash[name];
    if ((mCap = pcap_open(mNetInterfaceInfoHash[name].adapterName.toLocal8Bit().data(),
        65536,                // 要捕获的部分 (只捕获前100个字节)
        PCAP_OPENFLAG_PROMISCUOUS,  // 混杂模式
        1000,               // 读超时时间
        NULL,               // 远程机器验证
        errbuf              // 错误缓冲
        )) == NULL)
    {
        return false;
    }
    mCurNetInterface = mNetInterfaceInfoHash[name];
	return true;
}

bool XuNetInterfaceWindowManager::close()
{
    isStop = true;
    return true;
}

bool XuNetInterfaceWindowManager::send(unsigned char *data, int len)
{
    if(mCap)
    {
        return pcap_sendpacket(mCap, data,len) == len ;
    }
    return false;
}

QStringList XuNetInterfaceWindowManager::getNetInterfaceDesList()
{
    QStringList tmplist;
    QHash<QString, NetInterfaceInfo>::iterator iter = mNetInterfaceInfoHash.begin();
    while (iter!= mNetInterfaceInfoHash.end())
    {
        tmplist.push_back(iter.key());
        iter++;
    }
    return tmplist;
}

NetInterfaceInfo XuNetInterfaceWindowManager::getCurNetfaceInfo()
{
    return mCurNetInterface;
}

void XuNetInterfaceWindowManager::getNetInterfaceInfo()
{
	mNetInterfaceInfoHash.clear();

	/*****************获取网卡名、mac地址、ip地址、子网掩码、默认网关**********************/
	//PIP_ADAPTER_INFO结构体指针存储本机网卡信息
	PIP_ADAPTER_INFO adapter_info = new IP_ADAPTER_INFO();
	PIP_ADAPTER_INFO adapter = nullptr;
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	memset(adapter_info, 0, stSize);
	//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
	auto ret = GetAdaptersInfo(adapter_info, &stSize);
	if (ERROR_BUFFER_OVERFLOW == ret) {
		delete adapter_info;
		adapter_info = (PIP_ADAPTER_INFO)new BYTE[stSize];
		ret = GetAdaptersInfo(adapter_info, &stSize);
	}
	if (ERROR_SUCCESS == ret)
	{ 
		// 可能有多网卡,因此通过循环去判断
		adapter = adapter_info; // 保存链表头，等下需要释放空间
		while (adapter)
		{
			if (adapter->Type == MIB_IF_TYPE_LOOPBACK)
				continue;// 忽略环回地址
			NetInterfaceInfo info;
			char mac[128] = { 0 };
			for (DWORD i = 0; i < adapter->AddressLength; i++)
				sprintf(mac + strlen(mac)
					, (i < adapter->AddressLength - 1 ? "%02X:" : "%02X")
					, adapter->Address[i]);
			info.mac = QString::fromLocal8Bit(mac);
			info.adapterName = QString::fromLocal8Bit(adapter->AdapterName);
			info.ip = QString::fromLocal8Bit(adapter->IpAddressList.IpAddress.String);
			info.ipMask = QString::fromLocal8Bit(adapter->IpAddressList.IpMask.String);
			info.gateway = QString::fromLocal8Bit(adapter->GatewayList.IpAddress.String);
			info.description = QString::fromLocal8Bit(adapter->Description);
			mNetInterfaceInfoHash.insert(info.description, info);
			qDebug() << info.description << info.adapterName;
			adapter = adapter->Next;
		}
	}
	if (adapter_info) {
		delete adapter_info;
	}
	char errbuf[PCAP_ERRBUF_SIZE] = {0};
	pcap_if_t *alldevs;
	pcap_if_t *d;
	pcap_addr_t *a;
	quint32 net_ip;
	quint32 net_mask;
	//struct in_addr net_ip_address;
	//QString net_ip_string;
	pcap_findalldevs(&alldevs, errbuf);
	//char pacpsrc[9] = "rpcap://";
	//pcap_findalldevs_ex(pacpsrc, nullptr, &alldevs, errbuf);
	for (d = alldevs; d != NULL; d = d->next)
	{
		for (a = d->addresses; a; a = a->next)
		{
			if (a->addr->sa_family == AF_INET)
			{
				QString des = QString::fromLocal8Bit(d->description);
				qDebug() << "cap"<<des;
				QString fullname = QString::fromLocal8Bit(d->name);
				QString name = fullname.mid(fullname.indexOf('{'));
				if (mNetInterfaceInfoHash.contains(des))
				{
					mNetInterfaceInfoHash[des].adapterName = fullname;
				}
			}
		}
	}

}

void XuNetInterfaceWindowManager::run()
{
    while (!isStop)
    {
     	if (!mCap)
	    {
		    continue;;
	    }
        const u_char *pkt_data=nullptr;
        struct pcap_pkthdr *header=nullptr;
        int res = 0;
        res = pcap_next_ex(mCap, &header, &pkt_data);
        if (res == 0)
        {
            continue;
        }
        callBackData(pkt_data,header->caplen);
    }
}

#else

#endif
