LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := crc32.c \
	device.c \
	item.c \
	policy_interface.c

LOCAL_MODULE := libpolicy
LOCAL_SHARED_LIBRARIES := libcutils libc
LOCAL_LDLIBS :=  -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE := false 
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

