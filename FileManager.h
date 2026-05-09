#pragma once

#include "Bank.h"

#include <string>

class FileManager {
public:
    static void saveBank(const Bank& bank, const std::string& filename);
    static void loadBank(Bank& bank, const std::string& filename);
};
