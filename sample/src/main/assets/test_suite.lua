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

-- 6. Native HTML Parser (Lexbor C99 Engine & CSS Selectors)
print("\n[6] Native HTML Parser (Lexbor DOM & CSS Selectors)")

-- Verify Lexbor is accessible globally without require
assert_true(type(html_parse) == "function", "Global 'html_parse' is registered without require")
assert_true(type(lexbor) == "table", "Global 'lexbor' table is registered without require")
assert_true(type(lexbor.parse) == "function", "lexbor.parse is a function")
assert_true(type(lexbor.html_parse) == "function", "lexbor.html_parse is a function")
assert_true(type(html) == "table", "Global 'html' table is registered without require")
assert_true(type(html.parse) == "function", "html.parse is a function")

-- Verify require("lexbor") and require("html") also work seamlessly
local req_lexbor = require("lexbor")
assert_true(type(req_lexbor) == "table", "require('lexbor') works and returns table")
assert_true(type(req_lexbor.parse) == "function", "require('lexbor').parse is a function")

local sample_html = [[
<!DOCTYPE html>
<html>
<head>
    <title>NoveLA Test Page</title>
</head>
<body>
    <div id="main" class="container content">
        <h1 class="novel-title">Solo Leveling</h1>
        <p class="summary">Great <b>hunter</b> story.</p>
        <ul class="chapter-list">
            <li class="chapter-item"><a href="/ch-1">Chapter 1</a></li>
            <li class="chapter-item"><a href="/ch-2">Chapter 2</a></li>
            <li class="chapter-item"><a href="/ch-3">Chapter 3</a></li>
        </ul>
    </div>
</body>
</html>
]]

-- Test both global html_parse and lexbor.parse
local doc_lexbor = lexbor.parse(sample_html)
assert_true(doc_lexbor ~= nil, "lexbor.parse parses valid HTML document without require")
assert_eq(doc_lexbor:title(), "NoveLA Test Page", "doc_lexbor:title() == 'NoveLA Test Page'")

local doc = html_parse(sample_html)
assert_true(doc ~= nil, "html_parse parses valid HTML document")
assert_eq(doc:title(), "NoveLA Test Page", "doc:title() extracts <title>")
assert_eq(doc.title, "NoveLA Test Page", "doc.title callable/property extracts <title>")

-- 6.1 Elements Selection & Jsoup-like methods
local title_el = doc:selectFirst(".novel-title")
assert_true(title_el ~= nil, "doc:selectFirst('.novel-title') finds h1")
assert_eq(title_el:text(), "Solo Leveling", "title_el:text() returns text")
assert_eq(title_el.text, "Solo Leveling", "title_el.text property returns text")
assert_eq(title_el:tagName(), "h1", "title_el:tagName() == 'h1'")
assert_eq(title_el.tag, "h1", "title_el.tag property == 'h1'")

local main_div = doc:selectFirst("div#main")
assert_true(main_div ~= nil, "doc:selectFirst('div#main') finds main div")
assert_eq(main_div:attr("id"), "main", "main_div:attr('id') == 'main'")
assert_eq(main_div.id, "main", "main_div.id == 'main'")
assert_true(main_div:hasClass("container"), "main_div:hasClass('container') is true")
assert_true(main_div:hasClass("content"), "main_div:hasClass('content') is true")
assert_eq(main_div:hasClass("nonexistent"), false, "main_div:hasClass('nonexistent') is false")
assert_true(main_div:hasAttr("class"), "main_div:hasAttr('class') is true")

-- 6.2 Recursive text vs ownText
local summary_el = doc:selectFirst("p.summary")
assert_true(summary_el ~= nil, "doc:selectFirst('p.summary') finds summary paragraph")
assert_eq(summary_el:text(), "Great hunter story.", "summary_el:text() extracts recursive text")
assert_eq(summary_el:ownText(), "Great story.", "summary_el:ownText() excludes child <b> text")
assert_eq(summary_el:html(), "Great <b>hunter</b> story.", "summary_el:html() extracts inner HTML")

-- 6.3 List & Chaining
local chapters = doc:select(".chapter-list a")
assert_eq(chapters:size(), 3, "chapters:size() == 3")
assert_eq(#chapters, 3, "#chapters == 3")
assert_eq(chapters:isEmpty(), false, "chapters:isEmpty() == false")
assert_eq(chapters:first():text(), "Chapter 1", "chapters:first():text() == 'Chapter 1'")
assert_eq(chapters:last():text(), "Chapter 3", "chapters:last():text() == 'Chapter 3'")
assert_eq(chapters:get(2):text(), "Chapter 2", "chapters:get(2):text() == 'Chapter 2'")
assert_eq(chapters:attr("href"), "/ch-1", "chapters:attr('href') gets href from first element")
assert_eq(chapters:first().href, "/ch-1", "chapters:first().href gets href property")
assert_eq(chapters:text(), "Chapter 1 Chapter 2 Chapter 3", "chapters:text() joins all chapter names")

local count_loop = 0
chapters:forEach(function(el, i)
    count_loop = count_loop + 1
    assert_eq(el:text(), "Chapter " .. i, "chapters:forEach iteration " .. i)
end)
assert_eq(count_loop, 3, "forEach executed exactly 3 times")

-- 6.4 DOM Traversal & Modification
local first_a = chapters:first()
local li_parent = first_a:parent()
assert_true(li_parent ~= nil, "first_a:parent() returns parent <li>")
assert_eq(li_parent:tagName(), "li", "parent tagName is 'li'")

local ul_el = doc:selectFirst("ul.chapter-list")
assert_eq(ul_el:childrenSize(), 3, "ul_el:childrenSize() == 3")
local first_li = ul_el:child(1)
assert_true(first_li ~= nil, "ul_el:child(1) returns first <li>")
local second_li = first_li:nextElementSibling()
assert_true(second_li ~= nil, "first_li:nextElementSibling() returns second <li>")
assert_eq(second_li:selectFirst("a"):text(), "Chapter 2", "second_li contains Chapter 2")

-- Test remove
second_li:remove()
local chapters_after_remove = doc:select(".chapter-list a")
assert_eq(chapters_after_remove:size(), 2, "After remove(), chapter count decreases to 2")
assert_eq(chapters_after_remove:get(2):text(), "Chapter 3", "Remaining chapter 2 is now Chapter 3")

-- Summary
print("\n==========================================")
print(string.format("Test Results: %d Passed, %d Failed", passed, failed))
print("==========================================")

if failed > 0 then
    os.exit(1)
end
