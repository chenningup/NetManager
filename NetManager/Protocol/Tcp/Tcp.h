#ifndef TCP_H
#define TCP_H

#pragma pack(1)
struct LinkLayerData
{
    //链路层
    unsigned char disMac[6];
    unsigned char srcMac[6];
    unsigned short int type;
};
#pragma pack ()


#pragma pack(1)

struct IpData //Tcp的格式
{
    //IP层
    unsigned char versionAndHeaderLength;
    unsigned char serviceField[1];
    unsigned short int totalLength;
    unsigned short int identification;
    unsigned char flagAndOffset[2];
    unsigned char timeToLive;
    unsigned char protocol;
    unsigned char headerChecksum[2];
    unsigned char sourceIp[4];
    unsigned char disIp[4];
};
#pragma pack ()


#pragma pack(1)
struct TcpData
{
    unsigned short int sourcePort;
    unsigned short int distPort;
    unsigned int seqNum;
    unsigned int ackNum;
    unsigned char headerLengthAndFlag[2];
    unsigned short int window;
    unsigned short int checksum;
    unsigned short int urgentPtr;
};
#pragma pack ()

class Tcp
{
private:
    /* data */
public:
    Tcp(/* args */);
    ~Tcp();
};

Tcp::Tcp(/* args */)
{
}

Tcp::~Tcp()
{
}













#endif