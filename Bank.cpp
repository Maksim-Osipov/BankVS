#include "Bank.h"

#include "CheckingAccount.h"
#include "CreditAccount.h"
#include "Exceptions.h"
#include "SavingsAccount.h"

#include <fstream>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

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

    std::string transactionTypeToRussian(const std::string& type) {
        if (type == "Deposit") {
            return "Пополнение";
        }
        if (type == "Withdraw") {
            return "Снятие";
        }
        if (type == "Transfer In") {
            return "Входящий перевод";
        }
        if (type == "Transfer Out") {
            return "Исходящий перевод";
        }
        if (type == "Interest") {
            return "Проценты";
        }
        if (type == "Credit Interest") {
            return "Проценты по кредиту";
        }

        return type;
    }
}

void Bank::validateNewAccount(int id) const {
    if (id <= 0) {
        throw InvalidAccountException("Account ID must be positive.");
    }

    if (accounts.find(id) != accounts.end()) {
        throw InvalidAccountException("Account with this ID already exists.");
    }
}

void Bank::createSavingsAccount(int id, const std::string& ownerName, double initialBalance, double interestRate) {
    validateNewAccount(id);

    if (initialBalance < 0.0) {
        throw InvalidAmountException("Savings account initial balance cannot be negative.");
    }

    if (interestRate < 0.0) {
        throw InvalidAmountException("Interest rate cannot be negative.");
    }

    accounts[id] = std::make_shared<SavingsAccount>(id, ownerName, initialBalance, interestRate);
}

void Bank::createCheckingAccount(int id, const std::string& ownerName, double initialBalance, double overdraftLimit) {
    validateNewAccount(id);

    if (overdraftLimit < 0.0) {
        throw InvalidAmountException("Overdraft limit cannot be negative.");
    }

    if (initialBalance < -overdraftLimit) {
        throw InvalidAmountException("Initial balance exceeds overdraft limit.");
    }

    accounts[id] = std::make_shared<CheckingAccount>(id, ownerName, initialBalance, overdraftLimit);
}

void Bank::createCreditAccount(int id, const std::string& ownerName, double initialBalance, double creditLimit, double interestRate) {
    validateNewAccount(id);

    if (creditLimit < 0.0) {
        throw InvalidAmountException("Credit limit cannot be negative.");
    }

    if (interestRate < 0.0) {
        throw InvalidAmountException("Interest rate cannot be negative.");
    }

    if (initialBalance < -creditLimit) {
        throw InvalidAmountException("Initial balance exceeds credit limit.");
    }

    accounts[id] = std::make_shared<CreditAccount>(id, ownerName, initialBalance, creditLimit, interestRate);
}

std::shared_ptr<Account> Bank::getAccount(int id) {
    auto it = accounts.find(id);
    if (it == accounts.end()) {
        throw InvalidAccountException("Account not found.");
    }

    return it->second;
}

void Bank::removeAccount(int id) {
    if (accounts.erase(id) == 0) {
        throw InvalidAccountException("Account not found.");
    }
}

void Bank::deposit(int id, double amount) {
    getAccount(id)->deposit(amount);
}

void Bank::withdraw(int id, double amount) {
    getAccount(id)->withdraw(amount);
}

void Bank::transfer(int fromId, int toId, double amount) {
    *this + TransferRequest { fromId, toId, amount };
}

void Bank::applyInterestToAccount(int id) {
    getAccount(id)->applyInterest();
}

void Bank::applyInterestToAllAccounts() {
    for (const auto& pair : accounts) {
        pair.second->applyInterest();
    }
}

void Bank::printAllAccounts() const {
    std::cout << getAllAccountsText();
}

void Bank::printAccountHistory(int id) const {
    std::cout << getAccountHistoryText(id);
}

std::string Bank::getAllAccountsText() const {
    std::ostringstream oss;

    if (accounts.empty()) {
        oss << "Счета не найдены.\n";
        return oss.str();
    }

    oss << "Список счетов:\n\n";
    oss << std::left << std::setw(8) << "ID"
        << std::setw(18) << "Тип"
        << std::setw(24) << "Владелец"
        << std::right << std::setw(12) << "Баланс" << '\n';
    oss << std::string(62, '-') << '\n';

    for (const auto& pair : accounts) {
        const std::shared_ptr<Account>& account = pair.second;
        oss << std::left << std::setw(8) << account->getId()
            << std::setw(18) << accountTypeToRussian(account->getType())
            << std::setw(24) << account->getOwnerName()
            << std::right << std::setw(12) << std::fixed << std::setprecision(2)
            << account->getBalance() << '\n';
    }

    return oss.str();
}

std::string Bank::getAccountHistoryText(int id) const {
    auto it = accounts.find(id);
    if (it == accounts.end()) {
        throw InvalidAccountException("Account not found.");
    }

    std::ostringstream oss;
    const std::vector<Transaction<double>>& history = it->second->getHistory();
    if (history.empty()) {
        oss << "История операций пуста.\n";
        return oss.str();
    }

    oss << "История операций счёта " << id << ":\n\n";
    for (const Transaction<double>& transaction : history) {
        oss << std::left << std::setw(12) << transaction.getDate()
            << " | " << std::setw(22) << transactionTypeToRussian(transaction.getType())
            << " | " << std::right << std::setw(10) << std::fixed << std::setprecision(2)
            << transaction.getAmount();

        if (transaction.getRelatedAccountId() != -1) {
            oss << " | связанный счёт: " << transaction.getRelatedAccountId();
        }

        oss << '\n';
    }

    return oss.str();
}

