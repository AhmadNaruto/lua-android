package com.github.anaruto.lua

/**
 * Base abstract class for callable Lua functions, adapting LuaJ's `LuaFunction`.
 */
abstract class LuaFunction : LuaValue() {

    override fun type(): Int = TFUNCTION
    override fun isfunction(): Boolean = true
    override fun checkfunction(): LuaFunction = this

    abstract override fun invoke(args: Varargs): Varargs
}

/**
 * Native bridge implementation of a Lua function / chunk residing inside the C Lua state.
 *
 * Keeps a registry reference in `LUA_REGISTRYINDEX` to ensure the function is retained until closed.
 */
class NativeLuaFunction internal constructor(
    internal val statePtr: Long,
    internal val ref: Int
) : LuaFunction() {

    override fun invoke(args: Varargs): Varargs {
        if (statePtr == 0L || ref == -1) {
            throw LuaError("Attempt to call closed or invalid native Lua function")
        }
        val results = Globals.nativeInvoke(statePtr, ref, args.toArray())
        return Varargs.of(*results)
    }

    override fun toString(): String {
        return "function: 0x" + Integer.toHexString(ref)
    }

    fun close() {
        if (statePtr != 0L && ref != -1) {
            Globals.nativeUnref(statePtr, ref)
        }
    }
}
