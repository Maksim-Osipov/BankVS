#pragma once

#include "Account.h"

#include <memory>
#include <string>

namespace UiHelpers {
    std::wstring AccountTypeToRussian(const std::string& type);
    std::wstring FormatMoney(double value);
    std::wstring FormatPercent(double value);
    std::wstring GetAccountRateText(const std::shared_ptr<Account>& account);
    std::wstring GetAccountLimitText(const std::shared_ptr<Account>& account);
    std::wstring GetAccountSummaryText(const std::shared_ptr<Account>& account);
    std::wstring ToLower(std::wstring text);
}
