#ifndef TCP_H
#define TCP_H

#pragma pack(1)
struct LinkLayerData
{
    //链路层
    unsigned char disMac[6];
    unsigned char srcMac[6];
    short int type;
};
#pragma pack ()


#pragma pack(1)

struct IpData //Tcp的格式
{
    //IP层
    unsigned char versionAndHeaderLength;
    unsigned char serviceField[1];
    short int totalLength;
    short int identification;
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
    short int sourcePort;
    short int distPort;
    int seqNum;
    int ackNum;
    unsigned char headerLengthAndFlag[2];
    short int window;
    short int checksum;
    short int urgentPtr;
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