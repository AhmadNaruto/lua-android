/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB


#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "llimits.h"
#include "extensions/html/lhtmllib.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
static const luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {NULL, NULL}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API void luaL_openselectedlibs (lua_State *L, int load, int preload) {
  int mask;
  const luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  for (lib = stdlibs, mask = 1; lib->name != NULL; lib++, mask <<= 1) {
    if (load & mask) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_pop(L, 1);  /* remove result from the stack */
    }
    else if (preload & mask) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((mask >> 1) == LUA_UTF8LIBK);

  /* preload custom extension modules */
  lua_pushcfunction(L, luaopen_json);
  lua_setfield(L, -2, LUA_JSONLIBNAME);

  lua_pushcfunction(L, luaopen_regex);
  lua_setfield(L, -2, LUA_REGEXLIBNAME);

  lua_pushcfunction(L, luaopen_util);
  lua_setfield(L, -2, LUA_UTILLIBNAME);

  lua_pushcfunction(L, luaopen_html);
  lua_setfield(L, -2, LUA_HTMLLIBNAME);

  lua_pushcfunction(L, luaopen_html);
  lua_setfield(L, -2, "lexbor");

  lua_pop(L, 1);  /* remove PRELOAD table */

  /* if loading all/standard libraries, make json, regex, crypto/util, html, and lexbor available globally */
  if (load != 0) {
    luaL_requiref(L, LUA_JSONLIBNAME, luaopen_json, 1);
    lua_pop(L, 1);
    luaL_requiref(L, LUA_REGEXLIBNAME, luaopen_regex, 1);
    lua_pop(L, 1);
    luaL_requiref(L, LUA_UTILLIBNAME, luaopen_util, 1);
    /* export base64_encode, base64_decode, url_encode, aes_decrypt to global environment */
    lua_getfield(L, -1, "base64_encode");
    lua_setglobal(L, "base64_encode");
    lua_getfield(L, -1, "base64_decode");
    lua_setglobal(L, "base64_decode");
    lua_getfield(L, -1, "url_encode");
    lua_setglobal(L, "url_encode");
    lua_getfield(L, -1, "url_encode_charset");
    lua_setglobal(L, "url_encode_charset");
    lua_getfield(L, -1, "aes_decrypt");
    lua_setglobal(L, "aes_decrypt");
    lua_pop(L, 1);
    luaL_requiref(L, LUA_HTMLLIBNAME, luaopen_html, 1);
    /* also register global table 'lexbor' pointing to the same library */
    lua_pushvalue(L, -1);
    lua_setglobal(L, "lexbor");
    /* export html_parse to global environment */
    lua_getfield(L, -1, "html_parse");
    lua_setglobal(L, "html_parse");
    lua_pop(L, 1);
  }
}

