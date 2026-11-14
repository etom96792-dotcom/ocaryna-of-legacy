package com.ocarinamobile.legacy

/** Ponte para a biblioteca nativa. Toda a lógica está em C++ (native/rom_verifier). */
object NativeBridge {
    init {
        System.loadLibrary("oml_native")
    }

    /** Lê e fecha o descritor, verifica a ROM e devolve linhas "chave=valor". */
    external fun verifyRomFd(fd: Int): String

    external fun nativeInfo(): String
}
