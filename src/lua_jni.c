#include <jni.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

// Cached global references for JNI performance
static jclass g_class_LuaValue = NULL;
static jclass g_class_LuaNil = NULL;
static jclass g_class_LuaBoolean = NULL;
static jclass g_class_LuaInteger = NULL;
static jclass g_class_LuaDouble = NULL;
static jclass g_class_LuaString = NULL;
static jclass g_class_LuaTable = NULL;
static jclass g_class_NativeLuaTable = NULL;
static jclass g_class_NativeLuaFunction = NULL;
static jclass g_class_LuaError = NULL;

static jfieldID g_fid_NIL = NULL;
static jfieldID g_fid_TRUE = NULL;
static jfieldID g_fid_FALSE = NULL;

static jmethodID g_mid_valueOf_long = NULL;
static jmethodID g_mid_valueOf_double = NULL;
static jmethodID g_mid_valueOf_string = NULL;

static jmethodID g_mid_LuaBoolean_booleanValue = NULL;
static jmethodID g_mid_LuaInteger_longValue = NULL;
static jmethodID g_mid_LuaDouble_doubleValue = NULL;
static jmethodID g_mid_LuaValue_tojstring = NULL;

static jmethodID g_mid_NativeLuaTable_init = NULL;
static jmethodID g_mid_NativeLuaFunction_init = NULL;

static jfieldID g_fid_NativeLuaTable_ref = NULL;
static jfieldID g_fid_NativeLuaFunction_ref = NULL;

static lua_State *to_lua_state(jlong ptr) {
    return (lua_State *)(uintptr_t)ptr;
}

static void throw_lua_error(JNIEnv *env, const char *msg) {
    if (g_class_LuaError != NULL) {
        (*env)->ThrowNew(env, g_class_LuaError, msg ? msg : "Lua error");
    }
}

