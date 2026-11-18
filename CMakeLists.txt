cmake_minimum_required(VERSION 3.10)

project(lua LANGUAGES C)

add_library(${PROJECT_NAME} SHARED
    src/lapi.c
    src/lauxlib.c
    src/lbaselib.c
    src/lcode.c
    src/lcorolib.c
    src/lctype.c
    src/ldblib.c
    src/ldebug.c
    src/ldo.c
    src/ldump.c
    src/lfunc.c
    src/lgc.c
    src/linit.c
    src/liolib.c
    src/llex.c
    src/lmathlib.c
    src/lmem.c
    src/loadlib.c
    src/lobject.c
    src/lopcodes.c
    src/loslib.c
    src/lparser.c
    src/lstate.c
    src/lstring.c
    src/lstrlib.c
    src/ltable.c
    src/ltablib.c
    src/ltm.c
    src/lundump.c
    src/lutf8lib.c
    src/lvm.c
    src/lzio.c
    src/cJSON.c
    src/ljsonlib.c
    src/lregexlib.c
    src/aes.c
    src/b64_buffer.c
    src/b64_encode.c
    src/b64_decode.c
    src/lutilib.c
)

if(ANDROID)
    target_sources(${PROJECT_NAME} PRIVATE src/lua_jni.c)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    LUA_USE_LINUX
    "lua_getlocaledecpoint()='.'"
)

find_library(LIBDL dl)
find_library(LIBLOG log)

set(EXTRA_LIBS m)
if(LIBDL)
    list(APPEND EXTRA_LIBS ${LIBDL})
endif()
if(LIBLOG)
    list(APPEND EXTRA_LIBS ${LIBLOG})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTRA_LIBS})
