#define lutilib_c
#define LUA_LIB

#include "lprefix.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "b64.h"
#include "aes.h"

static int l_base64_encode(lua_State *L) {
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    char *encoded = b64_encode((const unsigned char *)str, len);
    if (encoded == NULL) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, encoded);
    free(encoded);
    return 1;
}

static int l_base64_decode(lua_State *L) {
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    size_t out_len = 0;
    unsigned char *decoded = b64_decode_ex(str, len, &out_len);
    if (decoded == NULL) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushlstring(L, (const char *)decoded, out_len);
    free(decoded);
    return 1;
}

static int l_url_encode(lua_State *L) {
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    // Ignore optional charset argument 2 for standard RFC 3986 / x-www-form-urlencoded
    luaL_Buffer b;
    luaL_buffinit(L, &b);

    static const char hex[] = "0123456789ABCDEF";

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            luaL_addchar(&b, c);
        } else if (c == ' ') {
            luaL_addchar(&b, '+');
        } else {
            luaL_addchar(&b, '%');
            luaL_addchar(&b, hex[(c >> 4) & 0x0F]);
            luaL_addchar(&b, hex[c & 0x0F]);
        }
    }

    luaL_pushresult(&b);
    return 1;
}

static int l_aes_decrypt(lua_State *L) {
    size_t cipher_b64_len = 0, key_len = 0, iv_len = 0;
    const char *cipher_b64 = luaL_checklstring(L, 1, &cipher_b64_len);
    const char *key = luaL_checklstring(L, 2, &key_len);
    const char *iv = luaL_checklstring(L, 3, &iv_len);

    if (key_len != 16) {
        return luaL_error(L, "aes_decrypt: Key must be exactly 16 bytes for AES-128, got %d", (int)key_len);
    }
    if (iv_len != 16) {
        return luaL_error(L, "aes_decrypt: IV must be exactly 16 bytes, got %d", (int)iv_len);
    }

    size_t raw_len = 0;
    unsigned char *raw_buf = b64_decode_ex(cipher_b64, cipher_b64_len, &raw_len);
    if (raw_buf == NULL || raw_len == 0 || (raw_len % 16) != 0) {
        if (raw_buf) free(raw_buf);
        lua_pushnil(L);
        lua_pushliteral(L, "aes_decrypt: Invalid Base64 ciphertext or length not multiple of 16");
        return 2;
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (const uint8_t *)key, (const uint8_t *)iv);
    AES_CBC_decrypt_buffer(&ctx, raw_buf, raw_len);

    // PKCS#7 / PKCS#5 Unpadding
    uint8_t pad = raw_buf[raw_len - 1];
    size_t plain_len = raw_len;
    if (pad >= 1 && pad <= 16) {
        int valid_pad = 1;
        for (size_t i = raw_len - pad; i < raw_len; i++) {
            if (raw_buf[i] != pad) {
                valid_pad = 0;
                break;
            }
        }
        if (valid_pad) {
            plain_len = raw_len - pad;
        }
    }

    lua_pushlstring(L, (const char *)raw_buf, plain_len);
    free(raw_buf);
    return 1;
}

static const struct luaL_Reg utillib[] = {
    {"base64_encode", l_base64_encode},
    {"base64_decode", l_base64_decode},
    {"url_encode", l_url_encode},
    {"url_encode_charset", l_url_encode},
    {"aes_decrypt", l_aes_decrypt},
    {NULL, NULL}
};

LUAMOD_API int luaopen_util(lua_State *L) {
    luaL_newlib(L, utillib);
    return 1;
}