void Bank::generateStatement(int id, const std::string& filename) const {
    auto it = accounts.find(id);
    if (it == accounts.end()) {
        throw InvalidAccountException("Account not found.");
    }

    const std::filesystem::path statementPath(filename);
    if (statementPath.has_parent_path()) {
        std::filesystem::create_directories(statementPath.parent_path());
    }

    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot create statement file.");
    }

    const std::shared_ptr<Account>& account = it->second;
    file << "ВЫПИСКА ПО СЧЁТУ\n";
    file << "ID счёта: " << account->getId() << '\n';
    file << "Владелец: " << account->getOwnerName() << '\n';
    file << "Тип счёта: " << accountTypeToRussian(account->getType()) << '\n';
    file << "Текущий баланс: " << std::fixed << std::setprecision(2) << account->getBalance() << "\n\n";

    file << "История операций:\n";
    for (const Transaction<double>& transaction : account->getHistory()) {
        file << transaction << '\n';
    }
}

const std::map<int, std::shared_ptr<Account>>& Bank::getAccounts() const {
    return accounts;
}

void Bank::clear() {
    accounts.clear();
}

Bank& Bank::operator+(const TransferRequest& request) {
    if (request.amount <= 0.0) {
        throw InvalidAmountException("Transfer amount must be positive.");
    }

    if (request.fromId == request.toId) {
        throw InvalidAccountException("Cannot transfer money to the same account.");
    }

    std::shared_ptr<Account> fromAccount = getAccount(request.fromId);
    std::shared_ptr<Account> toAccount = getAccount(request.toId);

    if (!fromAccount->canWithdraw(request.amount)) {
        throw InsufficientFundsException("Transfer failed: insufficient funds or limit exceeded.");
    }

    // Direct balance changes avoid adding ordinary Deposit/Withdraw records.
    // The history receives specific Transfer Out and Transfer In records.
    fromAccount->changeBalance(-request.amount);
    toAccount->changeBalance(request.amount);

    fromAccount->addTransaction(Transaction<double>(request.amount, Account::currentDate(), "Transfer Out", request.toId));
    toAccount->addTransaction(Transaction<double>(request.amount, Account::currentDate(), "Transfer In", request.fromId));

    return *this;
}

std::ostream& operator<<(std::ostream& os, const Bank& bank) {
    // Serialization of the whole bank. Polymorphism calls the correct derived serialize method.
    os << "ACCOUNTS " << bank.accounts.size() << '\n';

    for (const auto& pair : bank.accounts) {
        pair.second->serialize(os);
    }

    return os;
}

std::istream& operator>>(std::istream& is, Bank& bank) {
    std::string tag;
    std::size_t accountCount = 0;

    if (!(is >> tag >> accountCount) || tag != "ACCOUNTS") {
        throw std::runtime_error("Invalid bank file format.");
    }

    bank.clear();

    for (std::size_t i = 0; i < accountCount; ++i) {
        std::string accountType;
        int id = 0;
        std::string ownerName;
        double balance = 0.0;
        double interestRate = 0.0;
        double overdraftLimit = 0.0;
        double creditLimit = 0.0;
        std::size_t transactionCount = 0;

        if (!(is >> tag >> accountType) || tag != "ACCOUNT") {
            throw std::runtime_error("Invalid account record.");
        }

        if (!(is >> tag >> id) || tag != "ID") {
            throw std::runtime_error("Invalid account ID.");
        }

        if (!(is >> tag >> std::quoted(ownerName)) || tag != "OWNER") {
            throw std::runtime_error("Invalid owner name.");
        }

        if (!(is >> tag >> balance) || tag != "BALANCE") {
            throw std::runtime_error("Invalid account balance.");
        }

        if (accountType == "Savings") {
            if (!(is >> tag >> interestRate) || tag != "INTEREST_RATE") {
                throw std::runtime_error("Invalid savings account data.");
            }
            bank.createSavingsAccount(id, ownerName, balance, interestRate);
        } else if (accountType == "Checking") {
            if (!(is >> tag >> overdraftLimit) || tag != "OVERDRAFT_LIMIT") {
                throw std::runtime_error("Invalid checking account data.");
            }
            bank.createCheckingAccount(id, ownerName, balance, overdraftLimit);
        } else if (accountType == "Credit") {
            if (!(is >> tag >> creditLimit) || tag != "CREDIT_LIMIT") {
                throw std::runtime_error("Invalid credit account data.");
            }
            if (!(is >> tag >> interestRate) || tag != "INTEREST_RATE") {
                throw std::runtime_error("Invalid credit account data.");
            }
            bank.createCreditAccount(id, ownerName, balance, creditLimit, interestRate);
        } else {
            throw std::runtime_error("Unknown account type.");
        }

        if (!(is >> tag >> transactionCount) || tag != "TRANSACTIONS") {
            throw std::runtime_error("Invalid transaction list.");
        }

        std::shared_ptr<Account> account = bank.getAccount(id);
        for (std::size_t j = 0; j < transactionCount; ++j) {
            Transaction<double> transaction;
            if (!transaction.readFromFile(is)) {
                throw std::runtime_error("Invalid transaction record.");
            }
            account->addTransaction(transaction);
        }

        if (!(is >> tag) || tag != "END_ACCOUNT") {
            throw std::runtime_error("Missing END_ACCOUNT marker.");
        }
    }

    return is;
}
