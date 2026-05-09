#include "CreditAccount.h"
#include "Exceptions.h"

#include <cmath>
#include <iomanip>

CreditAccount::CreditAccount(int id, const std::string& ownerName, double balance, double creditLimit, double interestRate)
    : Account(id, ownerName, balance), creditLimit(creditLimit), interestRate(interestRate) {
}

bool CreditAccount::canWithdraw(double amount) const {
    return amount > 0.0 && balance - amount >= -creditLimit;
}

double CreditAccount::getCreditLimit() const {
    return creditLimit;
}

double CreditAccount::getInterestRate() const {
    return interestRate;
}

std::string CreditAccount::getType() const {
    return "Credit";
}

void CreditAccount::deposit(double amount) {
    // The base implementation is enough: a deposit increases the balance,
    // which reduces debt when the balance is negative.
    Account::deposit(amount);
}

void CreditAccount::withdraw(double amount) {
    if (amount <= 0.0) {
        throw InvalidAmountException("Withdraw amount must be positive.");
    }

    if (!canWithdraw(amount)) {
        throw InsufficientFundsException("Credit limit exceeded.");
    }

    changeBalance(-amount);
    addTransaction(Transaction<double>(amount, currentDate(), "Withdraw", -1));
}

void CreditAccount::applyInterest() {
    if (balance < 0.0) {
        const double interest = std::abs(balance) * interestRate;
        changeBalance(-interest);
        addTransaction(Transaction<double>(interest, currentDate(), "Credit Interest", -1));
    }
}

void CreditAccount::print(std::ostream& os) const {
    Account::print(os);
    os << ", Credit limit: " << std::fixed << std::setprecision(2) << creditLimit
       << ", Interest rate: " << interestRate;
}

void CreditAccount::serialize(std::ostream& os) const {
    os << "ACCOUNT Credit\n";
    os << "ID " << id << '\n';
    os << "OWNER " << std::quoted(ownerName) << '\n';
    os << "BALANCE " << std::fixed << std::setprecision(2) << balance << '\n';
    os << "CREDIT_LIMIT " << creditLimit << '\n';
    os << "INTEREST_RATE " << interestRate << '\n';
    os << "TRANSACTIONS " << history.size() << '\n';

    for (const Transaction<double>& transaction : history) {
        transaction.writeToFile(os);
    }

    os << "END_ACCOUNT\n";
}
