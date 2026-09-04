#include "ShaderDefines.h"

uint64_t ShaderDefines::MixHash(uint64_t hash) {
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ULL;
    hash ^= hash >> 33;
    return hash;
}

uint32_t ShaderDefines::Hash(const std::vector<std::string> &defines) {
    uint64_t mixHash = 0;
    for (const std::string &define : defines) {
        uint64_t hash = static_cast<uint64_t>(std::hash<std::string>{}(define));
        mixHash ^= MixHash(hash);
    }
    mixHash = MixHash(mixHash);
    return static_cast<uint32_t>(mixHash);
}
