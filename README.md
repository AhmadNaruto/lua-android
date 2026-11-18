# lua-android

[![Android Build & Release](https://github.com/AhmadNaruto/lua-android/actions/workflows/build.yml/badge.svg)](https://github.com/AhmadNaruto/lua-android/actions/workflows/build.yml)
[![Sample App Build](https://github.com/AhmadNaruto/lua-android/actions/workflows/sample.yml/badge.svg)](https://github.com/AhmadNaruto/lua-android/actions/workflows/sample.yml)
[![Lua Version](https://img.shields.io/badge/Lua-5.5.1-blue.svg)](https://www.lua.org)
[![HTML Parser](https://img.shields.io/badge/HTML5-Lexbor%20Native-orange.svg)](https://github.com/lexbor/lexbor)
[![API Style](https://img.shields.io/badge/API-LuaJ%20%2F%20NoveLA%20Compatible-green.svg)](https://github.com/AhmadNaruto/lua-android)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Embedded **Lua 5.5.1** runtime berkinerja tinggi untuk Android yang dirancang sebagai pengganti modern (*drop-in replacement*) untuk **LuaJ**. Dilengkapi parser HTML5 **Lexbor C native**, pustaka **cJSON**, POSIX/PCRE **Regex**, utilitas kriptografi C (**AES-128**, **Base64 RFC 4648**, **URL Encode**), serta keselarasan API penuh dengan spesifikasi ekstensi **NoveLA** (`novelx-sources`) dan **Shosetsu**.

---

## ✨ Fitur Utama

* **Core Lua 5.5.1 Upstream**:
  * Menggunakan engine Lua 5.5 terbaru dengan 64-bit integer, bitwise operators native (`<<`, `>>`, `&`, `|`, `~`, `//`), dan efisiensi memori tingkat tinggi.
* **Lexbor HTML5 & CSS Selector Engine Native**:
  * Pengurai HTML5 standar WHATWG tercepat di C (`lhtmllib.c` + `lexbor.c`).
  * 0 alokasi memori pada JVM Java heap saat mem-parsing DOM, menghilangkan masalah *GC pause/jank* pada Android.
  * Mendukung CSS Selectors, manipulasi tree, resolusi Base URI (`abs:href`, `abs:src`, `.href`, `.src`), dan format teks novel (`html_text`).
* **Built-in JSON (cJSON v1.7.18)**:
  * Parser & encoder JSON native C berkecepatan tinggi tanpa perlu `require("json")`.
  * Menyertakan fungsi global `json.decode`, `json.encode`, serta alias NoveLA `json_parse` dan `json_stringify`.
* **Built-in Regex (POSIX + PCRE Shorthand)**:
  * Pattern matching native dengan dukungan sintaks PCRE shorthand (`\d`, `\D`, `\w`, `\W`, `\s`, `\S`).
  * Menyertakan fungsi `regex.match`, `regex.test`, `regex.find`, `regex.replace`, `regex.split`, serta alias global `regex_match` dan `regex_replace`.
* **Native Crypto & Encoding**:
  * Dekripsi AES-128-CBC dengan unpadding otomatis PKCS#7/PKCS#5 (`aes_decrypt`).
  * Encoder/decoder Base64 RFC 4648 (`base64_encode`, `base64_decode`).
  * Standard URL encoder (`url_encode`, `url_encode_charset`).
* **NoveLA Extension Specification Compatibility**:
  * Kompatibel penuh dengan spesifikasi runtime **NoveLA** (`novelx-sources`) untuk Novel, Manga (`getPageList`), dan Video (`getVideoList`).
  * Helper DOM cepat: `html_select`, `html_select_first`, `html_attr`, `html_text`, `html_remove`, `url_resolve`.
  * Utilitas teks & Unicode: `string_trim`, `string_clean`, `string_normalize`, `string_starts_with`, `string_ends_with`, `string_split`, dan `unescape_unicode`.
  * Adapter Kotlin `NovelASupport.kt` dengan dukungan method chaining `http_get(url):toHtml()`.
* **Arsitektur Kotlin Bergaya LuaJ**:
  * `Globals` dan `JsePlatform.standardGlobals()` untuk pengelolaan state.
  * Hierarki polymorphic `LuaValue` (`LuaTable`, `LuaFunction`, `LuaString`, `LuaNumber`, `LuaBoolean`, `LuaNil`, `Varargs`).
  * Thread-safe (`synchronized(this)`) dan pembebasan memori registry C otomatis (*auto-unref*) saat objek Kotlin di-GC.
* **Multi-ABI Android Lengkap**:
  * `arm64-v8a` (64-bit ARM)
  * `armeabi-v7a` (32-bit ARM)
  * `x86_64` (64-bit Intel/AMD)
  * `x86` (32-bit Intel)
* **Sample Android App & 159 Test Suite**:
  * Modul aplikasi demo di folder `sample/` yang memverifikasi seluruh 159 unit test langsung di perangkat Android.
  * Hasil kompilasi APK siap pasang (`sample-debug.apk`) tersedia otomatis di setiap rilis CI/CD.

---

## 📁 Struktur Proyek

```text
lua-android/
├── src/                    # C Engine Lua 5.5.1, Lexbor, cJSON, POSIX Regex, Crypto & JNI
│   ├── lua_jni.c           # JNI Bridge dengan LuaJ-style bindings & JNI_OnLoad caching
│   ├── linit.c             # Registrasi library standar & custom global extensions
│   ├── extensions/
│   │   ├── html/           # Lexbor HTML5 parser & DOM methods (lhtmllib.c, lexbor.c)
│   │   ├── json/           # cJSON engine (ljsonlib.c, cJSON.c)
│   │   ├── regex/          # Regex engine dengan PCRE shorthand (lregexlib.c)
│   │   └── util/           # AES-128, Base64 RFC 4648, URL encode, string utils (lutilib.c)
├── include/                # Public C Headers (lua.h, lauxlib.h, lualib.h, cJSON.h, lexbor.h)
├── kotlin/                 # Kotlin Wrapper (com.github.anaruto.lua)
│   └── com/github/anaruto/lua/
│       ├── Globals.kt       # Global execution context & thread-safe JNI bridge
│       ├── JsePlatform.kt   # Factory helper (JsePlatform.standardGlobals())
│       ├── LuaValue.kt      # Polymorphic base class (type checks, conversions, table ops)
│       ├── LuaTable.kt      # LuaTable & NativeLuaTable (auto-unref lifecycle)
│       ├── LuaFunction.kt   # LuaFunction & NativeLuaFunction (ZeroArg, OneArg, VarArg)
│       ├── Varargs.kt       # Multi-return values container
│       ├── LuaError.kt      # Exception handling
│       └── novela/
│           └── NovelASupport.kt # NoveLA runtime adapter (OkHttp bridge & :toHtml chaining)
├── sample/                 # Sample Android App module
├── tests/
│   └── test_suite.lua      # 159 Unit Tests komprehensif
├── .github/workflows/
│   ├── build.yml           # CI build library NDK 4 ABI
│   └── sample.yml          # CI build sample Android APK
├── CMakeLists.txt          # Konfigurasi CMake dengan dead-code stripping (-Os, -Wl,--gc-sections)
└── sample-debug.apk        # Berkas APK sample Android yang telah terkompilasi
```

---

## 🚀 Panduan Integrasi ke Aplikasi Android

### 1. Salin Biner Native (`liblua.so`)
Unduh artefak rilis atau ambil dari `sample-debug.apk`:
```text
app/src/main/jniLibs/
├── arm64-v8a/liblua.so
├── armeabi-v7a/liblua.so
├── x86/liblua.so
└── x86_64/liblua.so
```

### 2. Salin Kode Wrapper Kotlin
Salin paket `kotlin/com/github/anaruto/lua/` ke direktori sumber aplikasi Anda:
```text
app/src/main/java/com/github/anaruto/lua/
```

### 3. Inisialisasi Dasar
```kotlin
import com.github.anaruto.lua.JsePlatform
import com.github.anaruto.lua.LuaValue

val globals = JsePlatform.standardGlobals()

// Jalankan skrip Lua
val chunk = globals.load("return 'Halo dari ' .. _VERSION")
val result = chunk.call()
println(result.tojstring()) // "Halo dari Lua 5.5.1"

globals.close()
```

---

## 📖 Manual Referensi Fitur Lua Bawaan

### 1. HTML5 Parser & CSS Selectors (`lexbor` / `html_parse`)

Tersedia global tanpa perlu `require`:

```lua
local doc = html_parse([[
    <div id="main" class="container content">
        <h1 class="novel-title">Solo Leveling</h1>
        <p class="summary">Sung Jin-Woo adalah hunter <b>peringkat E</b>.</p>
        <ul class="chapter-list">
            <li><a href="/ch-1">Bab 1</a></li>
            <li><a href="/ch-2">Bab 2</a></li>
        </ul>
    </div>
]], "https://novel.my.id/series/100") -- opsional: Base URI

-- Properti & Method Dokumen
local title = doc:title() -- atau doc.title
local base  = doc:baseUri()
doc:setBaseUri("https://other.com")

-- Seleksi Elemen Pertama
local h1 = doc:selectFirst(".novel-title")
print(h1:text())      -- "Solo Leveling" (atau properti: h1.text)
print(h1:tagName())   -- "h1" (atau properti: h1.tag)
print(h1:attr("class"))

-- Seleksi Daftar Elemen (Elements List)
local links = doc:select(".chapter-list a")
print(#links)               -- 2 (atau links:size(), links:length())
print(links:first():text()) -- "Bab 1"
print(links:last():text())  -- "Bab 2"
print(links:get(2):text())  -- "Bab 2"

-- Resolusi URL Absolut Otomatis
local firstLink = links:first()
print(firstLink:attr("href"))     -- "/ch-1" (relatif)
print(firstLink:attr("abs:href")) -- "https://other.com/ch-1" (absolut)
print(firstLink.href)             -- "https://other.com/ch-1" (properti otomatis absolut)

-- Traversal & Modifikasi
local p = doc:selectFirst("p.summary")
print(p:text())    -- "Sung Jin-Woo adalah hunter peringkat E." (rekursif)
print(p:ownText()) -- "Sung Jin-Woo adalah hunter ." (tanpa teks anak <b>)
print(p:html())    -- inner HTML

local parentLi = firstLink:parent()
local nextLi   = parentLi:nextElementSibling()
parentLi:remove() -- menghapus elemen dari pohon DOM
```

---

### 2. NoveLA Fast DOM Helpers

Fungsi helper global yang dioptimalkan untuk siklus hidup ekstensi NoveLA:

| Fungsi | Deskripsi |
|---|---|
| `html_select(html, css)` | Mengurai HTML dan mengembalikan Elements List yang cocok. |
| `html_select_first(html, css)` | Mengurai HTML dan mengembalikan elemen pertama yang cocok atau `nil`. |
| `html_attr(html, css, attr)` | Mengambil atribut elemen pertama langsung dari string HTML. |
| `html_text(html)` | Mengekstrak teks bersih bab, menjaga `<p>` dan `<br>` sebagai `\n`, membuang `<script>`, `<style>`, `<iframe>`. |
| `html_remove(html, selector)` | Menghapus elemen tertentu dari HTML string (iklan/skrip) dan mengembalikan HTML bersih. |
| `url_resolve(base, href)` | Mengubah URL relatif menjadi URL absolut berdasarkan base URL. |

**Contoh Penggunaan pada Ekstensi Novel:**
```lua
function getChapterText(html, url)
    -- Bersihkan iklan terlebih dahulu
    local clean = html_remove(html, ".ads, script, style, .watermark")
    
    -- Ambil teks bab berformat rapi
    local text = html_text(clean)
    return text
end
```

---

### 3. Utilitas String & Unicode NoveLA

| Fungsi | Deskripsi |
|---|---|
| `string_trim(str)` | Menghapus spasi dan newline di awal & akhir string. |
| `string_clean(str)` / `string_normalize(str)` | Merampingkan spasi berlebih berturut-turut menjadi satu spasi. |
| `string_starts_with(str, prefix)` | Pengecekan prefix (`true`/`false`). |
| `string_ends_with(str, suffix)` | Pengecekan suffix (`true`/`false`). |
| `string_split(str, delimiter)` | Memecah string menjadi array table berdasarkan pemisah. |
| `unescape_unicode(str)` | Mengonversi format `\uXXXX` dan surrogate pair UTF-16 ke karakter UTF-8. |

```lua
local tags = string_split("Action, Fantasy, Romance", ", ")
-- tags = { "Action", "Fantasy", "Romance" }

local clean_title = string_normalize("  Solo    Leveling   Ragnarok  ")
-- clean_title = "Solo Leveling Ragnarok"

local text = unescape_unicode("Halo\\u0020Dunia\\u0021 \\u3000\\u4e00")
-- text = "Halo Dunia! 　一"
```

---

### 4. JSON API (cJSON)

| Fungsi | Deskripsi |
|---|---|
| `json.decode(str)` / `json_parse(str)` | Mengubah string JSON menjadi table Lua. |
| `json.encode(val [, pretty])` / `json_stringify(val)` | Mengubah table Lua menjadi string JSON. |

```lua
local obj = json_parse('{"id": 101, "title": "NoveLA", "active": true}')
print(obj.title) -- "NoveLA"

local jsonString = json_stringify({ code = 200, message = "Success" })
```

---

### 5. Regex API (POSIX + PCRE Shorthand)

| Fungsi | Deskripsi |
|---|---|
| `regex.match(str, pat [, flags])` / `regex_match(str, pat)` | Mengambil string kecocokan penuh dan capture groups. |
| `regex.replace(str, pat, repl [, flags])` / `regex_replace(str, pat, repl)` | Mengganti pola kecocokan dengan string baru (mendukung `$1`, `$2`). |
| `regex.test(str, pat [, flags])` | Cek apakah cocok (`true`/`false`). |
| `regex.find(str, pat [, init, flags])` | Indeks awal dan akhir kecocokan (1-based). |
| `regex.split(str, sep [, flags])` | Memecah string berdasarkan pola regex. |

```lua
local num = regex_match("Chapter 245 End", "\\d+")
-- num = "245"

local out = regex_replace("Harga: Rp 50.000", "Rp (\\d+\\.\\d+)", "IDR $1")
-- out = "Harga: IDR 50.000"
```

---

### 6. Crypto & Encoding API

| Fungsi | Deskripsi |
|---|---|
| `base64_encode(str)` | Encode string ke Base64 RFC 4648. |
| `base64_decode(b64)` | Decode string Base64 kembali ke teks asli. |
| `url_encode(str)` | Encode string ke format URL x-www-form-urlencoded. |
| `aes_decrypt(b64_cipher, key, iv)` | Dekripsi AES-128-CBC dengan unpadding PKCS#7/PKCS#5. Key dan IV harus 16 bytes. |

```lua
local enc = base64_encode("Secret Payload")
local dec = base64_decode(enc)

local plaintext = aes_decrypt(encrypted_b64, "16bytessecretkey", "16bytesinitvecto")
```

---

## 🛠️ Menggunakan Kotlin NoveLA Adapter (`NovelASupport`)

Paket `com.github.anaruto.lua.novela.NovelASupport` menghubungkan engine Lua ke stack jaringan dan UI Android:

```kotlin
import com.github.anaruto.lua.JsePlatform
import com.github.anaruto.lua.novela.NovelASupport
import okhttp3.OkHttpClient
import okhttp3.Request

val globals = JsePlatform.standardGlobals()
val okHttpClient = OkHttpClient()

// Pasang bridge NoveLA
NovelASupport.install(
    globals = globals,
    httpHandler = { method, url, body, config ->
        val reqBuilder = Request.Builder().url(url)
        // Pasang headers dari config jika ada
        val response = okHttpClient.newCall(reqBuilder.build()).execute()
        NovelASupport.HttpResponse(
            success = response.isSuccessful,
            code = response.code,
            body = response.body?.string() ?: "",
            headers = response.headers.toMap()
        )
    },
    cookieHandler = object : NovelASupport.CookieHandler {
        override fun getCookies(url: String): String = "cookie_data=1"
        override fun setCookies(url: String, cookies: String) {}
    },
    preferenceHandler = object : NovelASupport.PreferenceHandler {
        override fun getPreference(key: String): String? = "saved_pref"
        override fun setPreference(key: String, value: String) {}
    },
    errorHandler = { title, message, authUrl ->
        // Munculkan dialog Android error / Cloudflare WebView solver
    }
)

// Ekstensi NoveLA sekarang dapat menggunakan method chaining:
globals.load("""
    local doc = http_get("https://novel.my.id"):toHtml()
    local title = doc:selectFirst(".title"):text()
    print("Judul:", title)
""").call()

globals.close()
```

---

## 🧪 Pengujian & Validasi

Suite pengujian komprehensif memvalidasi seluruh fitur engine:
* **159 Unit Tests Lulus Sempurna (159 Passed, 0 Failed)**:
  1. Core Lua 5.5 VM & Operator Bitwise.
  2. cJSON Encoder/Decoder & Global Aliases.
  3. POSIX/PCRE Regex & Global Aliases.
  4. AES-128-CBC Crypto & Base64 RFC 4648.
  5. OOP String Metatable Getters `('Chapter 1')()`.
  6. Lexbor HTML5 DOM & CSS Selectors Chaining.
  7. Base URI & Absolute URL auto-resolution (`.href`, `.src`).
  8. 100x Selector Re-use Stress Test & Malformed Selector Resilience.
  9. NoveLA Fast DOM Helpers (`html_text`, `html_attr`, `html_remove`, `url_resolve`).
  10. NoveLA String & Unicode Utilities (`string_trim`, `unescape_unicode`).
  11. Simulasi Lengkap Siklus Hidup Ekstensi Novel, Manga (`getPageList`), dan Video (`getVideoList`).

---

## 📄 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE).
Komponen pustaka pihak ketiga:
- **Lua**: MIT License (PUC-Rio).
- **Lexbor**: Apache License 2.0.
- **cJSON**: MIT License (Dave Gamble).
