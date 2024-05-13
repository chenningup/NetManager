#ifndef ARP_H
#define ARP_H
#include <QObject>
#include <memory>
#include<iostream>
#include <QByteArray>

struct ArpPack
{
	QByteArray disMac;
	QByteArray surMac;//6
	int type;
	QByteArray sendMac;//6
	QByteArray sendIp;//4
	QByteArray targetMac;//6
	QByteArray  targetIp;//4
};
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
