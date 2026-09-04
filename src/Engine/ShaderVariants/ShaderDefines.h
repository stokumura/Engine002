#pragma once

#include <cstdint>
#include <string>
#include <vector>

class ShaderDefines {
    public:
        static uint32_t Hash(const std::vector<std::string> &defines);
    private:
        static uint64_t MixHash(uint64_t hash);
};
