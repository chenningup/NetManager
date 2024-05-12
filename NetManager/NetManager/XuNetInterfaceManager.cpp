
#include "XuNetInterfaceManager.h"
#include "XuNetInterfaceWindowManager.h"
XuNetInterfaceManager::XuNetInterfaceManager(/* args */)
{
    isStop = false;
}

XuNetInterfaceManager::~XuNetInterfaceManager()
{
    isStop = true;
}

XuNetInterfaceManager *XuNetInterfaceManager::Instance()
{
#ifdef WIN32
    static XuNetInterfaceWindowManager mXuNetInterfaceWindowManager;
    return &mXuNetInterfaceWindowManager;
#else
    return nullptr;
#endif
    
}

bool XuNetInterfaceManager::attch(void *object, dataCallBackfun callback)
{
    mCallBackMutex.lock();
    if (mCallBackHash.contains(object))
    {
        mCallBackMutex.unlock();
        return false;
    }
    mCallBackHash.insert(object,callback);
    mCallBackMutex.unlock();
    return true;
}

bool XuNetInterfaceManager::dettch(void *object, dataCallBackfun callback)
{
    mCallBackMutex.lock();
    if (!mCallBackHash.contains(object))
    {
        mCallBackMutex.unlock();
        return false;
    }
    mCallBackHash.remove(object);
    mCallBackMutex.unlock();
    return true;
}

bool XuNetInterfaceManager::callBackData(const unsigned char *data, int len)
{
    if (isStop)
    {
        return false;
    }
    mCallBackMutex.lock();
    QHash<void *,dataCallBackfun>::const_iterator i;
    QByteArray *tmp = new QByteArray((char *)data,len);
    std::shared_ptr<QByteArray>tmpptr(tmp);
    for (i = mCallBackHash.constBegin(); i != mCallBackHash.constEnd(); ++i)
    {
        i.value()(tmpptr);
    }
    mCallBackMutex.unlock();
    return true;
}
