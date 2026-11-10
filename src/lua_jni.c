#include <jni.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

static lua_State *to_lua_state(jlong ptr) {
    return (lua_State *)(uintptr_t)ptr;
}

JNIEXPORT jlong JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeNewState(JNIEnv *env, jclass clazz) {
    (void)env;
    (void)clazz;
    lua_State *L = luaL_newstate();
    if (L != NULL) {
        luaL_openlibs(L);
    }
    return (jlong)(uintptr_t)L;
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeClose(JNIEnv *env, jclass clazz, jlong ptr) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL) {
        lua_close(L);
    }
}

JNIEXPORT jstring JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeGetVersion(JNIEnv *env, jclass clazz) {
    (void)clazz;
    return (*env)->NewStringUTF(env, LUA_RELEASE);
}

JNIEXPORT jstring JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeDoString(JNIEnv *env, jclass clazz, jlong ptr, jstring jscript) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jscript == NULL) {
        return (*env)->NewStringUTF(env, "Invalid Lua state or null script");
    }

    const char *script = (*env)->GetStringUTFChars(env, jscript, NULL);
    int status = luaL_dostring(L, script);
    (*env)->ReleaseStringUTFChars(env, jscript, script);

    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        jstring jerr = (*env)->NewStringUTF(env, err ? err : "Unknown Lua error");
        lua_pop(L, 1);
        return jerr;
    }
    return NULL;
}

JNIEXPORT jstring JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeDoFile(JNIEnv *env, jclass clazz, jlong ptr, jstring jpath) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jpath == NULL) {
        return (*env)->NewStringUTF(env, "Invalid Lua state or null path");
    }

    const char *path = (*env)->GetStringUTFChars(env, jpath, NULL);
    int status = luaL_dofile(L, path);
    (*env)->ReleaseStringUTFChars(env, jpath, path);

    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        jstring jerr = (*env)->NewStringUTF(env, err ? err : "Unknown Lua file error");
        lua_pop(L, 1);
        return jerr;
    }
    return NULL;
}

JNIEXPORT jstring JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeEval(JNIEnv *env, jclass clazz, jlong ptr, jstring jexpr) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jexpr == NULL) {
        return NULL;
    }

    const char *expr = (*env)->GetStringUTFChars(env, jexpr, NULL);
    size_t len = strlen(expr) + 8;
    char *buf = (char *)malloc(len);
    if (buf == NULL) {
        (*env)->ReleaseStringUTFChars(env, jexpr, expr);
        return NULL;
    }
    snprintf(buf, len, "return %s", expr);

    int status = luaL_dostring(L, buf);
    free(buf);
    (*env)->ReleaseStringUTFChars(env, jexpr, expr);

    if (status != LUA_OK) {
        lua_pop(L, 1);
        return NULL;
    }

    const char *result = lua_tostring(L, -1);
    jstring jres = result ? (*env)->NewStringUTF(env, result) : NULL;
    lua_pop(L, 1);
    return jres;
}

// Global variable setters
JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeSetGlobalString(JNIEnv *env, jclass clazz, jlong ptr, jstring jname, jstring jval) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    if (jval != NULL) {
        const char *val = (*env)->GetStringUTFChars(env, jval, NULL);
        lua_pushstring(L, val);
        (*env)->ReleaseStringUTFChars(env, jval, val);
    } else {
        lua_pushnil(L);
    }
    lua_setglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeSetGlobalNumber(JNIEnv *env, jclass clazz, jlong ptr, jstring jname, jdouble val) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_pushnumber(L, (lua_Number)val);
    lua_setglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeSetGlobalInteger(JNIEnv *env, jclass clazz, jlong ptr, jstring jname, jlong val) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_pushinteger(L, (lua_Integer)val);
    lua_setglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeSetGlobalBoolean(JNIEnv *env, jclass clazz, jlong ptr, jstring jname, jboolean val) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_pushboolean(L, val ? 1 : 0);
    lua_setglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);
}

// Global variable getters
JNIEXPORT jstring JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeGetGlobalString(JNIEnv *env, jclass clazz, jlong ptr, jstring jname) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return NULL;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_getglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);

    const char *str = lua_tostring(L, -1);
    jstring res = str ? (*env)->NewStringUTF(env, str) : NULL;
    lua_pop(L, 1);
    return res;
}

JNIEXPORT jdouble JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeGetGlobalNumber(JNIEnv *env, jclass clazz, jlong ptr, jstring jname) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return 0.0;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_getglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);

    lua_Number num = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return (jdouble)num;
}

JNIEXPORT jlong JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeGetGlobalInteger(JNIEnv *env, jclass clazz, jlong ptr, jstring jname) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return 0;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_getglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);

    lua_Integer num = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return (jlong)num;
}

JNIEXPORT jboolean JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeGetGlobalBoolean(JNIEnv *env, jclass clazz, jlong ptr, jstring jname) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return JNI_FALSE;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_getglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);

    int b = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return b ? JNI_TRUE : JNI_FALSE;
}

// Stack operations
JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativePushNil(JNIEnv *env, jclass clazz, jlong ptr) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL) lua_pushnil(L);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativePushBoolean(JNIEnv *env, jclass clazz, jlong ptr, jboolean val) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL) lua_pushboolean(L, val ? 1 : 0);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativePushInteger(JNIEnv *env, jclass clazz, jlong ptr, jlong val) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL) lua_pushinteger(L, (lua_Integer)val);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativePushNumber(JNIEnv *env, jclass clazz, jlong ptr, jdouble val) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL) lua_pushnumber(L, (lua_Number)val);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativePushString(JNIEnv *env, jclass clazz, jlong ptr, jstring jstr) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL) return;
    if (jstr != NULL) {
        const char *str = (*env)->GetStringUTFChars(env, jstr, NULL);
        lua_pushstring(L, str);
        (*env)->ReleaseStringUTFChars(env, jstr, str);
    } else {
        lua_pushnil(L);
    }
}

JNIEXPORT jstring JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeToString(JNIEnv *env, jclass clazz, jlong ptr, jint index) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL) return NULL;
    const char *str = lua_tostring(L, index);
    return str ? (*env)->NewStringUTF(env, str) : NULL;
}

JNIEXPORT jdouble JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeToNumber(JNIEnv *env, jclass clazz, jlong ptr, jint index) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    return L ? (jdouble)lua_tonumber(L, index) : 0.0;
}

JNIEXPORT jlong JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeToInteger(JNIEnv *env, jclass clazz, jlong ptr, jint index) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    return L ? (jlong)lua_tointeger(L, index) : 0;
}

JNIEXPORT jboolean JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeToBoolean(JNIEnv *env, jclass clazz, jlong ptr, jint index) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    return (L && lua_toboolean(L, index)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeGetTop(JNIEnv *env, jclass clazz, jlong ptr) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    return L ? (jint)lua_gettop(L) : 0;
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativePop(JNIEnv *env, jclass clazz, jlong ptr, jint n) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL) lua_pop(L, n);
}

JNIEXPORT jstring JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativePCall(JNIEnv *env, jclass clazz, jlong ptr, jint nArgs, jint nResults) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL) {
        return (*env)->NewStringUTF(env, "Invalid Lua state");
    }
    int status = lua_pcall(L, nArgs, nResults, 0);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        jstring jerr = (*env)->NewStringUTF(env, err ? err : "Lua pcall execution error");
        lua_pop(L, 1);
        return jerr;
    }
    return NULL;
}
