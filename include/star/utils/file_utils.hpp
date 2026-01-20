#pragma once
#include "star/core/types.hpp"
#include <fstream>
#include <vector>

namespace star::utils {
    inline std::vector<u8> read_binary_file(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {};
        }

        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<u8> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        file.close();

        return buffer;
    }

    inline bool file_exists(const std::string& path) {
        const std::ifstream file(path);
        return file.good();
    }
} // namespace star::utils
