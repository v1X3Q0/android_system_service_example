LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= client.cpp poc/binder_stub.cpp

LOCAL_SHARED_LIBRARIES := libutils libbinder liblog

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../include $(LOCAL_PATH)/poc/include
LOCAL_C_INCLUDES += $(BINDERDEMO_C_INCLUDES)

LOCAL_LDLIBS += $(BINDERDEMO_LDLIBS)

LOCAL_STATIC_LIBRARIES := binderTC_Stub

LOCAL_MODULE:= binderTClient
LOCAL_CFLAGS += -pie -fPIE -O0 -g
LOCAL_LDFLAGS += -pie -fPIE -O0 -g

include $(BUILD_EXECUTABLE)
