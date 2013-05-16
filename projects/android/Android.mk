LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

$(shell cp  $(LOCAL_PATH)/obj/local/armeabi/libmkswf.a  $(LOCAL_PATH)/libs/libmkswf.a)

LOCAL_MODULE := prebuild-mkswf
LOCAL_SRC_FILES := libs/libmkswf.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../

include $(PREBUILT_STATIC_LIBRARY)
