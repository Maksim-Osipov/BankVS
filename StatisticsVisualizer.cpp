#include "StatisticsVisualizer.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace {
    std::string accountTypeToRussian(const std::string& type) {
        if (type == "Savings") {
            return "Накопительный";
        }
        if (type == "Checking") {
            return "Расчётный";
        }
        if (type == "Credit") {
            return "Кредитный";
        }

        return type;
    }
}

std::string StatisticsVisualizer::getAccountTable(const std::map<int, std::shared_ptr<Account>>& accounts) {
    std::ostringstream oss;

    oss << "+---------+------------+----------------------+------------+\n";
    oss << "| " << std::left << std::setw(7) << "ID"
        << " | " << std::setw(10) << "Тип"
        << " | " << std::setw(20) << "Владелец"
        << " | " << std::setw(10) << "Баланс" << " |\n";
    oss << "+---------+------------+----------------------+------------+\n";

    for (const auto& pair : accounts) {
        const std::shared_ptr<Account>& account = pair.second;
        oss << "| " << std::left << std::setw(7) << account->getId()
            << " | " << std::setw(10) << accountTypeToRussian(account->getType())
            << " | " << std::setw(20) << account->getOwnerName()
            << " | " << std::right << std::setw(10) << std::fixed << std::setprecision(2)
            << account->getBalance() << " |\n";
    }

    oss << "+---------+------------+----------------------+------------+\n";
    return oss.str();
}

std::string StatisticsVisualizer::getBalanceChart(const std::map<int, std::shared_ptr<Account>>& accounts) {
    std::ostringstream oss;

    oss << "График балансов:\n\n";

    double maxAbsBalance = 0.0;
    for (const auto& pair : accounts) {
        maxAbsBalance = std::max(maxAbsBalance, std::abs(pair.second->getBalance()));
    }

    if (maxAbsBalance == 0.0) {
        maxAbsBalance = 1.0;
    }

    const int maxBarWidth = 30;
    for (const auto& pair : accounts) {
        const std::shared_ptr<Account>& account = pair.second;
        const double balance = account->getBalance();
        int barWidth = static_cast<int>((std::abs(balance) / maxAbsBalance) * maxBarWidth);

        if (barWidth == 0 && balance != 0.0) {
            barWidth = 1;
        }

        const char symbol = balance >= 0.0 ? '#' : '-';
        oss << std::left << std::setw(6) << account->getId()
            << std::setw(20) << account->getOwnerName()
            << " | " << std::string(barWidth, symbol) << ' '
            << std::fixed << std::setprecision(2) << balance << '\n';
    }

    return oss.str();
}

std::string StatisticsVisualizer::getAccountTypeChart(const std::map<int, std::shared_ptr<Account>>& accounts) {
    std::map<std::string, int> typeCounts;
    std::ostringstream oss;

    for (const auto& pair : accounts) {
        ++typeCounts[pair.second->getType()];
    }

    oss << "Статистика типов счетов:\n\n";
    for (const auto& pair : typeCounts) {
        oss << std::left << std::setw(16) << accountTypeToRussian(pair.first)
            << " | " << std::string(pair.second * 4, '#')
            << ' ' << pair.second << '\n';
    }

    return oss.str();
}

void StatisticsVisualizer::showAccountTable(const std::map<int, std::shared_ptr<Account>>& accounts) {
    std::cout << getAccountTable(accounts);
}

void StatisticsVisualizer::showBalanceChart(const std::map<int, std::shared_ptr<Account>>& accounts) {
    std::cout << getBalanceChart(accounts);
}

void StatisticsVisualizer::showAccountTypeChart(const std::map<int, std::shared_ptr<Account>>& accounts) {
    std::cout << getAccountTypeChart(accounts);
}
