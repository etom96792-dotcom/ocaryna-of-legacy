// Ponte JNI fina. Toda a lógica está em native/rom_verifier (testada no host).
#include <jni.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <string>
#include <vector>

#include "rom_verifier.hpp"

namespace {

constexpr size_t kMaxRead = 64ull * 1024 * 1024 + 1;  // 64 MiB + 1: acima disso já é inválido

// Lê o descritor até EOF (ou até kMaxRead). Fecha o descritor sempre.
bool readAll(int fd, std::vector<uint8_t>& out) {
    out.clear();
    out.reserve(33u * 1024 * 1024);
    uint8_t buf[1 << 16];
    bool ok = true;
    for (;;) {
        ssize_t n = ::read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR) continue;
            ok = false;
            break;
        }
        if (n == 0) break;
        out.insert(out.end(), buf, buf + n);
        if (out.size() >= kMaxRead) break;
    }
    ::close(fd);
    return ok;
}

}  // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_com_ocarinamobile_legacy_NativeBridge_verifyRomFd(JNIEnv* env, jobject /*thiz*/, jint fd) {
    std::vector<uint8_t> data;
    std::string result;
    if (!readAll(fd, data)) {
        result = "status=ReadError\nmessage=Falha ao ler o arquivo.\n";
    } else {
        oml::RomReport r = oml::verifyRom(data);
        char crc[32];
        std::snprintf(crc, sizeof(crc), "%08X/%08X", r.crc1, r.crc2);
        result = std::string("status=") + oml::statusName(r.status) + "\n" +
                 "size=" + std::to_string(r.size) + "\n" +
                 "format=" + std::to_string(static_cast<int>(r.format)) + "\n" +
                 "gameCode=" + r.gameCode + "\n" +
                 "revision=" + std::to_string(r.revision) + "\n" +
                 "crc=" + crc + "\n" +
                 "sha1=" + r.sha1 + "\n" +
                 "version=" + r.versionName + "\n" +
                 "message=" + r.message + "\n";
    }
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_ocarinamobile_legacy_NativeBridge_nativeInfo(JNIEnv* env, jobject /*thiz*/) {
#if defined(__aarch64__)
    const char* abi = "arm64-v8a";
#elif defined(__arm__)
    const char* abi = "armeabi-v7a";
#elif defined(__x86_64__)
    const char* abi = "x86_64";
#else
    const char* abi = "desconhecida";
#endif
    std::string s = std::string("abi_nativa=") + abi + "\ncpp=" + std::to_string(__cplusplus) + "\n";
    return env->NewStringUTF(s.c_str());
}
