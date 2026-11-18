# lua-android

[![Android Build & Release](https://github.com/AhmadNaruto/lua-android/actions/workflows/build.yml/badge.svg)](https://github.com/AhmadNaruto/lua-android/actions/workflows/build.yml)
[![Lua Version](https://img.shields.io/badge/Lua-5.5.1-blue.svg)](https://www.lua.org)
[![API Style](https://img.shields.io/badge/API-LuaJ%20Compatible-green.svg)](https://github.com/AhmadNaruto/lua-android)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Embedded **Lua 5.5.1** runtime untuk platform Android dengan pustaka bawaan **JSON** dan **Regex (PCRE/POSIX)** yang siap pakai tanpa `require`, dilengkapi arsitektur wrapper Kotlin yang mengadopsi gaya API **LuaJ** (`Globals`, `LuaValue`, `Varargs`, `JsePlatform`).

---

## ✨ Fitur Utama

* **Lua 5.5.1 Upstream**: Menggunakan mesin core Lua terbaru dengan virtual machine cepat, 64-bit integer native, dan efisiensi memori tinggi.
* **LuaJ-Compatible API Paradigm**: Mengadaptasi antarmuka populer **LuaJ** (`org.luaj.vm2`):
  * `Globals` dan `JsePlatform.standardGlobals()` untuk pengelolaan global environment.
  * Hierarki polymorphic `LuaValue` (`LuaNil`, `LuaBoolean`, `LuaInteger`, `LuaDouble`, `LuaString`, `LuaTable`, `LuaFunction`).
  * Method type checking & conversion (`isnil()`, `isstring()`, `tojstring()`, `toint()`, `todouble()`, dll.).
  * Multiple return values via `Varargs` (`arg(1)`, `narg()`, `subargs(start)`).
  * Kotlin operator overloading: `globals["key"]`, `table["field"] = value`, `func(arg1, arg2)`.
* **Built-in JSON (`json`)**: Menggunakan parser **cJSON** v1.7.18 C-library. Aktif global (`json.decode`, `json.encode`) tanpa perlu `require("json")`.
* **Built-in Regex (`regex`)**: Menggunakan engine POSIX regex native Android Bionic libc dengan dukungan shorthand PCRE (`\d`, `\D`, `\w`, `\W`, `\s`, `\S`). Aktif global (`regex.match`, `regex.test`, `regex.find`, `regex.find_all`, `regex.replace`, `regex.split`) tanpa perlu `require("regex")`.
* **High-Performance JNI Bridge**: Menggunakan caching `jclass` & `jmethodID` global pada `JNI_OnLoad` untuk meminimalkan JNI call overhead, proteksi memory leak local reference, dan penggunaan `LUA_REGISTRYINDEX` untuk retensi objek GC yang aman.
* **Backward Compatible**: Menyertakan `LuaEngine` lama bagi kode yang ingin menggunakan gaya direct procedural.
* **Multi-ABI Android**: Mendukung seluruh 4 arsitektur Android:
  * `arm64-v8a` (64-bit ARM)
  * `armeabi-v7a` (32-bit ARM, dengan fallback `l_fseek`/`l_ftell` untuk Android API < 24)
  * `x86_64` (64-bit Intel/AMD)
  * `x86` (32-bit Intel)
* **Automasi CI/CD (GitHub Actions)**: Build otomatis pada setiap push dengan mode `test` (verifikasi biner ELF 4 ABI) dan mode `release` (publikasi GitHub Release dan paket `.zip` & `.tar.gz`).

---

## 📁 Struktur Direktori

```text
lua-android/
├── src/                # Core C Lua 5.5.1, cJSON, ljsonlib, lregexlib, dan JNI bridge
│   ├── lua_jni.c       # JNI Bridge dengan LuaJ-style bindings & JNI_OnLoad caching
│   ├── ljsonlib.c      # Built-in JSON module
│   └── lregexlib.c     # Built-in Regex module
├── include/            # Public C headers (lua.h, lauxlib.h, lualib.h, luaconf.h, cJSON.h)
├── kotlin/             # Kotlin Wrapper (com.github.anaruto.lua)
│   └── com/github/anaruto/lua/
│       ├── Globals.kt           # Global state controller (LuaJ Globals)
│       ├── JsePlatform.kt       # Platform factory (JsePlatform.standardGlobals())
│       ├── LuaValue.kt          # Polymorphic base class (LuaValue)
│       ├── LuaNil.kt            # Nil value representation
│       ├── LuaBoolean.kt        # Boolean value representation
│       ├── LuaNumber.kt         # LuaInteger (64-bit) & LuaDouble representation
│       ├── LuaString.kt         # String value representation
│       ├── LuaTable.kt          # LuaTable & NativeLuaTable
│       ├── LuaFunction.kt       # LuaFunction & NativeLuaFunction
│       ├── Varargs.kt           # Multi-return values container
│       ├── LuaError.kt          # LuaError (subclass of LuaException)
│       ├── LuaEngine.kt         # Legacy engine wrapper
│       └── LuaException.kt      # Base exception
├── .github/workflows/  # GitHub Actions CI/CD (build.yml)
├── CMakeLists.txt      # Konfigurasi CMake build
├── Android.mk          # Konfigurasi ndk-build legacy
└── build.sh            # Skrip otomatisasi build lokal multi-ABI
```

---

## 🚀 Panduan Integrasi ke Aplikasi Android

### Cara 1: Menggunakan Prebuilt Binaries (Rekomendasi)
1. Unduh arsip rilis `lua-android-*.zip` dari halaman [Releases](https://github.com/AhmadNaruto/lua-android/releases) atau artifact GitHub Actions.
2. Ekstrak dan salin biner ke folder aplikasi Android Anda:
   ```text
   app/src/main/jniLibs/
   ├── arm64-v8a/liblua.so
   ├── armeabi-v7a/liblua.so
   ├── x86/liblua.so
   └── x86_64/liblua.so
   ```
3. Salin paket wrapper Kotlin dari `kotlin/com/github/anaruto/lua/` ke folder project Android:
   ```text
   app/src/main/java/com/github/anaruto/lua/
   ```

---

## 💻 Contoh Penggunaan (Gaya API LuaJ)

### 1. Inisialisasi dan Evaluasi Dasar
```kotlin
import com.github.anaruto.lua.JsePlatform
import com.github.anaruto.lua.LuaValue

// Inisialisasi globals ala LuaJ
val globals = JsePlatform.standardGlobals()

// Load script chunk dan eksekusi
val chunk = globals.load("return 'Hello from ' .. _VERSION")
val result = chunk.call()
println(result.tojstring()) // "Hello from Lua 5.5.1"

globals.close()
```

### 2. Mengakses dan Memanipulasi Table / Global Variables
```kotlin
JsePlatform.standardGlobals().use { globals ->
    // Set global value
    globals.set("appName", "NoveLA")
    globals.set("versionCode", 102)

    // Kotlin operator indexing: globals["key"]
    globals["isDebug"] = LuaValue.valueOf(true)

    // Akses library standar Lua
    val math = globals.get("math")
    val sqrt = math.get("sqrt")
    val res = sqrt.call(LuaValue.valueOf(144))
    println(res.todouble()) // 12.0
}
```

### 3. Menggunakan Fitur Bawaan JSON & Regex (Tanpa `require`)
```kotlin
JsePlatform.standardGlobals().use { globals ->
    val script = """
        -- Regex PCRE shorthand (\d, \w)
        local match, id, user = regex.match("Order #9921 from Budi", "Order #(\\d+) from (\\w+)")

        -- Encode ke JSON
        local payload = json.encode({
            order_id = tonumber(id),
            customer = user,
            verified = regex.test(user, "^[A-Z]")
        })
        return payload
    """

    val chunk = globals.load(script)
    val jsonString = chunk.call().tojstring()
    println(jsonString)
    // {"order_id":9921,"customer":"Budi","verified":true}
}
```

### 4. Menangani Multiple Return Values (`Varargs`)
```kotlin
JsePlatform.standardGlobals().use { globals ->
    val chunk = globals.load("return 'apple', 42, true")
    val varargs = chunk.invoke()

    println(varargs.narg())              // 3
    println(varargs.arg(1).tojstring())  // "apple"
    println(varargs.arg(2).toint())      // 42
    println(varargs.arg(3).toboolean())  // true
}
```

### 5. Mendaftarkan Fungsi Kotlin ke Lua (Host Callbacks)
Anda dapat mengekspos fungsi atau method Kotlin/Android agar dapat dipanggil langsung dari dalam skrip Lua:
```kotlin
JsePlatform.standardGlobals().use { globals ->
    // Cara 1: Menggunakan lambda register yang ringkas
    globals.register("showNotification") { args ->
        val title = args.arg(1).tojstring()
        val count = args.arg(2).optint(1)
        println("Notifikasi: $title ($count)")
        LuaValue.TRUE // Nilai kembalian ke Lua
    }

    // Cara 2: Menggunakan turunan OneArgFunction / TwoArgFunction / VarArgFunction
    globals.set("tambah", object : TwoArgFunction() {
        override fun call(arg1: LuaValue, arg2: LuaValue): LuaValue {
            return LuaValue.valueOf(arg1.todouble() + arg2.todouble())
        }
    })

    // Panggil dari skrip Lua
    globals.load("""
        local ok = showNotification("Pesan Baru", 3)
        print("Hasil penjumlahan:", tambah(10, 25))
    """).call()
}
```

### 6. Integrasi Android Logcat & Asset Loading
* **Android Logcat**: Pemanggilan `print(...)` di Lua secara otomatis dialirkan ke **Logcat Android** (`tag: Lua`, level: `INFO`), sehingga debugging di Android Studio sangat mudah tanpa konfigurasi tambahan.
* **Loading dari APK `assets/`**:
```kotlin
// 1. Eksekusi skrip langsung dari assets/
val mainChunk = globals.loadAsset(context.assets, "scripts/main.lua")
mainChunk.call()

// 2. Pasang automatic package searcher untuk require()
// require("network.api") otomatis memuat file assets/lua/network/api.lua!
globals.installAssetSearcher(context.assets, basePath = "lua")
```

### 7. Menangani Exception
```kotlin
try {
    globals.load("function broken() return end broken(1").call()
} catch (e: LuaError) {
    println("Lua compilation/runtime error: ${e.message}")
}
```

---

## 📜 Contoh API Lua Bawaan

### 1. JSON API (`json`)
| Fungsi | Deskripsi |
| :--- | :--- |
| `json.decode(str)` | Mengonversi string JSON menjadi table Lua. Mengembalikan `nil, err` jika format tidak valid. |
| `json.encode(table [, pretty])` | Mengonversi table Lua menjadi string JSON. Berikan `true` untuk format rapi (*pretty-print*). |

```lua
-- Decode
local data = json.decode('{"title":"NoveLA","chapters":[1,2,3]}')
print(data.title)        -- "NoveLA"
print(data.chapters[1])  -- 1

-- Encode
local str = json.encode({ status = "ok", code = 200 })
print(str)               -- {"status":"ok","code":200}
```

### 2. Regex API (`regex`)
| Fungsi | Deskripsi |
| :--- | :--- |
| `regex.test(str, pattern [, flags])` | Cek apakah string cocok dengan pola (`true`/`false`). |
| `regex.match(str, pattern [, flags])` | Mengembalikan kecocokan teks penuh dan hasil *capture groups*. |
| `regex.find(str, pattern [, init, flags])` | Mengembalikan indeks awal & akhir (`start, end`) kecocokan (1-based). |
| `regex.find_all(str, pattern [, flags])` | Mengembalikan array table berisi seluruh substring yang cocok. |
| `regex.replace(str, pattern, repl [, limit, flags])` | Mengganti kecocokan regex dengan string pengganti (mendukung `$1`, `$2`, dll). |
| `regex.split(str, delimiter [, limit, flags])` | Memecah string menjadi table berdasarkan pola pemisah regex. |

*Flags yang didukung*: `"i"` (case-insensitive), `"m"` (multiline).

---

## 🔨 Build Lokal (NDK)

```bash
export ANDROID_NDK_HOME="/path/to/android-sdk/ndk/25.1.8937393"
chmod +x build.sh
./build.sh
```

---

## 📄 Lisensi
Proyek ini dilisensikan di bawah [MIT License](LICENSE) (sesuai lisensi resmi Lua dan cJSON).
