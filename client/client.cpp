#include "api.h"
#include <iostream>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
#include <string>

#include "poc/include/binder_stub.h"
#include "poc/include/Chelpers.h"

using namespace android;
using namespace demo_api;

#define BINDER_THREAD_EXIT 0x40046208ul

/*
 * ---------------------------------------------------------------------------
 *
 *  Client Proxy
 */
namespace demo_api {

    BpDemoAPI::BpDemoAPI(const sp<IBinder>& impl ):BpInterface<IDemoAPI>(impl)
    {

    }

    char* BpDemoAPI::getName()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDemoAPI::getInterfaceDescriptor());
        //By operation code to transact
        remote()->transact(GET_NAME, data, &reply);
        //Exception Code. In Java Level, aidl auto generate codes will process exceptioncode.
        reply.readExceptionCode();
        return (char*)reply.readCString();
    }

    String16* BpDemoAPI::getFullName(String16* part)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDemoAPI::getInterfaceDescriptor());
        data.writeString16(*part);
        remote()->transact(GET_FULL_NAME, data, &reply);
        reply.readExceptionCode();
        String16* s = new String16(reply.readString16());
        return s;
    }

    int BpDemoAPI::sum(int a, int b)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDemoAPI::getInterfaceDescriptor());
        data.writeInt32(a);
        data.writeInt32(b);
        remote()->transact(SUM, data, &reply);
        reply.readExceptionCode();
        return (int)reply.readInt32();
    }

}//end of namespace demo_api
/*
 * End of Client Proxy
 */

void cout_utf16(unsigned short* buffer, size_t size)
{
    std::string buftmp = "";

    for(int i = 0; i < size; i++)
    {
        if (buffer[i] == 0)
        {
            break;
        }
        buftmp += (char)(buffer[i]);
    }
    std::cout << buftmp << std::endl;
}

#define SAFE_PAIL(x, ...) \
    if (x) \
    { \
        printf(__VA_ARGS__); \
    }

void list_available_services(sp<IServiceManager> sm)
{
    std::cout << "getting services available" << std::endl;

    Vector<String16> services = sm->listServices();

    std::cout << "service count is " << services.size() << std::endl;

    for (int i = 0; i < services.size(); i++)
    {
        std::cout << "service " << i << ": " << services[i] << " ";
        cout_utf16((unsigned short *)services[i].string(), services[i].size());
    }
}

int main(int argc, char *argv[])
{
    sp<IBinder> binder;
    sp<ProcessState> proc(ProcessState::self());
    //get service manager
    init_binderglobs();
    sp<IServiceManager> sm = defaultServiceManager();
    // sp<IServiceManager> sm = defaultservicemanager;

    int fd = 0;
    int epfd = 0;
    int result = -1;
    unsigned long argup = 0;
    void *ptr = 0;
    struct epoll_event event = {.events = EPOLLIN};
    std::vector<std::wstring> servlist;

    // list_available_services(sm);
    getfd(&fd);
    epfd = epoll_create(1000);

    ioctl(fd, 0xc0186201, 0xdeadbeef);
    listServices(0);
    ioctl(fd, 0xc0186201, 0xdeadbeef);

    // listServices(&servlist);
    list_available_services(sm);

    do{
        //Search service by SERVICE_NAME, instigates a write IOCTL
        binder = sm->getService(String16("SensorServer"));
        if(binder != 0)
            break;
        sleep(1);
    }while(true);

    // SAFE_PAIL(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1, "epoll_ctl failed\n");
    // SAFE_PAIL(ioctl(fd, BINDER_THREAD_EXIT, NULL) < 0, "thread exit failed\n");

    const sp<IDemoAPI>& bts = interface_cast<IDemoAPI>(binder);
    ALOGE("bindertest client is starting....."); 

    ALOGE("Service Name=%s",bts->getName());
    ALOGE("Service SUM %d+%d=%d",1,2,bts->sum(1,2));

fail:
    return result;
}