// Forward declarations
static jobject lua_value_from_stack(JNIEnv *env, lua_State *L, int index, jlong statePtr);
static void push_lua_value(JNIEnv *env, lua_State *L, jobject jval);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;
    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass local_LuaValue = (*env)->FindClass(env, "com/github/anaruto/lua/LuaValue");
    if (local_LuaValue == NULL) return JNI_ERR;
    g_class_LuaValue = (jclass)(*env)->NewGlobalRef(env, local_LuaValue);
    (*env)->DeleteLocalRef(env, local_LuaValue);

    jclass local_LuaNil = (*env)->FindClass(env, "com/github/anaruto/lua/LuaNil");
    if (local_LuaNil != NULL) {
        g_class_LuaNil = (jclass)(*env)->NewGlobalRef(env, local_LuaNil);
        (*env)->DeleteLocalRef(env, local_LuaNil);
    }

    jclass local_LuaBoolean = (*env)->FindClass(env, "com/github/anaruto/lua/LuaBoolean");
    if (local_LuaBoolean != NULL) {
        g_class_LuaBoolean = (jclass)(*env)->NewGlobalRef(env, local_LuaBoolean);
        (*env)->DeleteLocalRef(env, local_LuaBoolean);
    }

    jclass local_LuaInteger = (*env)->FindClass(env, "com/github/anaruto/lua/LuaInteger");
    if (local_LuaInteger != NULL) {
        g_class_LuaInteger = (jclass)(*env)->NewGlobalRef(env, local_LuaInteger);
        (*env)->DeleteLocalRef(env, local_LuaInteger);
    }

    jclass local_LuaDouble = (*env)->FindClass(env, "com/github/anaruto/lua/LuaDouble");
    if (local_LuaDouble != NULL) {
        g_class_LuaDouble = (jclass)(*env)->NewGlobalRef(env, local_LuaDouble);
        (*env)->DeleteLocalRef(env, local_LuaDouble);
    }

    jclass local_LuaString = (*env)->FindClass(env, "com/github/anaruto/lua/LuaString");
    if (local_LuaString != NULL) {
        g_class_LuaString = (jclass)(*env)->NewGlobalRef(env, local_LuaString);
        (*env)->DeleteLocalRef(env, local_LuaString);
    }

    jclass local_LuaTable = (*env)->FindClass(env, "com/github/anaruto/lua/LuaTable");
    if (local_LuaTable != NULL) {
        g_class_LuaTable = (jclass)(*env)->NewGlobalRef(env, local_LuaTable);
        (*env)->DeleteLocalRef(env, local_LuaTable);
    }

    jclass local_NativeLuaTable = (*env)->FindClass(env, "com/github/anaruto/lua/NativeLuaTable");
    if (local_NativeLuaTable != NULL) {
        g_class_NativeLuaTable = (jclass)(*env)->NewGlobalRef(env, local_NativeLuaTable);
        (*env)->DeleteLocalRef(env, local_NativeLuaTable);
    }

    jclass local_NativeLuaFunction = (*env)->FindClass(env, "com/github/anaruto/lua/NativeLuaFunction");
    if (local_NativeLuaFunction != NULL) {
        g_class_NativeLuaFunction = (jclass)(*env)->NewGlobalRef(env, local_NativeLuaFunction);
        (*env)->DeleteLocalRef(env, local_NativeLuaFunction);
    }

    jclass local_LuaError = (*env)->FindClass(env, "com/github/anaruto/lua/LuaError");
    if (local_LuaError != NULL) {
        g_class_LuaError = (jclass)(*env)->NewGlobalRef(env, local_LuaError);
        (*env)->DeleteLocalRef(env, local_LuaError);
    }

    // Cache field IDs
    g_fid_NIL = (*env)->GetStaticFieldID(env, g_class_LuaValue, "NIL", "Lcom/github/anaruto/lua/LuaNil;");
    g_fid_TRUE = (*env)->GetStaticFieldID(env, g_class_LuaValue, "TRUE", "Lcom/github/anaruto/lua/LuaBoolean;");
    g_fid_FALSE = (*env)->GetStaticFieldID(env, g_class_LuaValue, "FALSE", "Lcom/github/anaruto/lua/LuaBoolean;");

    // Cache method IDs
    if (g_class_LuaInteger != NULL) {
        g_mid_valueOf_long = (*env)->GetStaticMethodID(env, g_class_LuaInteger, "valueOf", "(J)Lcom/github/anaruto/lua/LuaInteger;");
        g_mid_LuaInteger_longValue = (*env)->GetMethodID(env, g_class_LuaInteger, "longValue", "()J");
    }
    if (g_class_LuaDouble != NULL) {
        g_mid_valueOf_double = (*env)->GetStaticMethodID(env, g_class_LuaDouble, "valueOf", "(D)Lcom/github/anaruto/lua/LuaDouble;");
        g_mid_LuaDouble_doubleValue = (*env)->GetMethodID(env, g_class_LuaDouble, "doubleValue", "()D");
    }
    if (g_class_LuaString != NULL) {
        g_mid_valueOf_string = (*env)->GetStaticMethodID(env, g_class_LuaString, "valueOf", "(Ljava/lang/String;)Lcom/github/anaruto/lua/LuaString;");
    }
    if (g_class_LuaBoolean != NULL) {
        g_mid_LuaBoolean_booleanValue = (*env)->GetMethodID(env, g_class_LuaBoolean, "booleanValue", "()Z");
    }
    if (g_class_LuaValue != NULL) {
        g_mid_LuaValue_tojstring = (*env)->GetMethodID(env, g_class_LuaValue, "tojstring", "()Ljava/lang/String;");
    }

    if (g_class_NativeLuaTable != NULL) {
        g_mid_NativeLuaTable_init = (*env)->GetMethodID(env, g_class_NativeLuaTable, "<init>", "(JI)V");
        g_fid_NativeLuaTable_ref = (*env)->GetFieldID(env, g_class_NativeLuaTable, "ref", "I");
    }

    if (g_class_NativeLuaFunction != NULL) {
        g_mid_NativeLuaFunction_init = (*env)->GetMethodID(env, g_class_NativeLuaFunction, "<init>", "(JI)V");
        g_fid_NativeLuaFunction_ref = (*env)->GetFieldID(env, g_class_NativeLuaFunction, "ref", "I");
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    (void)reserved;
    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return;
    }

    if (g_class_LuaValue) (*env)->DeleteGlobalRef(env, g_class_LuaValue);
    if (g_class_LuaNil) (*env)->DeleteGlobalRef(env, g_class_LuaNil);
    if (g_class_LuaBoolean) (*env)->DeleteGlobalRef(env, g_class_LuaBoolean);
    if (g_class_LuaInteger) (*env)->DeleteGlobalRef(env, g_class_LuaInteger);
    if (g_class_LuaDouble) (*env)->DeleteGlobalRef(env, g_class_LuaDouble);
    if (g_class_LuaString) (*env)->DeleteGlobalRef(env, g_class_LuaString);
    if (g_class_LuaTable) (*env)->DeleteGlobalRef(env, g_class_LuaTable);
    if (g_class_NativeLuaTable) (*env)->DeleteGlobalRef(env, g_class_NativeLuaTable);
    if (g_class_NativeLuaFunction) (*env)->DeleteGlobalRef(env, g_class_NativeLuaFunction);
    if (g_class_LuaError) (*env)->DeleteGlobalRef(env, g_class_LuaError);
}

