package com.github.anaruto.lua

import java.io.Closeable

/**
 * Global environment and Lua execution context, adapting LuaJ's `org.luaj.vm2.Globals`.
 *
 * Implements [Closeable] to support Kotlin's `.use { ... }` idiom.
 *
 * Example LuaJ-style usage:
 * ```kotlin
 * val globals = JsePlatform.standardGlobals()
 *
 * // Compile and run a script chunk
 * val chunk = globals.load("return 'Hello from ' .. _VERSION")
 * println(chunk.call().tojstring())
 *
 * // Access and invoke globals
 * val math = globals.get("math")
 * val sqrt = math.get("sqrt")
 * val result = sqrt.call(LuaValue.valueOf(144))
 * println(result.todouble()) // 12.0
 *
 * // Use built-in JSON without require
 * val json = globals.get("json")
 * val encode = json.get("encode")
 * val jsonStr = encode.call(LuaValue.tableOf("name" to "Android", "code" to 100))
 * println(jsonStr.tojstring())
 *
 * globals.close()
 * ```
 */
class Globals private constructor(internal var statePtr: Long) : LuaTable(), Closeable {

    companion object {
        init {
            System.loadLibrary("lua")
        }

        /**
         * Creates a new Globals instance with standard libraries, JSON, and Regex loaded.
         */
        @JvmStatic
        fun create(): Globals {
            val ptr = nativeNewState()
            if (ptr == 0L) {
                throw LuaError("Failed to allocate native Lua state")
            }
            return Globals(ptr)
        }

        // --- Native Bindings ---

        @JvmStatic
        internal external fun nativeNewState(): Long

        @JvmStatic
        internal external fun nativeClose(ptr: Long)

        @JvmStatic
        internal external fun nativeLoad(ptr: Long, script: String, chunkname: String): Int

        @JvmStatic
        internal external fun nativeLoadFile(ptr: Long, filePath: String): Int

        @JvmStatic
        internal external fun nativeInvoke(ptr: Long, funcRef: Int, args: Array<LuaValue>): Array<LuaValue>

        @JvmStatic
        internal external fun nativeUnref(ptr: Long, ref: Int)

        @JvmStatic
        internal external fun nativeGetGlobal(ptr: Long, name: String): LuaValue

        @JvmStatic
        internal external fun nativeSetGlobal(ptr: Long, name: String, value: LuaValue)

        @JvmStatic
        internal external fun nativeTableGet(ptr: Long, tableRef: Int, key: LuaValue): LuaValue

        @JvmStatic
        internal external fun nativeTableSet(ptr: Long, tableRef: Int, key: LuaValue, value: LuaValue)

        @JvmStatic
        internal external fun nativeTableLength(ptr: Long, tableRef: Int): Int

        @JvmStatic
        internal external fun nativeTableKeys(ptr: Long, tableRef: Int): Array<LuaValue>

        @JvmStatic
        internal external fun nativeNewTable(ptr: Long): Int

        @JvmStatic
        internal external fun nativeDoString(ptr: Long, script: String): Array<LuaValue>?

        @JvmStatic
        internal external fun nativeDoFile(ptr: Long, filePath: String): Array<LuaValue>?
    }

    val isClosed: Boolean
        get() = statePtr == 0L

    private fun checkState() {
        if (statePtr == 0L) {
            throw LuaError("Globals state has already been closed")
        }
    }

    /**
     * Compiles a Lua script chunk into an executable [LuaFunction].
     *
     * @param script Lua source code string.
     * @param chunkname Identifying name for error reporting (default: "chunk").
     * @return [LuaFunction] chunk ready to be called via `.call()` or `.invoke()`.
     */
    fun load(script: String, chunkname: String = "chunk"): LuaFunction {
        checkState()
        val ref = nativeLoad(statePtr, script, chunkname)
        if (ref == -1) {
            throw LuaError("Failed to compile script chunk: $chunkname")
        }
        return NativeLuaFunction(statePtr, ref)
    }

    /**
     * Compiles a Lua script file from the given path into an executable [LuaFunction].
     */
    fun loadfile(filePath: String): LuaFunction {
        checkState()
        val ref = nativeLoadFile(statePtr, filePath)
        if (ref == -1) {
            throw LuaError("Failed to load script file: $filePath")
        }
        return NativeLuaFunction(statePtr, ref)
    }

