#pragma once

#include "Account.h"

#include <map>
#include <memory>
#include <string>

class StatisticsVisualizer {
public:
    static std::string getAccountTable(const std::map<int, std::shared_ptr<Account>>& accounts);
    static std::string getBalanceChart(const std::map<int, std::shared_ptr<Account>>& accounts);
    static std::string getAccountTypeChart(const std::map<int, std::shared_ptr<Account>>& accounts);

    static void showAccountTable(const std::map<int, std::shared_ptr<Account>>& accounts);
    static void showBalanceChart(const std::map<int, std::shared_ptr<Account>>& accounts);
    static void showAccountTypeChart(const std::map<int, std::shared_ptr<Account>>& accounts);
};
