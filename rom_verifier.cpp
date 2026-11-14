#include "rom_verifier.hpp"

#include <cstring>
#include <utility>

#include "sha1.hpp"

namespace oml {

namespace {

struct KnownRom { const char* name; const char* sha1; };

const KnownRom kKnown[] = {
#include "known_hashes.inc"
};

constexpr uint64_t kSize32 = 32ull * 1024 * 1024;
constexpr uint64_t kSize64 = 64ull * 1024 * 1024;

RomFormat detect(const uint8_t* p) {
    if (p[0] == 0x80 && p[1] == 0x37 && p[2] == 0x12 && p[3] == 0x40) return RomFormat::Z64_BigEndian;
    if (p[0] == 0x37 && p[1] == 0x80 && p[2] == 0x40 && p[3] == 0x12) return RomFormat::V64_ByteSwapped;
    if (p[0] == 0x40 && p[1] == 0x12 && p[2] == 0x37 && p[3] == 0x80) return RomFormat::N64_LittleEndian;
    return RomFormat::Unknown;
}

uint32_t be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
}

}  // namespace

const char* statusName(RomStatus s) {
    switch (s) {
        case RomStatus::Ok: return "Ok";
        case RomStatus::TooSmallOrBadSize: return "TooSmallOrBadSize";
        case RomStatus::UnknownFormat: return "UnknownFormat";
        case RomStatus::NotOcarina: return "NotOcarina";
        case RomStatus::UnknownHash: return "UnknownHash";
        case RomStatus::IsZipArchive: return "IsZipArchive";
    }
    return "?";
}

std::vector<uint8_t> normalizeToZ64(const std::vector<uint8_t>& data, RomFormat* detected) {
    RomFormat f = RomFormat::Unknown;
    if (data.size() >= 4) f = detect(data.data());
    if (detected) *detected = f;
    if (f == RomFormat::Unknown) return {};

    std::vector<uint8_t> out(data);
    const size_t n = out.size() & ~size_t(3);  // só palavras completas
    if (f == RomFormat::V64_ByteSwapped) {
        for (size_t i = 0; i < n; i += 2) std::swap(out[i], out[i + 1]);
    } else if (f == RomFormat::N64_LittleEndian) {
        for (size_t i = 0; i < n; i += 4) { std::swap(out[i], out[i + 3]); std::swap(out[i + 1], out[i + 2]); }
    }
    return out;
}

RomReport verifyRom(const std::vector<uint8_t>& data) {
    RomReport r;
    r.size = data.size();

    if (data.size() >= 4 && data[0] == 'P' && data[1] == 'K' && data[2] == 3 && data[3] == 4) {
        r.status = RomStatus::IsZipArchive;
        r.message = "Este arquivo é um .zip. Descompacte e escolha o arquivo .z64, .v64 ou .n64.";
        return r;
    }

    if (data.size() != kSize32 && data.size() != kSize64) {
        r.status = RomStatus::TooSmallOrBadSize;
        r.message = "Tamanho de arquivo inesperado. A ROM deve ter 32 MiB (N64) ou 64 MiB (GameCube).";
        return r;
    }

    std::vector<uint8_t> z = normalizeToZ64(data, &r.format);
    if (z.empty()) {
        r.status = RomStatus::UnknownFormat;
        r.message = "Formato de ROM não reconhecido. Use um arquivo .z64, .v64 ou .n64.";
        return r;
    }

    r.crc1 = be32(&z[0x10]);
    r.crc2 = be32(&z[0x14]);
    r.gameCode.assign(reinterpret_cast<const char*>(&z[0x3B]), 4);
    r.revision = z[0x3F];

    Sha1 h;
    h.update(z.data(), z.size());
    r.sha1 = Sha1::toHex(h.finish());

    // O hash decide. O cabeçalho só serve para escolher a mensagem de erro,
    // porque não verifiquei o título de todas as versões da tabela (GameCube, iQue).
    for (const auto& k : kKnown) {
        if (r.sha1 == k.sha1) {
            r.status = RomStatus::Ok;
            r.versionName = k.name;
            r.message = std::string("ROM reconhecida: ") + k.name + ".";
            return r;
        }
    }

    static const char kTitle[] = "THE LEGEND OF ZELDA";
    if (std::memcmp(&z[0x20], kTitle, sizeof(kTitle) - 1) != 0) {
        r.status = RomStatus::NotOcarina;
        r.message = "Este arquivo não parece ser Ocarina of Time.";
        return r;
    }
    r.status = RomStatus::UnknownHash;
    r.message = "Parece Ocarina of Time, mas a versão não é reconhecida (ROM alterada ou versão não suportada).";
    return r;
}

}  // namespace oml
