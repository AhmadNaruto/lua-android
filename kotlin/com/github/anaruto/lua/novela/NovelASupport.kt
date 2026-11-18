package com.github.anaruto.lua.novela

import com.github.anaruto.lua.*

/**
 * High-performance adapter bridging `lua-android` with the NoveLA extension specification.
 *
 * Provides drop-in support for:
 * - HTTP client bridging (`http_get`, `http_post`, `http_get_batch`)
 * - Response chaining (`response:toHtml()`, `response:to_html()`)
 * - Cookie management (`get_cookies`, `set_cookies`)
 * - System error dialogues (`show_error(title, msg, authUrl)`)
 * - Logging (`log_info`, `log_error`)
 * - Time and pause primitives (`os_time`, `sleep`)
 */
object NovelASupport {

    data class HttpResponse(
        val success: Boolean,
        val code: Int,
        val body: String,
        val headers: Map<String, String> = emptyMap()
    )

    fun interface HttpHandler {
        fun request(method: String, url: String, body: String?, config: LuaTable?): HttpResponse
    }

    fun interface CookieHandler {
        fun getCookies(url: String): String
        fun setCookies(url: String, cookies: String) {}
    }

    fun interface ErrorHandler {
        fun showError(title: String, message: String, authUrl: String?)
    }

    interface LogHandler {
        fun info(tag: String, message: String)
        fun error(tag: String, message: String)
    }

    private val defaultLogHandler = object : LogHandler {
        override fun info(tag: String, message: String) {
            println("[$tag] $message")
        }

        override fun error(tag: String, message: String) {
            System.err.println("[$tag] ERROR: $message")
        }
    }

    /**
     * Builds a NoveLA-compliant HTTP response table with chaining capabilities (`toHtml`).
     */
    fun createResponseTable(globals: Globals, res: HttpResponse, requestUrl: String? = null): LuaTable {
        val table = globals.newTable()
        table.set("success", LuaValue.valueOf(res.success))
        table.set("code", LuaValue.valueOf(res.code))
        table.set("body", LuaValue.valueOf(res.body))

        val headersTable = globals.newTable()
        for ((k, v) in res.headers) {
            headersTable.set(k, LuaValue.valueOf(v))
        }
        table.set("headers", headersTable)

        // Method chaining: response:toHtml() / response:to_html()
        val toHtmlFunc = object : LuaFunction() {
            override fun call(arg: LuaValue): LuaValue {
                val parseFunc = globals.get("html_parse")
                val bodyVal = table.get("body")
                return if (requestUrl != null) {
                    parseFunc.call(bodyVal, LuaValue.valueOf(requestUrl))
                } else {
                    parseFunc.call(bodyVal)
                }
            }
        }
        table.set("toHtml", toHtmlFunc)
        table.set("to_html", toHtmlFunc)

        return table
    }

    /**
     * Installs NoveLA runtime globals into the given [Globals] instance.
     */
    @JvmStatic
    @JvmOverloads
    fun install(
        globals: Globals,
        httpHandler: HttpHandler? = null,
        cookieHandler: CookieHandler? = null,
        errorHandler: ErrorHandler? = null,
        logHandler: LogHandler = defaultLogHandler
    ) {
        // --- HTTP GET ---
        globals.set("http_get", object : LuaFunction() {
            override fun call(arg: LuaValue): LuaValue = call(arg, LuaValue.NIL)

            override fun call(arg1: LuaValue, arg2: LuaValue): LuaValue {
                val url = arg1.checkjstring()
                val config = if (arg2.istable()) arg2.checktable() else null
                if (httpHandler == null) {
                    throw LuaError("http_get called but no HttpHandler is registered")
                }
                val res = httpHandler.request("GET", url, null, config)
                return createResponseTable(globals, res, url)
            }
        })

        // --- HTTP POST ---
        globals.set("http_post", object : LuaFunction() {
            override fun call(arg1: LuaValue, arg2: LuaValue): LuaValue {
                return call(arg1, arg2, LuaValue.NIL)
            }

            override fun invoke(args: Varargs): Varargs {
                val url = args.checkjstring(1)
                val body = if (args.narg() >= 2 && !args.arg(2).isnil()) args.arg(2).tojstring() else ""
                val config = if (args.narg() >= 3 && args.arg(3).istable()) args.arg(3).checktable() else null
                if (httpHandler == null) {
                    throw LuaError("http_post called but no HttpHandler is registered")
                }
                val res = httpHandler.request("POST", url, body, config)
                return createResponseTable(globals, res, url)
            }
        })

        // --- HTTP GET BATCH ---
        globals.set("http_get_batch", object : LuaFunction() {
            override fun invoke(args: Varargs): Varargs {
                val items = args.checktable(1)
                val config = if (args.narg() >= 2 && args.arg(2).istable()) args.arg(2).checktable() else null
                if (httpHandler == null) {
                    throw LuaError("http_get_batch called but no HttpHandler is registered")
                }
                val resultTable = globals.newTable()
                val len = items.length()
                for (i in 1..len) {
                    val urlVal = items.get(i)
                    val url = urlVal.checkjstring()
                    val res = httpHandler.request("GET", url, null, config)
                    resultTable.set(i, createResponseTable(globals, res, url))
                }
                return resultTable
            }
        })

        // --- COOKIES ---
        globals.set("get_cookies", object : LuaFunction() {
            override fun call(arg: LuaValue): LuaValue {
                val url = arg.checkjstring()
                val cookies = cookieHandler?.getCookies(url) ?: ""
                return LuaValue.valueOf(cookies)
            }
        })

        globals.set("set_cookies", object : LuaFunction() {
            override fun call(arg1: LuaValue, arg2: LuaValue): LuaValue {
                val url = arg1.checkjstring()
                val cookies = arg2.checkjstring()
                cookieHandler?.setCookies(url, cookies)
                return LuaValue.NIL
            }
        })

        // --- LOGGING ---
        globals.set("log_info", object : LuaFunction() {
            override fun call(arg: LuaValue): LuaValue {
                logHandler.info("NoveLA", arg.tojstring())
                return LuaValue.NIL
            }
        })

        globals.set("log_error", object : LuaFunction() {
            override fun call(arg: LuaValue): LuaValue {
                logHandler.error("NoveLA", arg.tojstring())
                return LuaValue.NIL
            }
        })

        // --- ERROR & AUTH UI ---
        globals.set("show_error", object : LuaFunction() {
            override fun invoke(args: Varargs): Varargs {
                val title = args.checkjstring(1)
                val msg = if (args.narg() >= 2) args.arg(2).tojstring() else ""
                val authUrl = if (args.narg() >= 3 && !args.arg(3).isnil()) args.arg(3).tojstring() else null
                errorHandler?.showError(title, msg, authUrl)
                return LuaValue.NIL
            }
        })

        // --- TIME & SLEEP ---
        globals.set("os_time", object : LuaFunction() {
            override fun call(): LuaValue {
                return LuaValue.valueOf(System.currentTimeMillis().toDouble())
            }
        })

        globals.set("sleep", object : LuaFunction() {
            override fun call(arg: LuaValue): LuaValue {
                val ms = arg.tolong()
                if (ms > 0) {
                    try {
                        Thread.sleep(ms)
                    } catch (e: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
                return LuaValue.NIL
            }
        })
    }
}
