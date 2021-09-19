LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils 	\
	libstlport

LOCAL_SRC_FILES := canitf.cpp	\
	ivSpec.cpp \
	log.cpp	\
	utils.cpp	\
	vehicle.cpp \
	clientCom.cpp \
	obdPort.cpp	\
	isoTp.cpp \
	portContext.cpp \
	portForward.cpp \
	portControl.cpp \
	onboard.cpp \
	udp.cpp

LOCAL_C_INCLUDES := \
	bionic \
	external/stlport/stlport \
	$(KERNEL_HEADERS)	\
	kernel/include


LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := canitf

LOCAL_CPPFLAGS:= -std=c++11 -pthread -shared -fpermissive -Wreorder

include $(BUILD_EXECUTABLE)

APP_ALLOW_MISSING_DEPS=true
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
