package com.ocarinamobile.legacy

import android.app.ActivityManager
import android.content.Context
import android.content.pm.PackageManager
import android.opengl.GLES20
import android.opengl.GLES30
import android.os.Build
import android.os.PowerManager
import android.util.DisplayMetrics
import android.view.WindowManager

/**
 * Coleta dados reais do aparelho. Nada aqui é estimativa: cada linha vem de uma API do Android
 * ou do driver. Serve para responder as perguntas em aberto da Fase 1 (GLES/Vulkan reais,
 * precisão de float, RAM, formatos de binário de programa).
 */
object HardwareProbe {

    @Suppress("DEPRECATION")
    fun systemReport(ctx: Context): String {
        val sb = StringBuilder()
        sb.appendLine("== Aparelho ==")
        sb.appendLine("fabricante=${Build.MANUFACTURER}")
        sb.appendLine("modelo=${Build.MODEL}")
        sb.appendLine("dispositivo=${Build.DEVICE}")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            sb.appendLine("soc=${Build.SOC_MANUFACTURER} ${Build.SOC_MODEL}")
        }
        sb.appendLine("android=${Build.VERSION.RELEASE} (SDK ${Build.VERSION.SDK_INT})")
        sb.appendLine("abis=${Build.SUPPORTED_ABIS.joinToString(",")}")
        sb.appendLine("nucleos_cpu=${Runtime.getRuntime().availableProcessors()}")

        sb.appendLine()
        sb.appendLine("== Memória ==")
        val am = ctx.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val mi = ActivityManager.MemoryInfo()
        am.getMemoryInfo(mi)
        sb.appendLine("ram_total_mb=${mi.totalMem / (1024 * 1024)}")
        sb.appendLine("ram_disponivel_mb=${mi.availMem / (1024 * 1024)}")
        sb.appendLine("limite_baixa_memoria_mb=${mi.threshold / (1024 * 1024)}")
        sb.appendLine("memoria_baixa=${mi.lowMemory}")
        sb.appendLine("memoryClass_mb=${am.memoryClass}")
        sb.appendLine("largeMemoryClass_mb=${am.largeMemoryClass}")
        sb.appendLine("heap_java_max_mb=${Runtime.getRuntime().maxMemory() / (1024 * 1024)}")

        sb.appendLine()
        sb.appendLine("== Vulkan (recursos declarados pelo sistema) ==")
        val vk = ctx.packageManager.systemAvailableFeatures
            .filter { it.name?.startsWith("android.hardware.vulkan") == true }
        if (vk.isEmpty()) {
            sb.appendLine("nenhum recurso android.hardware.vulkan.* declarado")
        } else {
            for (f in vk) sb.appendLine("${f.name} versao=0x${Integer.toHexString(f.version)}")
        }
        sb.appendLine("opengles_declarado=0x${Integer.toHexString(am.deviceConfigurationInfo.reqGlEsVersion)}")

