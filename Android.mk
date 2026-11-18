LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lua

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src $(LOCAL_PATH)/include $(LOCAL_PATH)/src/extensions/json $(LOCAL_PATH)/src/extensions/regex $(LOCAL_PATH)/src/extensions/util
LOCAL_CFLAGS    := -D"lua_getlocaledecpoint()='.'" -DLUA_USE_LINUX

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := src/lapi.c src/lauxlib.c src/lbaselib.c src/lcode.c src/lcorolib.c src/lctype.c src/ldblib.c src/ldebug.c src/ldo.c src/ldump.c src/lfunc.c src/lgc.c src/linit.c src/liolib.c src/llex.c src/lmathlib.c src/lmem.c src/loadlib.c src/lobject.c src/lopcodes.c src/loslib.c src/lparser.c src/lstate.c src/lstring.c src/lstrlib.c src/ltable.c src/ltablib.c src/ltm.c src/lundump.c src/lutf8lib.c src/lvm.c src/lzio.c src/extensions/json/cJSON.c src/extensions/json/ljsonlib.c src/extensions/regex/lregexlib.c src/extensions/util/aes.c src/extensions/util/b64_buffer.c src/extensions/util/b64_encode.c src/extensions/util/b64_decode.c src/extensions/util/lutilib.c src/lua_jni.c

LOCAL_STATIC_LIBRARIES :=

LOCAL_SHARED_LIBRARIES :=

LOCAL_LDLIBS := -lm -ldl -llog

include $(BUILD_SHARED_LIBRARY)