// Convert a Lua stack value to a polymorphic jobject (LuaValue)
static jobject lua_value_from_stack(JNIEnv *env, lua_State *L, int index, jlong statePtr) {
    int type = lua_type(L, index);
    switch (type) {
        case LUA_TNIL:
        case LUA_TNONE:
            return (*env)->GetStaticObjectField(env, g_class_LuaValue, g_fid_NIL);

        case LUA_TBOOLEAN:
            return lua_toboolean(L, index)
                ? (*env)->GetStaticObjectField(env, g_class_LuaValue, g_fid_TRUE)
                : (*env)->GetStaticObjectField(env, g_class_LuaValue, g_fid_FALSE);

        case LUA_TNUMBER:
            if (lua_isinteger(L, index)) {
                jlong val = (jlong)lua_tointeger(L, index);
                return (*env)->CallStaticObjectMethod(env, g_class_LuaInteger, g_mid_valueOf_long, val);
            } else {
                jdouble val = (jdouble)lua_tonumber(L, index);
                return (*env)->CallStaticObjectMethod(env, g_class_LuaDouble, g_mid_valueOf_double, val);
            }

        case LUA_TSTRING: {
            size_t len = 0;
            const char *str = lua_tolstring(L, index, &len);
            jstring js = (*env)->NewStringUTF(env, str ? str : "");
            jobject obj = (*env)->CallStaticObjectMethod(env, g_class_LuaString, g_mid_valueOf_string, js);
            (*env)->DeleteLocalRef(env, js);
            return obj;
        }

        case LUA_TTABLE: {
            lua_pushvalue(L, index);
            int ref = luaL_ref(L, LUA_REGISTRYINDEX);
            return (*env)->NewObject(env, g_class_NativeLuaTable, g_mid_NativeLuaTable_init, statePtr, (jint)ref);
        }

        case LUA_TFUNCTION: {
            lua_pushvalue(L, index);
            int ref = luaL_ref(L, LUA_REGISTRYINDEX);
            return (*env)->NewObject(env, g_class_NativeLuaFunction, g_mid_NativeLuaFunction_init, statePtr, (jint)ref);
        }

        default: {
            // Userdata, Thread, etc. - fall back to string representation
            const char *s = luaL_tolstring(L, index, NULL);
            jstring js = (*env)->NewStringUTF(env, s ? s : "");
            lua_pop(L, 1);
            jobject obj = (*env)->CallStaticObjectMethod(env, g_class_LuaString, g_mid_valueOf_string, js);
            (*env)->DeleteLocalRef(env, js);
            return obj;
        }
    }
}

