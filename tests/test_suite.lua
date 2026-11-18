-- Comprehensive Test Suite for lua-android
local passed = 0
local failed = 0

local function assert_eq(actual, expected, test_name)
    if actual == expected then
        passed = passed + 1
        print("  ✓ PASS: " .. test_name)
    else
        failed = failed + 1
        print("  ✗ FAIL: " .. test_name .. " | Expected: " .. tostring(expected) .. ", Got: " .. tostring(actual))
    end
end

local function assert_true(cond, test_name)
    assert_eq(cond, true, test_name)
end

print("==========================================")
print("Running lua-android Functional Test Suite")
print("==========================================")

-- 1. Core Lua 5.5 Features
print("\n[1] Core Lua Engine & Bitwise Operators")
assert_true(string.find(_VERSION, "Lua 5%.5") ~= nil, "Lua version is 5.5")
assert_eq(1 << 4, 16, "Bitwise left shift (1 << 4 == 16)")
assert_eq(0xFF & 0x0F, 0x0F, "Bitwise AND (0xFF & 0x0F == 0x0F)")
assert_eq(0x01 | 0x10, 0x11, "Bitwise OR (0x01 | 0x10 == 0x11)")
assert_eq(10 // 3, 3, "Floor division (10 // 3 == 3)")

-- 2. Built-in JSON Library
print("\n[2] Built-in JSON Library (cJSON)")
assert_true(type(json) == "table", "Global 'json' table exists without require")
assert_true(type(json.encode) == "function", "json.encode is a function")
assert_true(type(json.decode) == "function", "json.decode is a function")

local test_data = {
    title = "NoveLA",
    version = 100,
    active = true,
    tags = {"action", "fantasy"}
}
local encoded_json = json.encode(test_data)
assert_true(type(encoded_json) == "string", "json.encode returns string")
assert_true(string.find(encoded_json, '"title":"NoveLA"') ~= nil, "JSON contains title field")
assert_true(string.find(encoded_json, '"version":100') ~= nil, "JSON contains version field")

local decoded_data = json.decode(encoded_json)
assert_true(type(decoded_data) == "table", "json.decode returns table")
assert_eq(decoded_data.title, "NoveLA", "Decoded string field matches")
assert_eq(decoded_data.version, 100, "Decoded numeric field matches")
assert_eq(decoded_data.active, true, "Decoded boolean field matches")
assert_eq(decoded_data.tags[1], "action", "Decoded array element 1 matches")
assert_eq(decoded_data.tags[2], "fantasy", "Decoded array element 2 matches")

-- 3. Built-in Regex Library
print("\n[3] Built-in Regex Library (POSIX with PCRE shorthand)")
assert_true(type(regex) == "table", "Global 'regex' table exists without require")
assert_true(type(regex.test) == "function", "regex.test is a function")
assert_true(type(regex.match) == "function", "regex.match is a function")
assert_true(type(regex.replace) == "function", "regex.replace is a function")
assert_true(type(regex.split) == "function", "regex.split is a function")

-- Test PCRE shorthand \d and \w
assert_true(regex.test("Order #9921", "\\d+"), "regex.test with \\d+ shorthand")
assert_true(regex.test("Hello_World", "^\\w+$"), "regex.test with \\w+ shorthand")

local full, id, user = regex.match("Order #452: Ahmad", "Order #(\\d+): (\\w+)")
assert_eq(full, "Order #452: Ahmad", "regex.match full capture")
assert_eq(id, "452", "regex.match group 1 (\\d+)")
assert_eq(user, "Ahmad", "regex.match group 2 (\\w+)")

local replaced = regex.replace("10/11/2026", "(\\d{2})/(\\d{2})/(\\d{4})", "$3-$2-$1")
assert_eq(replaced, "2026-11-10", "regex.replace with backreferences")

local parts = regex.split("apel, jeruk; mangga", "[,;]\\s*")
assert_true(type(parts) == "table", "regex.split returns table")
assert_eq(parts[1], "apel", "regex.split item 1")
assert_eq(parts[2], "jeruk", "regex.split item 2")
assert_eq(parts[3], "mangga", "regex.split item 3")

-- 4. Built-in C Native Crypto & Encoding Utilities (100% Native C, No JNI bridge)
print("\n[4] Built-in C Native Crypto & Encoding Utilities")
assert_true(type(base64_encode) == "function", "Global 'base64_encode' is registered without require")
assert_true(type(base64_decode) == "function", "Global 'base64_decode' is registered without require")
assert_true(type(url_encode) == "function", "Global 'url_encode' is registered without require")
assert_true(type(url_encode_charset) == "function", "Global 'url_encode_charset' is registered without require")
assert_true(type(aes_decrypt) == "function", "Global 'aes_decrypt' is registered without require")

-- 4.1 Base64 Tests
local b64_1 = base64_encode("Hello World")
assert_eq(b64_1, "SGVsbG8gV29ybGQ=", "base64_encode('Hello World') matches standard RFC 4648")
local dec_1 = base64_decode(b64_1)
assert_eq(dec_1, "Hello World", "base64_decode roundtrip matches original text")

local b64_2 = base64_encode("NoveLA")
assert_eq(b64_2, "Tm92ZUxB", "base64_encode('NoveLA')")
assert_eq(base64_decode("Tm92ZUxB"), "NoveLA", "base64_decode('Tm92ZUxB')")

-- 4.2 URL Encode Tests
local url_raw = "hello world & kawan"
local url_enc = url_encode(url_raw)
assert_eq(url_enc, "hello+world+%26+kawan", "url_encode spaces and special characters RFC 3986")
assert_eq(url_encode_charset("foo bar", "UTF-8"), "foo+bar", "url_encode_charset accepts charset param")

-- 4.3 AES-128-CBC Decrypt Tests
local key1 = "1234567890123456"
local iv1  = "abcdefghijklmnop"
local cipher1 = "G/C9PfrDE5jrYKaB0iNQDlxlKwhlMhK+OhLdoV1VxnI="
local plain1 = aes_decrypt(cipher1, key1, iv1)
assert_eq(plain1, "NoveLA-Secret-2026", "aes_decrypt vector 1 ('NoveLA-Secret-2026') with PKCS#7 unpadding")

local key2 = "my_secret_key_16"
local iv2  = "my_iv_param_1234"
local cipher2 = "p5il/RYSxc2cs0+Uj9o8ZA=="
local plain2 = aes_decrypt(cipher2, key2, iv2)
assert_eq(plain2, "hello", "aes_decrypt vector 2 ('hello') with PKCS#7 unpadding")

-- Error handling checks
local ok_invalid_key = pcall(function() aes_decrypt(cipher1, "short_key", iv1) end)
assert_eq(ok_invalid_key, false, "aes_decrypt rejects invalid key size")

local ok_invalid_iv = pcall(function() aes_decrypt(cipher1, key1, "short_iv") end)
assert_eq(ok_invalid_iv, false, "aes_decrypt rejects invalid IV size")

-- 5. Shosetsu / NoveLA String Callable Idiom
print("\n[5] Shosetsu / NoveLA OOP String Metatable")
local test_str = "Chapter 1"
local ok_call, res_call = pcall(function() return test_str() end)
if ok_call then
    assert_eq(res_call, "Chapter 1", "String instance invoked as getter: ('Chapter 1')() returns 'Chapter 1'")
else
    print("  - String __call not enabled in standalone interpreter")
end

-- Summary
print("\n==========================================")
print(string.format("Test Results: %d Passed, %d Failed", passed, failed))
print("==========================================")

if failed > 0 then
    os.exit(1)
end
