#include "UiHelpers.h"

#include "CheckingAccount.h"
#include "CreditAccount.h"
#include "DialogUtils.h"
#include "SavingsAccount.h"

#include <cwctype>
#include <iomanip>
#include <sstream>

namespace UiHelpers {
    std::wstring AccountTypeToRussian(const std::string& type) {
        if (type == "Savings") {
            return L"Накопительный";
        }
        if (type == "Checking") {
            return L"Расчётный";
        }
        if (type == "Credit") {
            return L"Кредитный";
        }

        return DialogUtils::Utf8ToWide(type);
    }

    std::wstring FormatMoney(double value) {
        std::wostringstream oss;
        oss << std::fixed << std::setprecision(2) << value;
        return oss.str();
    }

    std::wstring FormatPercent(double value) {
        std::wostringstream oss;
        oss << std::fixed << std::setprecision(2) << value * 100.0 << L"%";
        return oss.str();
    }

    std::wstring GetAccountRateText(const std::shared_ptr<Account>& account) {
        if (const SavingsAccount* savings = dynamic_cast<const SavingsAccount*>(account.get())) {
            return FormatPercent(savings->getInterestRate());
        }
        if (const CreditAccount* credit = dynamic_cast<const CreditAccount*>(account.get())) {
            return FormatPercent(credit->getInterestRate());
        }

        return L"-";
    }

    std::wstring GetAccountLimitText(const std::shared_ptr<Account>& account) {
        if (const CheckingAccount* checking = dynamic_cast<const CheckingAccount*>(account.get())) {
            return L"Овердрафт: " + FormatMoney(checking->getOverdraftLimit());
        }
        if (const CreditAccount* credit = dynamic_cast<const CreditAccount*>(account.get())) {
            return L"Кредит: " + FormatMoney(credit->getCreditLimit());
        }

        return L"-";
    }

    std::wstring GetAccountSummaryText(const std::shared_ptr<Account>& account) {
        return L"ID: " + std::to_wstring(account->getId()) +
               L" | Тип: " + AccountTypeToRussian(account->getType()) +
               L" | Владелец: " + DialogUtils::Utf8ToWide(account->getOwnerName()) +
               L" | Баланс: " + FormatMoney(account->getBalance()) +
               L" | Ставка: " + GetAccountRateText(account) +
               L" | Лимит / овердрафт: " + GetAccountLimitText(account);
    }

    std::wstring ToLower(std::wstring text) {
        for (wchar_t& ch : text) {
            ch = static_cast<wchar_t>(std::towlower(ch));
        }
        return text;
    }
}