// Push a polymorphic Kotlin LuaValue onto the Lua stack
static void push_lua_value(JNIEnv *env, lua_State *L, jobject jval) {
    if (jval == NULL) {
        lua_pushnil(L);
        return;
    }

    if ((*env)->IsInstanceOf(env, jval, g_class_LuaNil)) {
        lua_pushnil(L);
    } else if ((*env)->IsInstanceOf(env, jval, g_class_LuaBoolean)) {
        jboolean b = (*env)->CallBooleanMethod(env, jval, g_mid_LuaBoolean_booleanValue);
        lua_pushboolean(L, b ? 1 : 0);
    } else if ((*env)->IsInstanceOf(env, jval, g_class_LuaInteger)) {
        jlong l = (*env)->CallLongMethod(env, jval, g_mid_LuaInteger_longValue);
        lua_pushinteger(L, (lua_Integer)l);
    } else if ((*env)->IsInstanceOf(env, jval, g_class_LuaDouble)) {
        jdouble d = (*env)->CallDoubleMethod(env, jval, g_mid_LuaDouble_doubleValue);
        lua_pushnumber(L, (lua_Number)d);
    } else if ((*env)->IsInstanceOf(env, jval, g_class_LuaString)) {
        jstring js = (jstring)(*env)->CallObjectMethod(env, jval, g_mid_LuaValue_tojstring);
        if (js != NULL) {
            const char *s = (*env)->GetStringUTFChars(env, js, NULL);
            lua_pushstring(L, s ? s : "");
            (*env)->ReleaseStringUTFChars(env, js, s);
            (*env)->DeleteLocalRef(env, js);
        } else {
            lua_pushstring(L, "");
        }
    } else if ((*env)->IsInstanceOf(env, jval, g_class_NativeLuaTable)) {
        jint ref = (*env)->GetIntField(env, jval, g_fid_NativeLuaTable_ref);
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    } else if ((*env)->IsInstanceOf(env, jval, g_class_NativeLuaFunction)) {
        jint ref = (*env)->GetIntField(env, jval, g_fid_NativeLuaFunction_ref);
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    } else {
        // Fallback: convert to string via tojstring()
        jstring js = (jstring)(*env)->CallObjectMethod(env, jval, g_mid_LuaValue_tojstring);
        if (js != NULL) {
            const char *s = (*env)->GetStringUTFChars(env, js, NULL);
            lua_pushstring(L, s ? s : "");
            (*env)->ReleaseStringUTFChars(env, js, s);
            (*env)->DeleteLocalRef(env, js);
        } else {
            lua_pushnil(L);
        }
    }
}

// =========================================================================
// Globals native bindings (LuaJ architecture)
// =========================================================================

JNIEXPORT jlong JNICALL
Java_com_github_anaruto_lua_Globals_nativeNewState(JNIEnv *env, jclass clazz) {
    (void)env;
    (void)clazz;
    lua_State *L = luaL_newstate();
    if (L != NULL) {
        luaL_openlibs(L);
    }
    return (jlong)(uintptr_t)L;
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_Globals_nativeClose(JNIEnv *env, jclass clazz, jlong ptr) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL) {
        lua_close(L);
    }
}

JNIEXPORT jint JNICALL
Java_com_github_anaruto_lua_Globals_nativeLoad(JNIEnv *env, jclass clazz, jlong ptr, jstring jscript, jstring jname) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jscript == NULL) {
        throw_lua_error(env, "Invalid Lua state or null script");
        return -1;
    }

    const char *script = (*env)->GetStringUTFChars(env, jscript, NULL);
    const char *name = jname ? (*env)->GetStringUTFChars(env, jname, NULL) : "chunk";
    int status = luaL_loadbuffer(L, script, strlen(script), name);
    (*env)->ReleaseStringUTFChars(env, jscript, script);
    if (jname) (*env)->ReleaseStringUTFChars(env, jname, name);

    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        throw_lua_error(env, err ? err : "Compilation error");
        lua_pop(L, 1);
        return -1;
    }

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return (jint)ref;
}

JNIEXPORT jint JNICALL
Java_com_github_anaruto_lua_Globals_nativeLoadFile(JNIEnv *env, jclass clazz, jlong ptr, jstring jpath) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jpath == NULL) {
        throw_lua_error(env, "Invalid Lua state or null file path");
        return -1;
    }

    const char *path = (*env)->GetStringUTFChars(env, jpath, NULL);
    int status = luaL_loadfile(L, path);
    (*env)->ReleaseStringUTFChars(env, jpath, path);

    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        throw_lua_error(env, err ? err : "Failed to load script file");
        lua_pop(L, 1);
        return -1;
    }

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return (jint)ref;
}

