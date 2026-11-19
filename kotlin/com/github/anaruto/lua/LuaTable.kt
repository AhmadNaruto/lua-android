package com.github.anaruto.lua

import java.util.Collections
import java.util.LinkedHashMap

/**
 * Represents a Lua table, adapting LuaJ's `LuaTable`.
 *
 * Can function both as a standalone in-memory table and as a bridge to native Lua tables.
 */
open class LuaTable : LuaValue() {

    private val map: MutableMap<LuaValue, LuaValue> = LinkedHashMap()
    private var metatable: LuaValue = NIL

    override fun type(): Int = TTABLE
    override fun istable(): Boolean = true
    override fun checktable(): LuaTable = this

    override fun getmetatable(): LuaValue = metatable

    override fun setmetatable(metatable: LuaValue): LuaValue {
        this.metatable = metatable
        return this
    }

    override operator fun get(key: LuaValue): LuaValue {
        val direct = map[key]
        if (direct != null && !direct.isnil()) {
            return direct
        }
        if (!metatable.isnil()) {
            val indexHandler = metatable.get(INDEX)
            if (indexHandler.istable()) {
                return indexHandler.get(key)
            } else if (indexHandler.isfunction()) {
                return indexHandler.call(this, key)
            }
        }
        return NIL
    }

    override operator fun set(key: LuaValue, value: LuaValue) {
        if (value.isnil()) {
            map.remove(key)
        } else {
            map[key] = value
        }
    }

    override fun rawget(key: LuaValue): LuaValue = map[key] ?: NIL
    override fun rawget(key: String): LuaValue = rawget(valueOf(key))
    override fun rawget(key: Int): LuaValue = rawget(valueOf(key))

    override fun rawset(key: LuaValue, value: LuaValue): LuaTable {
        if (value.isnil()) {
            map.remove(key)
        } else {
            map[key] = value
        }
        return this
    }
    override fun rawset(key: String, value: LuaValue): LuaTable = rawset(valueOf(key), value)
    override fun rawset(key: Int, value: LuaValue): LuaTable = rawset(valueOf(key), value)

    override fun length(): Int {
        var len = 0
        while (get(valueOf(len + 1)) !is LuaNil) {
            len++
        }
        return len
    }

    open fun keys(): List<LuaValue> {
        return ArrayList(map.keys)
    }

    open fun toMap(): Map<LuaValue, LuaValue> {
        return Collections.unmodifiableMap(map)
    }

    override fun tojstring(): String = toString()

    override fun toString(): String {
        return map.entries.joinToString(prefix = "{", postfix = "}") { (k, v) ->
            "${k.tojstring()}=${v.tojstring()}"
        }
    }
}

/**
 * Native bridge implementation of a Lua table residing inside the C Lua state.
 *
 * Keeps a registry reference in `LUA_REGISTRYINDEX` to ensure the table is not garbage-collected.
 */
class NativeLuaTable internal constructor(
    internal val statePtr: Long,
    internal val ref: Int
) : LuaTable() {

    private val closed = java.util.concurrent.atomic.AtomicBoolean(false)

    override operator fun get(key: LuaValue): LuaValue {
        if (statePtr == 0L || ref == -1 || closed.get()) return NIL
        return Globals.nativeTableGet(statePtr, ref, key)
    }

    override operator fun set(key: LuaValue, value: LuaValue) {
        if (statePtr == 0L || ref == -1 || closed.get()) throw LuaError("Native table is closed or invalid")
        Globals.nativeTableSet(statePtr, ref, key, value)
    }

    override fun rawget(key: LuaValue): LuaValue = get(key)

    override fun rawset(key: LuaValue, value: LuaValue): LuaTable {
        set(key, value)
        return this
    }

    override fun length(): Int {
        if (statePtr == 0L || ref == -1 || closed.get()) return 0
        return Globals.nativeTableLength(statePtr, ref)
    }

    override fun keys(): List<LuaValue> {
        if (statePtr == 0L || ref == -1 || closed.get()) return emptyList()
        return Globals.nativeTableKeys(statePtr, ref).toList()
    }

    override fun toMap(): Map<LuaValue, LuaValue> {
        val result = LinkedHashMap<LuaValue, LuaValue>()
        for (k in keys()) {
            result[k] = get(k)
        }
        return result
    }

    override fun toString(): String {
        return "table: 0x" + Integer.toHexString(ref)
    }

    fun close() {
        if (closed.compareAndSet(false, true)) {
            if (statePtr != 0L && ref != -1) {
                Globals.nativeUnref(statePtr, ref)
            }
        }
    }
}
