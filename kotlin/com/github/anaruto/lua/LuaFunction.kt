package com.github.anaruto.lua

/**
 * Base abstract class for callable Lua functions, adapting LuaJ's `LuaFunction`.
 */
abstract class LuaFunction : LuaValue() {

    override fun type(): Int = TFUNCTION
    override fun isfunction(): Boolean = true
    override fun checkfunction(): LuaFunction = this

    abstract override operator fun invoke(args: Varargs): Varargs
}

/**
 * Convenience base class for functions accepting variable arguments, adapting LuaJ's `VarArgFunction`.
 */
abstract class VarArgFunction : LuaFunction()

/**
 * Convenience base class for functions taking no arguments, adapting LuaJ's `ZeroArgFunction`.
 */
abstract class ZeroArgFunction : LuaFunction() {
    abstract override fun call(): LuaValue
    override operator fun invoke(args: Varargs): Varargs = Varargs.of(call())
}

/**
 * Convenience base class for functions taking exactly one argument, adapting LuaJ's `OneArgFunction`.
 */
abstract class OneArgFunction : LuaFunction() {
    abstract override fun call(arg1: LuaValue): LuaValue
    override operator fun invoke(args: Varargs): Varargs = Varargs.of(call(args.arg1()))
}

/**
 * Convenience base class for functions taking two arguments, adapting LuaJ's `TwoArgFunction`.
 */
abstract class TwoArgFunction : LuaFunction() {
    abstract override fun call(arg1: LuaValue, arg2: LuaValue): LuaValue
    override operator fun invoke(args: Varargs): Varargs = Varargs.of(call(args.arg(1), args.arg(2)))
}

/**
 * Convenience base class for functions taking three arguments, adapting LuaJ's `ThreeArgFunction`.
 */
abstract class ThreeArgFunction : LuaFunction() {
    abstract override fun call(arg1: LuaValue, arg2: LuaValue, arg3: LuaValue): LuaValue
    override operator fun invoke(args: Varargs): Varargs = Varargs.of(call(args.arg(1), args.arg(2), args.arg(3)))
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

    private val closed = java.util.concurrent.atomic.AtomicBoolean(false)

    override operator fun invoke(args: Varargs): Varargs {
        if (statePtr == 0L || ref == -1 || closed.get()) {
            throw LuaError("Attempt to call closed or invalid native Lua function")
        }
        val results = Globals.nativeInvoke(statePtr, ref, args.toArray())
        return Varargs.of(*results)
    }

    override fun toString(): String {
        return "function: 0x" + Integer.toHexString(ref)
    }

    fun close() {
        if (closed.compareAndSet(false, true)) {
            if (statePtr != 0L && ref != -1) {
                Globals.nativeUnref(statePtr, ref)
            }
        }
    }
}

/**
 * Registers a Kotlin lambda as a global function accessible within Lua scripts.
 *
 * Example:
 * ```kotlin
 * globals.register("toast") { args ->
 *     val message = args.arg1().tojstring()
 *     println("Toast: $message")
 *     LuaValue.TRUE
 * }
 * ```
 */
fun Globals.register(name: String, action: (Varargs) -> LuaValue) {
    set(name, object : VarArgFunction() {
        override operator fun invoke(args: Varargs): Varargs = Varargs.of(action(args))
    })
}

/**
 * Registers a Kotlin lambda as a function field within this [LuaTable].
 *
 * Example:
 * ```kotlin
 * val myModule = globals.newTable()
 * myModule.register("calculate") { args ->
 *     val x = args.arg(1).todouble()
 *     val y = args.arg(2).todouble()
 *     LuaValue.valueOf(x + y)
 * }
 * globals.set("myModule", myModule)
 * ```
 */
fun LuaTable.register(name: String, action: (Varargs) -> LuaValue) {
    set(name, object : VarArgFunction() {
        override operator fun invoke(args: Varargs): Varargs = Varargs.of(action(args))
    })
}
