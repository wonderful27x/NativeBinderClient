//
// Created by wonderful on 21-1-25.
//
#include <stdlib.h>

#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>

#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include "wonderful_binder.h"

#define LOG_TAG "binder_demo"

using namespace android;

#define INFO(...) \
    do { \
        printf(__VA_ARGS__); \
        printf("\n"); \
        ALOGD(__VA_ARGS__); \
    } while(0)

void assert_fail(const char *file, int line, const char *func, const char *expr) {
    INFO("assertion failed at file %s, line %d, function %s:",
         file, line, func);
    INFO("%s", expr);
    abort();
}

#define ASSERT(e) \
    do { \
        if (!(e)) \
            assert_fail(__FILE__, __LINE__, __func__, #e); \
    } while(0)


// Where to print the parcel contents: aout, alog, aerr. alog doesn't seem to work.
#define PLOG aout

class IDemo : public IInterface{
public:
    enum {
        ALERT = IBinder::FIRST_INC_STRONG,
        PUSH,
        ADD
    };

    //declare our binder interface
    virtual void push(int32_t data) = 0;
    virtual void alert() = 0;
    virtual int32_t add(int32_t v1,int32_t  v2) = 0;

    //DECLARE_META_INTERFACE(Demo);
    static const String16 descriptor;
    static sp<IDemo> asInterface(const sp<IBinder>& obj);
    virtual const String16& getInterfaceDescriptor() const;
    IDemo();
    virtual ~IDemo();

};

//binder client
class BpDemo : public BpInterface<IDemo> {
public:
    BpDemo(const sp<IBinder>& impl) : BpInterface<IDemo> (impl){
        ALOGD("BpDemo::BpDemo()");
    }

    virtual void push(int32_t pushData){
        Parcel data,reply;
        data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
        data.writeInt32(pushData);

        aout << "BpDemo::push parcel to be sent:\n";
        data.print(PLOG);endl(PLOG);

        remote()->transact(PUSH,data,&reply);

        aout<< "BpDemo::push parcel reply:\n";
        reply.print(PLOG); endl(PLOG);

        ALOGD("BpDemo::push(%i)",pushData);
    }

    virtual void alert(){
        Parcel data,reply;
        data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
        data.writeString16(String16("The alert string"));
        remote()->transact(ALERT,data,&reply,IBinder::FLAG_ONEWAY);
        ALOGD("BpDemo::alert()");
    }

    virtual int32_t add(int32_t v1,int32_t v2){
        Parcel data,reply;
        data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
        data.writeInt32(v1);
        data.writeInt32(v2);

        aout<<"BpDemo::add parcel to be sent:\n";
        data.print(PLOG);endl(PLOG);

        remote()->transact(ADD,data,&reply);
        ALOGD("BpDemo::add transact reply");
        reply.print(PLOG);endl(PLOG);

        int32_t  res;
        status_t status = reply.readInt32(&res);
        ALOGD("BpDemo::add(%i,%i) = %i(status:%i)",v1,v2,res,status);
        return res;
    }
};

//IMPLEMENT_META_INTERFACE(Demo,"Demo");
const String16 IDemo::descriptor("Demo");
const String16& IDemo::getInterfaceDescriptor() const{
    return IDemo::descriptor;
}
sp<IDemo> IDemo::asInterface(const sp<IBinder>& obj){
    sp<IDemo> intr;
    if(obj == NULL){
        intr = static_cast<IDemo*> (obj->queryLocalInterface(IDemo::descriptor).get());
        if(intr == NULL){
            intr = new BpDemo(obj);
        }
    }
    return intr;
}
IDemo::IDemo() {ALOGD("IDemo::IDemo()");}
IDemo::~IDemo() {ALOGD("IDemo::~IDemo()");}

//binder service
class BnDemo : public BnInterface<IDemo>{
    virtual status_t onTransact(uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags);
};
status_t BnDemo::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
    ALOGD("BnDemo::onTransact(%i) %i",code,flags);
    data.checkInterface(this);
    data.print(PLOG);endl(PLOG);

    switch(code){
        case ALERT:{
            alert();
            return NO_ERROR;
        }break;
        case PUSH:{
            int32_t inData = data.readInt32();
            ALOGD("BnDemo::onTransact got %i",inData);
            push(inData);
            ASSERT(reply != 0);
            reply->print(PLOG);endl(PLOG);
            return NO_ERROR;
        }break;
        case ADD:{
            int32_t inV1 = data.readInt32();
            int32_t inV2 = data.readInt32();
            int32_t sum = add(inV1,inV2);
            ALOGD("BnDemo::onTransact add(%i,%i) = %i",inV1,inV2,sum);
            ASSERT(reply != 0);
            reply->print(PLOG);endl(PLOG);
            reply->writeInt32(sum);
            return NO_ERROR;
        }break;
        default:
            return BBinder::onTransact(code,data,reply,flags);
    }
}

//binder service implementation
class Demo : public BnDemo{
    virtual void alert(){
        INFO("Demo::alert()");
    }

    virtual void push(int32_t data){
        INFO("Demo::push(%i)",data);
    }

    virtual int32_t add(int32_t v1,int32_t v2){
        INFO("Demo::add(%i,%i)",v1,v2);
        return v1 + v2;
    }
};

//Helper function to get a hold of the Demo service
sp<IDemo> getDemoService(){
    sp<IServiceManager> sm = defaultServiceManager();
    ASSERT( sm != 0);
    sp<IBinder> binder = sm->getService(String16("Demo"));
    ASSERT(binder != 0);
    sp<IDemo> demo = interface_cast<IDemo>(binder);
    ASSERT(demo != 0);
    return demo;
}

//Helper function to add the Demo service
void addDemoService(){
    defaultServiceManager()->addService(String16("Demo"),new Demo());
    ProcessState::self()->startThreadPool();
    ALOGD("Demo service is now ready");
    IPCThreadState::self()->joinThreadPool();
    ALOGD("Demo service thread joined");
}

void executeAlert(){
    sp<IDemo> demo = getDemoService();
    demo->alert();
}

void executePush(int32_t data){
    sp<IDemo> demo = getDemoService();
    demo->push(data);
}

int32_t executeAdd(int32_t v1,int32_t v2){
    sp<IDemo> demo = getDemoService();
    return demo->add(v1,v2);
}
