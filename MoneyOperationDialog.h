#pragma once

#include <windows.h>

#include "Bank.h"

#include <functional>
#include <string>

enum class MoneyOperationType {
    Deposit,
    Withdraw
};

class MoneyOperationDialog {
public:
    static void Show(HINSTANCE hInstance, HWND owner, Bank& bank, MoneyOperationType operationType,
                     const std::function<void(const std::wstring&)>& outputCallback,
                     int prefilledAccountId = -1);
};
