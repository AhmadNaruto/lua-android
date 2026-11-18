package com.github.anaruto.lua.sample

import android.os.Bundle
import android.widget.ScrollView
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.github.anaruto.lua.JsePlatform
import com.github.anaruto.lua.LuaValue
import com.github.anaruto.lua.register
import com.github.anaruto.lua.sample.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.InputStreamReader

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var passedCounter = 0
    private var failedCounter = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.btnRun.setOnClickListener {
            runTestSuite()
        }

        binding.btnClear.setOnClickListener {
            clearLog()
        }
    }

    private fun clearLog() {
        binding.tvLog.text = ""
        passedCounter = 0
        failedCounter = 0
        updateCounters()
        binding.tvStatus.text = getString(R.string.status_ready)
    }

    private fun updateCounters() {
        binding.tvPassedCount.text = "Passed: $passedCounter"
        binding.tvFailedCount.text = "Failed: $failedCounter"
    }

    private fun appendLog(line: String) {
        if (line.contains("PASS:")) {
            passedCounter++
        } else if (line.contains("FAIL:")) {
            failedCounter++
        }

        binding.tvLog.append("$line\n")
        updateCounters()

        // Auto-scroll to bottom
        binding.scrollView.post {
            binding.scrollView.fullScroll(ScrollView.FOCUS_DOWN)
        }
    }

    private fun runTestSuite() {
        binding.btnRun.isEnabled = false
        binding.btnClear.isEnabled = false
        binding.tvStatus.text = getString(R.string.status_running)
        clearLog()

        lifecycleScope.launch(Dispatchers.Default) {
            val startTime = System.currentTimeMillis()

            try {
                // 1. Read test_suite.lua from assets
                val script = assets.open("test_suite.lua").use { inputStream ->
                    BufferedReader(InputStreamReader(inputStream)).readText()
                }

                // 2. Initialize Lua environment (adapting LuaJ's JsePlatform)
                val globals = JsePlatform.standardGlobals()

                // 3. Hook print() to stream logs to Android UI in real-time
                globals.register("print") { args ->
                    val sb = StringBuilder()
                    for (i in 1..args.narg()) {
                        if (i > 1) sb.append("\t")
                        sb.append(args.arg(i).tojstring())
                    }
                    val line = sb.toString()

                    runOnUiThread {
                        appendLog(line)
                    }
                    LuaValue.NIL
                }

                // 4. Compile and execute test suite
                val chunk = globals.load(script, "test_suite.lua")
                chunk.call()

                globals.close()

                val duration = System.currentTimeMillis() - startTime
                withContext(Dispatchers.Main) {
                    binding.tvStatus.text = "Selesai dalam ${duration}ms! ($passedCounter Passed, $failedCounter Failed)"
                }
            } catch (e: Throwable) {
                withContext(Dispatchers.Main) {
                    appendLog("FATAL ERROR: ${e.message}")
                    binding.tvStatus.text = "Terjadi Kesalahan saat eksekusi!"
                }
            } finally {
                withContext(Dispatchers.Main) {
                    binding.btnRun.isEnabled = true
                    binding.btnClear.isEnabled = true
                }
            }
        }
    }
}
