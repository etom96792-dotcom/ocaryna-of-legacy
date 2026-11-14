// Verificador de ROM de Ocarina of Time (Fase 2).
// Não depende de Android: roda no host para teste e é chamado via JNI no app.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace oml {

enum class RomFormat { Unknown, Z64_BigEndian, V64_ByteSwapped, N64_LittleEndian };

enum class RomStatus {
    Ok,                 // hash conhecido
    TooSmallOrBadSize,  // tamanho não é 32 MiB nem 64 MiB
    UnknownFormat,      // magic não reconhecido
    NotOcarina,         // cabeçalho não é de OoT (título/código)
    UnknownHash,        // parece OoT, mas o SHA-1 não está na tabela (ROM alterada ou versão não suportada)
    IsZipArchive,       // arquivo .zip: o usuário precisa descompactar antes
};

struct RomReport {
    RomStatus status = RomStatus::UnknownFormat;
    RomFormat format = RomFormat::Unknown;
    uint64_t size = 0;
    std::string gameCode;   // 4 caracteres, ex.: "CZLE"
    int revision = -1;      // byte 0x3F
    uint32_t crc1 = 0;
    uint32_t crc2 = 0;
    std::string sha1;       // do ROM normalizado para z64
    std::string versionName;  // nome da tabela, se Ok
    std::string message;    // texto curto, em português, para a UI
};

// `data` é o arquivo inteiro como lido. Ele não é modificado.
RomReport verifyRom(const std::vector<uint8_t>& data);

// Normaliza para z64 (big-endian). Retorna vazio se o formato for desconhecido.
std::vector<uint8_t> normalizeToZ64(const std::vector<uint8_t>& data, RomFormat* detected);

const char* statusName(RomStatus s);

}  // namespace oml
