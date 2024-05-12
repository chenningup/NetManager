#ifndef ARP_H
#define ARP_H
#include <QObject>
#include <memory>
#include<iostream>
#include <QByteArray>

#pragma pack(1)
struct ArpPack
{
	unsigned char disMac[10];//6
	unsigned char surMac[10];//6
	int type;
	unsigned char sendMac[10];//6
	unsigned char sendIp[10];//4
	unsigned char targetMac[10];//6
	unsigned char targetIp[10];//4
};
#pragma pack()
class Arp : public QObject
{
    Q_OBJECT
public:
    Arp(/* args */);
    ~Arp();

	static Arp &Instance();

    std::shared_ptr<QByteArray> createPack(const ArpPack &pack);
private:
    /* data */
};



#endif
