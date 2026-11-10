# lua-android

[![Android Build & Release](https://github.com/AhmadNaruto/lua-android/actions/workflows/build.yml/badge.svg)](https://github.com/AhmadNaruto/lua-android/actions/workflows/build.yml)
[![Lua Version](https://img.shields.io/badge/Lua-5.5.1-blue.svg)](https://www.lua.org)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Embedded **Lua 5.5.1** runtime untuk platform Android dengan pustaka bawaan **JSON** dan **Regex (PCRE/POSIX)** yang siap pakai tanpa `require`, dilengkapi pembungkus (**Kotlin JNI Wrapper**) idiomatik.

---

## ✨ Fitur Utama

* **Lua 5.5.1**: Menggunakan rilis core Lua terbaru dengan performa virtual machine tinggi dan konsumsi memori rendah.
* **Built-in JSON (`json`)**: Menggunakan parser **cJSON** yang sangat cepat dan ringan. Tersedia langsung di scope global (`json.decode`, `json.encode`) tanpa perlu `require("json")`.
* **Built-in Regex (`regex`)**: Menggunakan engine POSIX regex native Android Bionic libc dengan dukungan shorthand PCRE (`\d`, `\D`, `\w`, `\W`, `\s`, `\S`). Tersedia langsung di scope global (`regex.match`, `regex.test`, `regex.find`, `regex.find_all`, `regex.replace`, `regex.split`) tanpa perlu `require("regex")`.
* **Kotlin JNI Wrapper (`LuaEngine`)**: Pembungkus siap pakai dengan package `com.github.anaruto.lua` yang mendukung blok `.use { ... }` untuk manajemen memori otomatis dan penanganan error terstruktur via `LuaException`.
* **Multi-ABI Android**: Mendukung seluruh arsitektur Android:
  * `arm64-v8a` (64-bit ARM)
  * `armeabi-v7a` (32-bit ARM)
  * `x86_64` (64-bit Intel/AMD)
  * `x86` (32-bit Intel)
* **Automasi CI/CD (GitHub Actions)**: Build otomatis pada push ke branch manapun dengan mode `test` (verifikasi & artifact pengujian) dan mode `release` (membuat rilis otomatis beserta paket distribusi `.zip` & `.tar.gz`).

---

## 📁 Struktur Direktori

```text
lua-android/
├── src/                # Core C Lua 5.5.1, cJSON, ljsonlib, lregexlib, dan JNI bridge
├── include/            # Public C headers (lua.h, lauxlib.h, lualib.h, luaconf.h, cJSON.h)
├── kotlin/             # Kotlin Wrapper (com.github.anaruto.lua)
│   └── com/github/anaruto/lua/
│       ├── LuaEngine.kt
│       └── LuaException.kt
├── .github/workflows/  # GitHub Actions CI/CD (build.yml)
├── CMakeLists.txt      # Konfigurasi CMake build
├── Android.mk          # Konfigurasi ndk-build legacy
├── build.sh            # Skrip otomatisasi build lokal multi-ABI
└── Makefile            # Upstream Makefile Lua
```

---

## 🚀 Panduan Integrasi ke Aplikasi Android

### Cara 1: Menggunakan Prebuilt Binaries (Paling Mudah)
1. Unduh arsip `lua-android-5.5.1.zip` dari halaman [Releases](https://github.com/AhmadNaruto/lua-android/releases) atau artifact GitHub Actions.
2. Salin folder `jniLibs/` ke proyek Android Anda di:
   ```text
   app/src/main/jniLibs/
   ├── arm64-v8a/liblua.so
   ├── armeabi-v7a/liblua.so
   ├── x86/liblua.so
   └── x86_64/liblua.so
   ```
3. Salin file wrapper Kotlin ke source code aplikasi Anda:
   * Salin file dari `kotlin/com/github/anaruto/lua/` ke `app/src/main/java/com/github/anaruto/lua/`.

---

### Cara 2: Mengompilasi Langsung dari Source (Submodule / CMake)
Tambahkan ke dalam `app/build.gradle`:
```groovy
android {
    ...
    defaultConfig {
        ndk {
            abiFilters "armeabi-v7a", "arm64-v8a", "x86", "x86_64"
        }
    }
    externalNativeBuild {
        cmake {
            path file("path/to/lua-android/CMakeLists.txt")
        }
    }
}
```

---

## 💻 Contoh Penggunaan di Kotlin

```kotlin
import com.github.anaruto.lua.LuaEngine
import com.github.anaruto.lua.LuaException

fun main() {
    // Otomatis menutup state saat keluar dari blok .use
    LuaEngine.create().use { lua ->

        // 1. Eksekusi script Lua
        lua.doString("""
            function hitungDiskon(harga, persen)
                return harga - (harga * persen / 100)
            end
        """)

        // 2. Set variabel global dari Kotlin
        lua.setGlobal("namaBarang", "Laptop Gaming")

        // 3. Evaluasi ekspresi (JSON & Regex langsung aktif tanpa require!)
        val output = lua.eval("""
            local total = hitungDiskon(15000000, 10)
            local jsonResult = json.encode({
                barang = namaBarang,
                harga_akhir = total,
                apakah_laptop = regex.test(namaBarang, "Laptop")
            })
            return jsonResult
        """)

        println(output)
        // {"barang":"Laptop Gaming","harga_akhir":13500000,"apakah_laptop":true}
    }
}
```

### Menangani Error
```kotlin
try {
    lua.doString("sintaks salah :== 123")
} catch (e: LuaException) {
    println("Gagal mengeksekusi Lua: ${e.message}")
}
```

---

## 📜 Contoh API Lua bawaan

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

```lua
-- Shorthand PCRE (\d, \w, \s) didukung penuh:
local match, id, name = regex.match("User #452: Ahmad", "User #(\\d+): (\\w+)")
print(id, name) -- "452", "Ahmad"

-- Replace dengan backreference:
local iso = regex.replace("10/11/2026", "(\\d{2})/(\\d{2})/(\\d{4})", "$3-$2-$1")
print(iso)      -- "2026-11-10"

-- Split string:
local parts = regex.split("apel, jeruk; mangga", "[,;]\\s*")
print(parts[2]) -- "jeruk"
```

---

## 🔨 Build Lokal

Jika Anda ingin mengompilasi library native secara lokal di komputer pengembang:

### Persyaratan:
* Android NDK (r21 atau lebih baru)
* CMake (3.10 atau lebih baru)
* Linux / macOS / WSL

### Menjalankan Build:
```bash
export ANDROID_NDK_HOME="/path/to/android-sdk/ndk/25.1.8937393"
chmod +x build.sh
./build.sh
```
File biner hasil build dan file include akan dikumpulkan secara otomatis di folder `dist/`.

---

## 📄 Lisensi
Proyek ini dilisensikan di bawah [MIT License](LICENSE) (sesuai lisensi resmi Lua dan cJSON).
