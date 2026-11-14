package com.ocarinamobile.legacy

import android.app.Activity
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.graphics.Typeface
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import java.io.File
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * Fase 2 — Protótipo 1: sonda de hardware + verificador de ROM.
 * Este app AINDA NÃO executa Ocarina of Time. Ele só mede o aparelho e confere a ROM.
 */
class MainActivity : Activity() {

    private lateinit var output: TextView
    private val sections = linkedMapOf<String, String>()
    private var glView: GLSurfaceView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val pad = (16 * resources.displayMetrics.density).toInt()

        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(pad, pad, pad, pad)
        }

        root.addView(TextView(this).apply {
            text = "Ocarina Mobile Legacy — Fase 2 (sonda)\nEste app ainda não executa o jogo."
            textSize = 16f
            setTypeface(typeface, Typeface.BOLD)
        })

        val buttons = LinearLayout(this).apply { orientation = LinearLayout.HORIZONTAL }
        buttons.addView(Button(this).apply {
            text = "Escolher ROM"
            setOnClickListener { pickRom() }
        })
        buttons.addView(Button(this).apply {
            text = "Copiar"
            setOnClickListener { copyReport() }
        })
        buttons.addView(Button(this).apply {
            text = "Salvar"
            setOnClickListener { saveReport() }
        })
        root.addView(buttons)

        // Superfície pequena só para existir um contexto OpenGL ES 3 real.
        glView = GLSurfaceView(this).apply {
            setEGLContextClientVersion(3)
            setRenderer(object : GLSurfaceView.Renderer {
                private var done = false
                override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
                    if (done) return
                    done = true
                    val text = try {
                        HardwareProbe.glReport()
                    } catch (t: Throwable) {
                        "== OpenGL ES ==\nERRO na sonda: ${t.javaClass.simpleName}: ${t.message}\n"
                    }
                    runOnUiThread { setSection("gl", text) }
                }
                override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {}
                override fun onDrawFrame(gl: GL10?) {}
            })
            renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        }
        root.addView(glView, LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, (24 * resources.displayMetrics.density).toInt()))

        output = TextView(this).apply {
            typeface = Typeface.MONOSPACE
            textSize = 11f
            setTextIsSelectable(true)
        }
        root.addView(ScrollView(this).apply { addView(output) },
            LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0, 1f))
        setContentView(root)

        setSection("sistema", HardwareProbe.systemReport(this))
        setSection("nativo", "== Biblioteca nativa ==\n" + try {
            NativeBridge.nativeInfo()
        } catch (t: Throwable) {
            "ERRO ao carregar libs nativas: ${t.javaClass.simpleName}: ${t.message}\n"
        })
        setSection("rom", "== ROM ==\nnenhuma ROM verificada ainda\n")
    }

    override fun onResume() { super.onResume(); glView?.onResume() }
    override fun onPause() { glView?.onPause(); super.onPause() }

    private fun pickRom() {
        val i = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
        }
        @Suppress("DEPRECATION")
        startActivityForResult(i, REQ_ROM)
    }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != REQ_ROM || resultCode != RESULT_OK) return
        val uri = data?.data ?: return
        setSection("rom", "== ROM ==\nverificando...\n")
        Thread {
            val text = try {
                val pfd = contentResolver.openFileDescriptor(uri, "r")
                    ?: throw IllegalStateException("não foi possível abrir o arquivo")
                val fd = pfd.detachFd()  // o código nativo lê e fecha o descritor
                "== ROM ==\n" + NativeBridge.verifyRomFd(fd)
            } catch (t: Throwable) {
                "== ROM ==\nERRO: ${t.javaClass.simpleName}: ${t.message}\n"
            }
            runOnUiThread { setSection("rom", text) }
        }.start()
    }

    private fun setSection(key: String, text: String) {
        sections[key] = text
        output.text = report()
    }

    private fun report(): String = sections.values.joinToString("\n")

    private fun copyReport() {
        val cm = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        cm.setPrimaryClip(ClipData.newPlainText("relatorio", report()))
        Toast.makeText(this, "Relatório copiado", Toast.LENGTH_SHORT).show()
    }

    private fun saveReport() {
        val dir = getExternalFilesDir(null) ?: filesDir
        val f = File(dir, "relatorio_sonda.txt")
        try {
            f.writeText(report())
            Toast.makeText(this, "Salvo em ${f.absolutePath}", Toast.LENGTH_LONG).show()
        } catch (t: Throwable) {
            Toast.makeText(this, "Falha ao salvar: ${t.message}", Toast.LENGTH_LONG).show()
        }
    }

    companion object {
        private const val REQ_ROM = 1001
    }
}
