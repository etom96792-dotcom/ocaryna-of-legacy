// Teste de host do verificador de ROM.
// Uso: rom_verifier_test <caminho da ROM z64 legítima>
// A ROM é só lida. Nada é gravado em disco.
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "../native/rom_verifier/rom_verifier.hpp"
#include "../native/rom_verifier/sha1.hpp"

using namespace oml;

static int failures = 0;
#define CHECK(cond, what) do { bool ok_ = (cond); std::printf("[%s] %s\n", ok_ ? "ok" : "FALHOU", what); if (!ok_) ++failures; } while (0)

static std::string sha1Of(const std::string& s) {
    Sha1 h;
    h.update(reinterpret_cast<const uint8_t*>(s.data()), s.size());
    return Sha1::toHex(h.finish());
}

int main(int argc, char** argv) {
    // Vetores de teste padrão do SHA-1 (FIPS 180)
    CHECK(sha1Of("abc") == "a9993e364706816aba3e25717850c26c9cd0d89d", "SHA-1 de 'abc'");
    CHECK(sha1Of("") == "da39a3ee5e6b4b0d3255bfef95601890afd80709", "SHA-1 de vazio");
    CHECK(sha1Of("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") == "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
          "SHA-1 do vetor de 56 bytes");

    if (argc < 2) { std::printf("Informe o caminho da ROM para os testes com ROM real.\n"); return failures ? 1 : 0; }

    std::ifstream in(argv[1], std::ios::binary);
    std::vector<uint8_t> z64((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(z64.size() == 32u * 1024 * 1024, "ROM tem 32 MiB");

    RomReport r = verifyRom(z64);
    std::printf("  status=%s formato=%d codigo=%s rev=%d crc1=%08X crc2=%08X\n  sha1=%s versao=%s\n  msg=%s\n",
                statusName(r.status), int(r.format), r.gameCode.c_str(), r.revision, r.crc1, r.crc2,
                r.sha1.c_str(), r.versionName.c_str(), r.message.c_str());
    CHECK(r.status == RomStatus::Ok, "ROM real: status Ok");
    CHECK(r.format == RomFormat::Z64_BigEndian, "ROM real: formato z64");
    CHECK(r.versionName == "NTSC 1.2 (US)", "ROM real: versão NTSC 1.2 (US)");
    CHECK(r.sha1 == "41b3bdc48d98c48529219919015a1af22f5057c2", "ROM real: SHA-1 esperado");
    CHECK(r.crc1 == 0x693BA2AE, "ROM real: CRC1 0x693BA2AE");
    CHECK(r.gameCode == "CZLE" && r.revision == 2, "ROM real: CZLE rev 2");

    // Variantes de formato geradas em memória
    std::vector<uint8_t> v64 = z64;
    for (size_t i = 0; i + 1 < v64.size(); i += 2) std::swap(v64[i], v64[i + 1]);
    RomReport rv = verifyRom(v64);
    CHECK(rv.status == RomStatus::Ok && rv.format == RomFormat::V64_ByteSwapped && rv.sha1 == r.sha1, "v64 normaliza para o mesmo SHA-1");

    std::vector<uint8_t> n64 = z64;
    for (size_t i = 0; i + 3 < n64.size(); i += 4) { std::swap(n64[i], n64[i + 3]); std::swap(n64[i + 1], n64[i + 2]); }
    RomReport rn = verifyRom(n64);
    CHECK(rn.status == RomStatus::Ok && rn.format == RomFormat::N64_LittleEndian && rn.sha1 == r.sha1, "n64 normaliza para o mesmo SHA-1");

    // Casos negativos
    std::vector<uint8_t> mod = z64; mod[0x1000] ^= 0x01;
    CHECK(verifyRom(mod).status == RomStatus::UnknownHash, "1 bit alterado: UnknownHash");

    std::vector<uint8_t> modTitle = z64; modTitle[0x1000] ^= 0x01; modTitle[0x20] = 'X';
    CHECK(verifyRom(modTitle).status == RomStatus::NotOcarina, "título alterado e hash desconhecido: NotOcarina");

    std::vector<uint8_t> trunc(z64.begin(), z64.begin() + 1000);
    CHECK(verifyRom(trunc).status == RomStatus::TooSmallOrBadSize, "arquivo truncado: TooSmallOrBadSize");

    std::vector<uint8_t> zeros(32u * 1024 * 1024, 0);
    CHECK(verifyRom(zeros).status == RomStatus::UnknownFormat, "32 MiB de zeros: UnknownFormat");

    CHECK(verifyRom({}).status == RomStatus::TooSmallOrBadSize, "vazio: TooSmallOrBadSize");

    std::vector<uint8_t> zip(26u * 1024 * 1024, 0xAB);
    zip[0] = 'P'; zip[1] = 'K'; zip[2] = 3; zip[3] = 4;
    CHECK(verifyRom(zip).status == RomStatus::IsZipArchive, "cabeçalho de .zip: IsZipArchive");

    std::printf("\n%s (%d falha(s))\n", failures ? "TESTES FALHARAM" : "TODOS OS TESTES PASSARAM", failures);
    return failures ? 1 : 0;
}