        sb.appendLine()
        sb.appendLine("== Tela e energia ==")
        val wm = ctx.getSystemService(Context.WINDOW_SERVICE) as WindowManager
        val dm = DisplayMetrics()
        wm.defaultDisplay.getRealMetrics(dm)
        sb.appendLine("resolucao=${dm.widthPixels}x${dm.heightPixels} densidade=${dm.densityDpi}dpi")
        sb.appendLine("taxa_atualizacao_hz=${wm.defaultDisplay.refreshRate}")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            val pm = ctx.getSystemService(Context.POWER_SERVICE) as PowerManager
            sb.appendLine("estado_termico=${pm.currentThermalStatus} (0=nenhum, 1=leve, 2=moderado, 3=severo...)")
        }
        sb.appendLine("economia_de_energia=${(ctx.getSystemService(Context.POWER_SERVICE) as PowerManager).isPowerSaveMode}")
        return sb.toString()
    }

    /** Deve ser chamado na thread do GL, com o contexto ativo. */
    fun glReport(): String {
        val sb = StringBuilder()
        sb.appendLine("== OpenGL ES (driver real) ==")
        sb.appendLine("GL_VENDOR=${GLES20.glGetString(GLES20.GL_VENDOR)}")
        sb.appendLine("GL_RENDERER=${GLES20.glGetString(GLES20.GL_RENDERER)}")
        sb.appendLine("GL_VERSION=${GLES20.glGetString(GLES20.GL_VERSION)}")
        sb.appendLine("GLSL=${GLES20.glGetString(GLES20.GL_SHADING_LANGUAGE_VERSION)}")

        val tmp = IntArray(1)
        fun geti(name: String, p: Int) {
            GLES20.glGetIntegerv(p, tmp, 0)
            sb.appendLine("$name=${tmp[0]}")
        }
        geti("GL_MAX_TEXTURE_SIZE", GLES20.GL_MAX_TEXTURE_SIZE)
        geti("GL_MAX_FRAGMENT_UNIFORM_VECTORS", GLES20.GL_MAX_FRAGMENT_UNIFORM_VECTORS)
        geti("GL_MAX_SAMPLES (MSAA)", GLES30.GL_MAX_SAMPLES)
        geti("GL_NUM_PROGRAM_BINARY_FORMATS", GLES30.GL_NUM_PROGRAM_BINARY_FORMATS)

        // Precisão de float no fragment shader: responde a hipótese do `precision mediump float` do LUS.
        val range = IntArray(2)
        val prec = IntArray(1)
        GLES20.glGetShaderPrecisionFormat(GLES20.GL_FRAGMENT_SHADER, GLES20.GL_MEDIUM_FLOAT, range, 0, prec, 0)
        sb.appendLine("fragment_mediump_float: faixa=2^${range[0]}..2^${range[1]} bits_precisao=${prec[0]}")
        GLES20.glGetShaderPrecisionFormat(GLES20.GL_FRAGMENT_SHADER, GLES20.GL_HIGH_FLOAT, range, 0, prec, 0)
        sb.appendLine("fragment_highp_float:   faixa=2^${range[0]}..2^${range[1]} bits_precisao=${prec[0]}")

        val ext = (GLES20.glGetString(GLES20.GL_EXTENSIONS) ?: "").split(" ").filter { it.isNotBlank() }
        sb.appendLine("extensoes_total=${ext.size}")
        val interesse = listOf(
            "GL_EXT_shader_framebuffer_fetch", "GL_ARM_shader_framebuffer_fetch",
            "GL_EXT_texture_filter_anisotropic", "GL_KHR_texture_compression_astc_ldr",
            "GL_EXT_buffer_storage", "GL_EXT_multisampled_render_to_texture",
            "GL_OES_get_program_binary", "GL_KHR_debug",
        )
        for (e in interesse) sb.appendLine("$e=${if (ext.contains(e)) "sim" else "nao"}")

        sb.appendLine()
        sb.appendLine(shaderCompileBenchmark())
        return sb.toString()
    }

    /**
     * Mede o tempo de compilar e ligar 20 variantes de um fragment shader parecido com um
     * "combiner" do Fast3D. NÃO é o shader real do jogo: é uma medida de ordem de grandeza
     * para a hipótese de engasgos por compilação de shader.
     */
    private fun shaderCompileBenchmark(): String {
        val vs = """#version 300 es
in vec4 aPos; in vec2 aUv; in vec4 aColor;
out vec2 vUv; out vec4 vColor;
void main() { gl_Position = aPos; vUv = aUv; vColor = aColor; }"""
        val fsTemplate = """#version 300 es
precision mediump float;
uniform sampler2D uTex0; uniform sampler2D uTex1;
in vec2 vUv; in vec4 vColor; out vec4 outColor;
void main() {
  vec4 t0 = texture(uTex0, vUv);
  vec4 t1 = texture(uTex1, vUv * VARIANT.0);
  vec4 c = (t0 - vec4(0.5)) * vColor + t1;
  c.rgb = mix(c.rgb, vColor.rgb, fract(float(VARIANT) * 0.37));
  outColor = c;
  if (outColor.a < 0.1) discard;
}"""
        val times = ArrayList<Double>()
        var failures = 0
        for (i in 1..20) {
            val t0 = System.nanoTime()
            val v = compile(GLES20.GL_VERTEX_SHADER, vs)
            val f = compile(GLES20.GL_FRAGMENT_SHADER, fsTemplate.replace("VARIANT", i.toString()))
            val prog = GLES20.glCreateProgram()
            GLES20.glAttachShader(prog, v)
            GLES20.glAttachShader(prog, f)
            GLES20.glLinkProgram(prog)
            val ok = IntArray(1)
            GLES20.glGetProgramiv(prog, GLES20.GL_LINK_STATUS, ok, 0)
            if (ok[0] == 0) failures++ else GLES20.glUseProgram(prog)
            GLES20.glFinish()
            times.add((System.nanoTime() - t0) / 1e6)
            GLES20.glUseProgram(0)
            GLES20.glDeleteProgram(prog)
            GLES20.glDeleteShader(v)
            GLES20.glDeleteShader(f)
        }
        val first = times.first()
        val rest = times.drop(1)
        return buildString {
            appendLine("== Benchmark de compilação de shader (simplificado) ==")
            appendLine("variantes=20 falhas=$failures")
            appendLine("primeira_ms=${"%.2f".format(first)}")
            appendLine("media_demais_ms=${"%.2f".format(rest.average())}")
            appendLine("maxima_ms=${"%.2f".format(times.max())}")
        }
    }

    private fun compile(type: Int, src: String): Int {
        val s = GLES20.glCreateShader(type)
        GLES20.glShaderSource(s, src)
        GLES20.glCompileShader(s)
        return s
    }
}