    /**
     * Executes the script chunk immediately and returns all return values as [Varargs].
     */
    fun doString(script: String): Varargs {
        checkState()
        val results = nativeDoString(statePtr, script)
            ?: throw LuaError("Failed to execute Lua script")
        return Varargs.of(*results)
    }

    /**
     * Executes the script file immediately and returns all return values as [Varargs].
     */
    fun doFile(filePath: String): Varargs {
        checkState()
        val results = nativeDoFile(statePtr, filePath)
            ?: throw LuaError("Failed to execute Lua script file: $filePath")
        return Varargs.of(*results)
    }

    /**
     * Evaluates a single Lua expression (e.g. `1 + 2` or `json.encode(data)`) and returns the resulting [LuaValue].
     */
    fun eval(expression: String): LuaValue {
        val chunk = load("return $expression", "eval")
        return chunk.call()
    }

    /**
     * Creates a new native Lua table inside the Lua state.
     */
    fun newTable(): LuaTable {
        checkState()
        val ref = nativeNewTable(statePtr)
        return NativeLuaTable(statePtr, ref)
    }

    // --- Global Variable Access (LuaJ Globals as environment table) ---

    override fun get(key: LuaValue): LuaValue {
        checkState()
        return if (key.isstring()) {
            nativeGetGlobal(statePtr, key.tojstring())
        } else {
            NIL
        }
    }

    override fun get(key: String): LuaValue {
        checkState()
        return nativeGetGlobal(statePtr, key)
    }

    override fun set(key: LuaValue, value: LuaValue) {
        checkState()
        if (key.isstring()) {
            nativeSetGlobal(statePtr, key.tojstring(), value)
        } else {
            throw LuaError("Global variable keys must be strings, got " + key.typename())
        }
    }

    override fun set(key: String, value: LuaValue) {
        checkState()
        nativeSetGlobal(statePtr, key, value)
    }

    override fun close() {
        if (statePtr != 0L) {
            nativeClose(statePtr)
            statePtr = 0L
        }
    }

    /**
     * Compiles a Lua script directly from a Java [java.io.InputStream].
     */
    fun load(inputStream: java.io.InputStream, chunkname: String = "stream"): LuaFunction {
        val script = inputStream.bufferedReader(Charsets.UTF_8).use { it.readText() }
        return load(script, chunkname)
    }

    /**
     * Loads a script file directly from Android's assets folder via reflection,
     * maintaining compatibility with both Android and standard JVM runtimes.
     *
     * @param assetManager An instance of `android.content.res.AssetManager`.
     * @param assetPath Path within the `assets/` directory (e.g. "scripts/main.lua").
     */
    fun loadAsset(assetManager: Any, assetPath: String): LuaFunction {
        val method = assetManager.javaClass.getMethod("open", String::class.java)
        val stream = method.invoke(assetManager, assetPath) as java.io.InputStream
        return load(stream, assetPath)
    }

    /**
     * Installs a custom loader into Lua's `package.searchers` so that calls like `require("my.module")`
     * will automatically resolve and execute `assets/{basePath}/my/module.lua`.
     *
     * @param assetManager An instance of `android.content.res.AssetManager`.
     * @param basePath Base subdirectory inside `assets/` (default: "lua").
     */
    fun installAssetSearcher(assetManager: Any, basePath: String = "lua") {
        val packageTable = get("package")
        val searchers = packageTable.get("searchers")
        if (searchers.istable()) {
            val searcherFunc = object : OneArgFunction() {
                override fun call(arg: LuaValue): LuaValue {
                    val modName = arg.checkjstring()
                    val relativePath = (if (basePath.isEmpty()) "" else "$basePath/") +
                        modName.replace('.', '/') + ".lua"
                    return try {
                        val method = assetManager.javaClass.getMethod("open", String::class.java)
                        val stream = method.invoke(assetManager, relativePath) as java.io.InputStream
                        val script = stream.bufferedReader(Charsets.UTF_8).use { it.readText() }
                        load(script, relativePath)
                    } catch (e: Exception) {
                        LuaValue.valueOf("\n\tno asset '$relativePath'")
                    }
                }
            }
            val len = searchers.length()
            searchers.set(len + 1, searcherFunc)
        }
    }
}
