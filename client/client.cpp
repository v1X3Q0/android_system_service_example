#include "api.h"
#include <iostream>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

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

#define PROCESSSTATE_SELF       "_ZN7android12ProcessState4selfEv"
#define DEFAULTSERVICEMANAGER   "_ZN7android21defaultServiceManagerEv"

// interface variables
int mDriverFD = 0;
int mHandle = 0;

// loader variables
void* libbinder = 0;
void* (*processstate_self_f)(size_t*) = 0;
void* processstate_self = 0;
void* (*defaultservicemanager_f)(size_t*) = 0;
void* defaultservicemanager = 0;

#include <dlfcn.h>

int init_binderglobs()
{
    int result = -1;
    // these actually hold the value, but so does the deref
    size_t processstate_self_tmp = 0;
    size_t defaultServiceManager_tmp = 0;
    void* remote_tmp = 0;

    if (libbinder == 0)
    {
        libbinder = dlopen("libbinder.so", RTLD_NOW);

        processstate_self_f = (void* (*)(size_t*))dlsym(libbinder, PROCESSSTATE_SELF);
        SAFE_PAIL(processstate_self_f == 0, "couldn't find " PROCESSSTATE_SELF);
        processstate_self = processstate_self_f(&processstate_self_tmp);
        processstate_self = *(void**)processstate_self;

        defaultservicemanager_f = (void* (*)(size_t*))dlsym(libbinder, DEFAULTSERVICEMANAGER);
        SAFE_PAIL(defaultservicemanager_f == 0, "couldn't find " DEFAULTSERVICEMANAGER);
        defaultservicemanager = defaultservicemanager_f(&defaultServiceManager_tmp);
        defaultservicemanager = *(void**)defaultservicemanager;

        mDriverFD = *(int*)(((size_t)processstate_self) + 4);
        remote_tmp = *(void**)(((size_t)defaultservicemanager) + 8);
        mHandle = *(int*)(((size_t)remote_tmp) + 4);
    }

    result = 0;
fail:
    return result;
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

    // list_available_services(sm);
    fd = *(int*)(*((void**)&proc) + 4);
    epfd = epoll_create(1000);

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

