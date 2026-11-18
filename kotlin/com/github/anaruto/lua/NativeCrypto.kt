package com.github.anaruto.lua

import java.net.URLEncoder
import java.nio.charset.StandardCharsets
import java.util.Base64
import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

/**
 * Built-in cryptographic and encoding helpers for Lua runtime.
 */
object NativeCrypto {

    @JvmStatic
    fun base64Encode(data: ByteArray): String {
        return try {
            // Android android.util.Base64 or standard java.util.Base64 fallback
            val androidBase64Class = runCatching { Class.forName("android.util.Base64") }.getOrNull()
            if (androidBase64Class != null) {
                val encodeMethod = androidBase64Class.getMethod("encodeToString", ByteArray::class.java, Int::class.javaPrimitiveType)
                encodeMethod.invoke(null, data, 2) as String // 2 = NO_WRAP
            } else {
                Base64.getEncoder().encodeToString(data)
            }
        } catch (e: Exception) {
            Base64.getEncoder().encodeToString(data)
        }
    }

    @JvmStatic
    fun base64Decode(encoded: String): ByteArray? {
        return try {
            val androidBase64Class = runCatching { Class.forName("android.util.Base64") }.getOrNull()
            if (androidBase64Class != null) {
                val decodeMethod = androidBase64Class.getMethod("decode", String::class.java, Int::class.javaPrimitiveType)
                decodeMethod.invoke(null, encoded, 0) as ByteArray // 0 = DEFAULT
            } else {
                Base64.getDecoder().decode(encoded.trim())
            }
        } catch (e: Exception) {
            null
        }
    }

    @JvmStatic
    fun urlEncode(text: String, charset: String = "UTF-8"): String {
        return try {
            URLEncoder.encode(text, charset)
        } catch (e: Exception) {
            text
        }
    }

    @JvmStatic
    fun aesDecrypt(cipherBase64: String, key: ByteArray, iv: ByteArray): ByteArray? {
        return try {
            val decodedCipher = base64Decode(cipherBase64) ?: return null
            val cipher = Cipher.getInstance("AES/CBC/PKCS5Padding")
            cipher.init(
                Cipher.DECRYPT_MODE,
                SecretKeySpec(key, "AES"),
                IvParameterSpec(iv)
            )
            cipher.doFinal(decodedCipher)
        } catch (e: Exception) {
            null
        }
    }
}
