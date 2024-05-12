#include "arp.h"

Arp::Arp()
{
}

Arp::~Arp()
{
}

Arp & Arp::Instance()
{
	static Arp mArp;
	return mArp;
}

std::shared_ptr<QByteArray> Arp::createPack(const ArpPack &pack)
{
    QByteArray *byte = new QByteArray();
    std::shared_ptr<QByteArray>tmpptr(byte);
	char protoalType[2] = { 0x08, 0x06 };
	char tmp[6] = { 0x00, 0x01, 0x08, 0x00, 0x06, 0x04 };
	char opcode[2] = { 0 };
	opcode[1] = pack.type;
    byte->append((char*)pack.disMac,6);
    byte->append((char*)pack.surMac,6);
	byte->append(protoalType,2);
	byte->append(tmp,6);

    byte->append(opcode,2);
	byte->append((char*)pack.sendMac, 6);
	byte->append((char*)pack.sendIp,4);
	byte->append((char*)pack.targetMac, 6);
	byte->append((char*)pack.targetIp,4);
    return tmpptr;
}
