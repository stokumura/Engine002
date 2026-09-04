#include "FileReader.h"
#include <fstream>
#include <sstream>
#include "../Logger/Logger.h"

std::string FileReader::ReadFile(const std::string &filePath) {
    static const std::string empty = "";
    std::ifstream file(filePath); if(!file) {
        LOG("ERROR::FILE_READER Failed to open file: %s", filePath.c_str());
        return empty;
    }
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        std::string content = buffer.str();
        return content;
    } catch(std::ifstream::failure &exception) {
        LOG("ERROR::FILE_READER Failed when reading from file\n%s", exception.what());
        return empty;
    }
}