JNIEXPORT jobjectArray JNICALL
Java_com_github_anaruto_lua_Globals_nativeInvoke(JNIEnv *env, jclass clazz, jlong ptr, jint funcRef, jobjectArray jargs) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || funcRef == -1) {
        throw_lua_error(env, "Invalid Lua state or invalid function reference");
        return NULL;
    }

    int base = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        throw_lua_error(env, "Referenced object is not a function");
        return NULL;
    }

    jsize nargs = jargs ? (*env)->GetArrayLength(env, jargs) : 0;
    for (jsize i = 0; i < nargs; i++) {
        jobject arg = (*env)->GetObjectArrayElement(env, jargs, i);
        push_lua_value(env, L, arg);
        (*env)->DeleteLocalRef(env, arg);
    }

    int status = lua_pcall(L, (int)nargs, LUA_MULTRET, 0);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        throw_lua_error(env, err ? err : "Lua runtime execution error");
        lua_pop(L, 1);
        return NULL;
    }

    int nresults = lua_gettop(L) - base;
    jobjectArray resArray = (*env)->NewObjectArray(env, nresults, g_class_LuaValue, NULL);
    for (int i = 0; i < nresults; i++) {
        jobject val = lua_value_from_stack(env, L, base + 1 + i, ptr);
        (*env)->SetObjectArrayElement(env, resArray, i, val);
        (*env)->DeleteLocalRef(env, val);
    }
    lua_pop(L, nresults);
    return resArray;
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_Globals_nativeUnref(JNIEnv *env, jclass clazz, jlong ptr, jint ref) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L != NULL && ref != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
}

JNIEXPORT jobject JNICALL
Java_com_github_anaruto_lua_Globals_nativeGetGlobal(JNIEnv *env, jclass clazz, jlong ptr, jstring jname) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) {
        return (*env)->GetStaticObjectField(env, g_class_LuaValue, g_fid_NIL);
    }

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    lua_getglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);

    jobject val = lua_value_from_stack(env, L, -1, ptr);
    lua_pop(L, 1);
    return val;
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_Globals_nativeSetGlobal(JNIEnv *env, jclass clazz, jlong ptr, jstring jname, jobject jval) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jname == NULL) return;

    const char *name = (*env)->GetStringUTFChars(env, jname, NULL);
    push_lua_value(env, L, jval);
    lua_setglobal(L, name);
    (*env)->ReleaseStringUTFChars(env, jname, name);
}

JNIEXPORT jobject JNICALL
Java_com_github_anaruto_lua_Globals_nativeTableGet(JNIEnv *env, jclass clazz, jlong ptr, jint tableRef, jobject jkey) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || tableRef == -1) {
        return (*env)->GetStaticObjectField(env, g_class_LuaValue, g_fid_NIL);
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, tableRef);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return (*env)->GetStaticObjectField(env, g_class_LuaValue, g_fid_NIL);
    }

    push_lua_value(env, L, jkey);
    lua_gettable(L, -2);
    jobject val = lua_value_from_stack(env, L, -1, ptr);
    lua_pop(L, 2); // pop result and table
    return val;
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_Globals_nativeTableSet(JNIEnv *env, jclass clazz, jlong ptr, jint tableRef, jobject jkey, jobject jval) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || tableRef == -1) return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, tableRef);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    push_lua_value(env, L, jkey);
    push_lua_value(env, L, jval);
    lua_settable(L, -3);
    lua_pop(L, 1); // pop table
}

JNIEXPORT jint JNICALL
Java_com_github_anaruto_lua_Globals_nativeTableLength(JNIEnv *env, jclass clazz, jlong ptr, jint tableRef) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || tableRef == -1) return 0;

    lua_rawgeti(L, LUA_REGISTRYINDEX, tableRef);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }

    lua_len(L, -1);
    lua_Integer len = lua_tointeger(L, -1);
    lua_pop(L, 2);
    return (jint)len;
}

