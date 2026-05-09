#pragma once

#include <windows.h>

#include "Bank.h"

#include <functional>
#include <string>

class StatementDialog {
public:
    static void Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                     const std::function<void(const std::wstring&)>& outputCallback);
};
