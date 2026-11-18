#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int main(int argc, char **argv) {
    const char *test_script = (argc > 1) ? argv[1] : "tests/test_suite.lua";

    printf("Initializing Lua State for Functional Testing...\n");
    lua_State *L = luaL_newstate();
    if (L == NULL) {
        fprintf(stderr, "Error: Failed to create Lua state\n");
        return 1;
    }

    luaL_openlibs(L);

    printf("Executing test suite: %s\n", test_script);
    int status = luaL_dofile(L, test_script);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        fprintf(stderr, "FATAL: Test execution failed with error: %s\n", err ? err : "unknown");
        lua_close(L);
        return 1;
    }

    lua_close(L);
    printf("Test runner completed successfully.\n");
    return 0;
}
