// SHA-1 mínimo, sem dependências. Usado só para identificar a ROM (não para segurança).
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace oml {

class Sha1 {
public:
    Sha1();
    void update(const uint8_t* data, size_t len);
    std::array<uint8_t, 20> finish();  // só pode ser chamado uma vez
    static std::string toHex(const std::array<uint8_t, 20>& digest);

private:
    void block(const uint8_t* p);
    uint32_t h_[5];
    uint8_t buf_[64];
    size_t bufLen_ = 0;
    uint64_t totalBytes_ = 0;
};

}  // namespace oml
