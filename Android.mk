LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_STATIC_LIBRARIES :=event2

LOCAL_MODULE:= neubot
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES := \
    echo.c \
    log.c \
    poller.c \
    strtonum.c \
    utils.c

include $(BUILD_STATIC_LIBRARY)