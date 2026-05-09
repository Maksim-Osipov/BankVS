#include "FileManager.h"

#include <fstream>
#include <stdexcept>

void FileManager::saveBank(const Bank& bank, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file for saving.");
    }

    file << bank;
}

void FileManager::loadBank(Bank& bank, const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file for loading.");
    }

    file >> bank;
}