JNIEXPORT jobjectArray JNICALL
Java_com_github_anaruto_lua_Globals_nativeTableKeys(JNIEnv *env, jclass clazz, jlong ptr, jint tableRef) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || tableRef == -1) {
        return (*env)->NewObjectArray(env, 0, g_class_LuaValue, NULL);
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, tableRef);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return (*env)->NewObjectArray(env, 0, g_class_LuaValue, NULL);
    }

    // Count keys first
    int count = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        count++;
        lua_pop(L, 1); // pop value, keep key
    }

    jobjectArray keys = (*env)->NewObjectArray(env, count, g_class_LuaValue, NULL);
    int idx = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        jobject k = lua_value_from_stack(env, L, -2, ptr);
        (*env)->SetObjectArrayElement(env, keys, idx++, k);
        (*env)->DeleteLocalRef(env, k);
        lua_pop(L, 1); // pop value, keep key
    }
    lua_pop(L, 1); // pop table
    return keys;
}

JNIEXPORT jint JNICALL
Java_com_github_anaruto_lua_Globals_nativeNewTable(JNIEnv *env, jclass clazz, jlong ptr) {
    (void)env;
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL) return -1;
    lua_newtable(L);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return (jint)ref;
}

JNIEXPORT jobjectArray JNICALL
Java_com_github_anaruto_lua_Globals_nativeDoString(JNIEnv *env, jclass clazz, jlong ptr, jstring jscript) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jscript == NULL) {
        throw_lua_error(env, "Invalid Lua state or null script");
        return NULL;
    }

    const char *script = (*env)->GetStringUTFChars(env, jscript, NULL);
    int base = lua_gettop(L);
    int status = luaL_loadbuffer(L, script, strlen(script), "script");
    (*env)->ReleaseStringUTFChars(env, jscript, script);

    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        throw_lua_error(env, err ? err : "Compilation error");
        lua_pop(L, 1);
        return NULL;
    }

    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        throw_lua_error(env, err ? err : "Execution error");
        lua_pop(L, 1);
        return NULL;
    }

    int nresults = lua_gettop(L) - base;
    jobjectArray res = (*env)->NewObjectArray(env, nresults, g_class_LuaValue, NULL);
    for (int i = 0; i < nresults; i++) {
        jobject v = lua_value_from_stack(env, L, base + 1 + i, ptr);
        (*env)->SetObjectArrayElement(env, res, i, v);
        (*env)->DeleteLocalRef(env, v);
    }
    lua_pop(L, nresults);
    return res;
}

JNIEXPORT jobjectArray JNICALL
Java_com_github_anaruto_lua_Globals_nativeDoFile(JNIEnv *env, jclass clazz, jlong ptr, jstring jpath) {
    (void)clazz;
    lua_State *L = to_lua_state(ptr);
    if (L == NULL || jpath == NULL) {
        throw_lua_error(env, "Invalid Lua state or null file path");
        return NULL;
    }

    const char *path = (*env)->GetStringUTFChars(env, jpath, NULL);
    int base = lua_gettop(L);
    int status = luaL_loadfile(L, path);
    (*env)->ReleaseStringUTFChars(env, jpath, path);

    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        throw_lua_error(env, err ? err : "Failed to load script file");
        lua_pop(L, 1);
        return NULL;
    }

    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        throw_lua_error(env, err ? err : "Execution error");
        lua_pop(L, 1);
        return NULL;
    }

    int nresults = lua_gettop(L) - base;
    jobjectArray res = (*env)->NewObjectArray(env, nresults, g_class_LuaValue, NULL);
    for (int i = 0; i < nresults; i++) {
        jobject v = lua_value_from_stack(env, L, base + 1 + i, ptr);
        (*env)->SetObjectArrayElement(env, res, i, v);
        (*env)->DeleteLocalRef(env, v);
    }
    lua_pop(L, nresults);
    return res;
}

// =========================================================================
// Legacy LuaEngine bindings (Maintained for full backward compatibility)
// =========================================================================

JNIEXPORT jlong JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeNewState(JNIEnv *env, jclass clazz) {
    return Java_com_github_anaruto_lua_Globals_nativeNewState(env, clazz);
}

JNIEXPORT void JNICALL
Java_com_github_anaruto_lua_LuaEngine_nativeClose(JNIEnv *env, jclass clazz, jlong ptr) {
    Java_com_github_anaruto_lua_Globals_nativeClose(env, clazz, ptr);
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
