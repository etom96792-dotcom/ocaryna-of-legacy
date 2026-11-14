#include "sha1.hpp"

#include <cstring>

namespace oml {

static inline uint32_t rol(uint32_t v, int n) { return (v << n) | (v >> (32 - n)); }

Sha1::Sha1() : h_{0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u} {}

void Sha1::block(const uint8_t* p) {
    uint32_t w[80];
    for (int i = 0; i < 16; ++i) {
        w[i] = (uint32_t(p[i * 4]) << 24) | (uint32_t(p[i * 4 + 1]) << 16) |
               (uint32_t(p[i * 4 + 2]) << 8) | uint32_t(p[i * 4 + 3]);
    }
    for (int i = 16; i < 80; ++i) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

    uint32_t a = h_[0], b = h_[1], c = h_[2], d = h_[3], e = h_[4];
    for (int i = 0; i < 80; ++i) {
        uint32_t f, k;
        if (i < 20)      { f = (b & c) | (~b & d);          k = 0x5A827999u; }
        else if (i < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1u; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDCu; }
        else             { f = b ^ c ^ d;                   k = 0xCA62C1D6u; }
        uint32_t t = rol(a, 5) + f + e + k + w[i];
        e = d; d = c; c = rol(b, 30); b = a; a = t;
    }
    h_[0] += a; h_[1] += b; h_[2] += c; h_[3] += d; h_[4] += e;
}

void Sha1::update(const uint8_t* data, size_t len) {
    totalBytes_ += len;
    while (len > 0) {
        size_t take = 64 - bufLen_;
        if (take > len) take = len;
        std::memcpy(buf_ + bufLen_, data, take);
        bufLen_ += take; data += take; len -= take;
        if (bufLen_ == 64) { block(buf_); bufLen_ = 0; }
    }
}

std::array<uint8_t, 20> Sha1::finish() {
    uint64_t bits = totalBytes_ * 8;
    uint8_t pad = 0x80;
    update(&pad, 1);
    uint8_t zero = 0;
    while (bufLen_ != 56) update(&zero, 1);
    uint8_t lenBytes[8];
    for (int i = 0; i < 8; ++i) lenBytes[i] = uint8_t(bits >> (56 - 8 * i));
    update(lenBytes, 8);

    std::array<uint8_t, 20> out{};
    for (int i = 0; i < 5; ++i) {
        out[i * 4] = uint8_t(h_[i] >> 24);
        out[i * 4 + 1] = uint8_t(h_[i] >> 16);
        out[i * 4 + 2] = uint8_t(h_[i] >> 8);
        out[i * 4 + 3] = uint8_t(h_[i]);
    }
    return out;
}

std::string Sha1::toHex(const std::array<uint8_t, 20>& d) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    for (uint8_t b : d) { s += hex[b >> 4]; s += hex[b & 15]; }
    return s;
}

}  // namespace oml
