LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := mkswf

MY_PREFIX := $(LOCAL_PATH)/../../../src
MY_SOURCES := $(wildcard $(MY_PREFIX)/*.cpp)
MY_SOURCES += $(wildcard $(MY_PREFIX)/tags/*.cpp)

LOCAL_SRC_FILES := $(subst jni/, , $(MY_SOURCES))
#LOCAL_C_INCLUDES := $(MY_PREFIX) $(MY_PREFIX)/tags
LOCAL_C_INCLUDES := $(shell find $(MY_PREFIX) -type d |uniq)

include $(BUILD_STATIC_LIBRARY)
