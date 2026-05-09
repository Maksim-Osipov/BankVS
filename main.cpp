#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif
#define NOMINMAX

#include "Bank.h"
#include "Exceptions.h"
#include "FileManager.h"
#include "StatisticsVisualizer.h"

#include <windows.h>

#include <algorithm>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    constexpr int ID_EDIT_ACCOUNT_ID = 101;
    constexpr int ID_EDIT_TARGET_ID = 102;
    constexpr int ID_EDIT_OWNER = 103;
    constexpr int ID_EDIT_AMOUNT = 104;
    constexpr int ID_EDIT_INITIAL_BALANCE = 105;
    constexpr int ID_EDIT_INTEREST_RATE = 106;
    constexpr int ID_EDIT_OVERDRAFT_LIMIT = 107;
    constexpr int ID_EDIT_CREDIT_LIMIT = 108;
    constexpr int ID_EDIT_OUTPUT = 109;

    constexpr int ID_BTN_CREATE_SAVINGS = 201;
    constexpr int ID_BTN_CREATE_CHECKING = 202;
    constexpr int ID_BTN_CREATE_CREDIT = 203;
    constexpr int ID_BTN_DEPOSIT = 204;
    constexpr int ID_BTN_WITHDRAW = 205;
    constexpr int ID_BTN_TRANSFER = 206;
    constexpr int ID_BTN_INTEREST_ONE = 207;
    constexpr int ID_BTN_INTEREST_ALL = 208;
    constexpr int ID_BTN_SHOW_ALL = 209;
    constexpr int ID_BTN_HISTORY = 210;
    constexpr int ID_BTN_SAVE = 211;
    constexpr int ID_BTN_LOAD = 212;
    constexpr int ID_BTN_STATEMENT = 213;
    constexpr int ID_BTN_TABLE = 214;
    constexpr int ID_BTN_BALANCE_CHART = 215;
    constexpr int ID_BTN_TYPE_CHART = 216;
    constexpr int ID_BTN_CLEAR = 217;
    constexpr int ID_BTN_EXIT = 218;

    Bank g_bank;
    HWND g_outputEdit = nullptr;
    HFONT g_defaultFont = nullptr;
    std::vector<HWND> g_controls;

    std::wstring utf8ToWide(const std::string& text) {
        if (text.empty()) {
            return L"";
        }

        const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        if (size <= 0) {
            return L"";
        }

        std::wstring result(static_cast<std::size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &result[0], size);
        result.resize(static_cast<std::size_t>(size - 1));
        return result;
    }

    std::string wideToUtf8(const std::wstring& text) {
        if (text.empty()) {
            return "";
        }

        const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0) {
            return "";
        }

        std::string result(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, &result[0], size, nullptr, nullptr);
        result.resize(static_cast<std::size_t>(size - 1));
        return result;
    }

    std::wstring getWindowText(HWND hwnd) {
        const int length = GetWindowTextLengthW(hwnd);
        if (length == 0) {
            return L"";
        }

        std::wstring text(static_cast<std::size_t>(length + 1), L'\0');
        GetWindowTextW(hwnd, &text[0], length + 1);
        text.resize(static_cast<std::size_t>(length));
        return text;
    }

    HWND getControl(HWND parent, int id) {
        return GetDlgItem(parent, id);
    }

    std::wstring getInputText(HWND parent, int id) {
        return getWindowText(getControl(parent, id));
    }

    int readInt(HWND parent, int id, const wchar_t* fieldName) {
        const std::wstring text = getInputText(parent, id);
        if (text.empty()) {
            throw InvalidAmountException(wideToUtf8(std::wstring(L"Поле \"") + fieldName + L"\" не заполнено."));
        }

        std::size_t processed = 0;
        int value = 0;
        try {
            value = std::stoi(text, &processed);
        } catch (...) {
            throw InvalidAmountException(wideToUtf8(std::wstring(L"Введите корректное целое число в поле \"") + fieldName + L"\"."));
        }

        if (processed != text.size()) {
            throw InvalidAmountException(wideToUtf8(std::wstring(L"Введите корректное целое число в поле \"") + fieldName + L"\"."));
        }

        return value;
    }

    double readDouble(HWND parent, int id, const wchar_t* fieldName) {
        std::wstring text = getInputText(parent, id);
        std::replace(text.begin(), text.end(), L',', L'.');

        if (text.empty()) {
            throw InvalidAmountException(wideToUtf8(std::wstring(L"Поле \"") + fieldName + L"\" не заполнено."));
        }

        std::size_t processed = 0;
        double value = 0.0;
        try {
            value = std::stod(text, &processed);
        } catch (...) {
            throw InvalidAmountException(wideToUtf8(std::wstring(L"Введите корректную сумму в поле \"") + fieldName + L"\"."));
        }

        if (processed != text.size()) {
            throw InvalidAmountException(wideToUtf8(std::wstring(L"Введите корректную сумму в поле \"") + fieldName + L"\"."));
        }

        return value;
    }

    std::string readOwnerName(HWND parent) {
        const std::wstring owner = getInputText(parent, ID_EDIT_OWNER);
        if (owner.empty()) {
            throw InvalidAccountException(wideToUtf8(L"Введите ФИО владельца."));
        }

        return wideToUtf8(owner);
    }

    void appendOutput(const std::wstring& text) {
        if (!g_outputEdit) {
            return;
        }

        const int length = GetWindowTextLengthW(g_outputEdit);
        SendMessageW(g_outputEdit, EM_SETSEL, length, length);
        SendMessageW(g_outputEdit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
        SendMessageW(g_outputEdit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L"\r\n"));
    }

    void appendOutputUtf8(const std::string& text) {
        std::wstring wideText = utf8ToWide(text);
        std::wstring normalized;
        normalized.reserve(wideText.size() * 2);

        for (wchar_t ch : wideText) {
            if (ch == L'\n') {
                normalized += L"\r\n";
            } else {
                normalized += ch;
            }
        }

        appendOutput(normalized);
    }

    void showError(HWND hwnd, const std::wstring& message) {
        appendOutput(message);
        MessageBoxW(hwnd, message.c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
    }

    void handleException(HWND hwnd, const std::exception& ex) {
        std::wstring message = L"Ошибка: операция не выполнена.";

        if (dynamic_cast<const InvalidAccountException*>(&ex)) {
            message = L"Ошибка: счёт не найден или указан неверный ID.";
        } else if (dynamic_cast<const InvalidAmountException*>(&ex)) {
            message = L"Ошибка: введите корректную сумму или параметры счёта.";
        } else if (dynamic_cast<const InsufficientFundsException*>(&ex)) {
            message = L"Ошибка: недостаточно средств или превышен лимит.";
        }

        const std::wstring details = utf8ToWide(ex.what());
        if (!details.empty()) {
            message += L"\r\n";
            message += details;
        }

        showError(hwnd, message);
    }

    HWND createControl(HWND parent, const wchar_t* className, const wchar_t* text, DWORD style,
                       int x, int y, int width, int height, int id) {
        HWND hwnd = CreateWindowExW(
            0,
            className,
            text,
            WS_CHILD | WS_VISIBLE | style,
            x,
            y,
            width,
            height,
            parent,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            GetModuleHandleW(nullptr),
            nullptr);

        if (hwnd && g_defaultFont) {
            SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_defaultFont), TRUE);
        }

        if (hwnd) {
            g_controls.push_back(hwnd);
        }

        return hwnd;
    }

    void createLabelAndEdit(HWND hwnd, const wchar_t* label, int editId, int x, int y, int editWidth) {
        createControl(hwnd, L"STATIC", label, 0, x, y + 4, 150, 22, 0);
        createControl(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, x + 155, y, editWidth, 24, editId);
    }

    void createButton(HWND hwnd, const wchar_t* text, int id, int x, int y, int width) {
        createControl(hwnd, L"BUTTON", text, BS_PUSHBUTTON, x, y, width, 30, id);
    }

    void createMainControls(HWND hwnd) {
        g_defaultFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

        createLabelAndEdit(hwnd, L"ID счёта:", ID_EDIT_ACCOUNT_ID, 15, 15, 290);
        createLabelAndEdit(hwnd, L"ID получателя:", ID_EDIT_TARGET_ID, 15, 45, 290);
        createLabelAndEdit(hwnd, L"ФИО владельца:", ID_EDIT_OWNER, 15, 75, 290);
        createLabelAndEdit(hwnd, L"Сумма:", ID_EDIT_AMOUNT, 15, 105, 290);
        createLabelAndEdit(hwnd, L"Начальный баланс:", ID_EDIT_INITIAL_BALANCE, 15, 135, 290);
        createLabelAndEdit(hwnd, L"Процентная ставка:", ID_EDIT_INTEREST_RATE, 15, 165, 290);
        createLabelAndEdit(hwnd, L"Лимит овердрафта:", ID_EDIT_OVERDRAFT_LIMIT, 15, 195, 290);
        createLabelAndEdit(hwnd, L"Кредитный лимит:", ID_EDIT_CREDIT_LIMIT, 15, 225, 290);

        const int leftX = 15;
        const int rightX = 340;
        const int buttonY = 265;
        const int buttonWidth = 310;
        const int buttonGap = 36;

        createButton(hwnd, L"Создать накопительный счёт", ID_BTN_CREATE_SAVINGS, leftX, buttonY, buttonWidth);
        createButton(hwnd, L"Создать расчётный счёт", ID_BTN_CREATE_CHECKING, rightX, buttonY, buttonWidth);
        createButton(hwnd, L"Создать кредитный счёт", ID_BTN_CREATE_CREDIT, leftX, buttonY + buttonGap, buttonWidth);
        createButton(hwnd, L"Пополнить счёт", ID_BTN_DEPOSIT, rightX, buttonY + buttonGap, buttonWidth);
        createButton(hwnd, L"Снять деньги", ID_BTN_WITHDRAW, leftX, buttonY + buttonGap * 2, buttonWidth);
        createButton(hwnd, L"Перевести деньги", ID_BTN_TRANSFER, rightX, buttonY + buttonGap * 2, buttonWidth);
        createButton(hwnd, L"Начислить проценты по счёту", ID_BTN_INTEREST_ONE, leftX, buttonY + buttonGap * 3, buttonWidth);
        createButton(hwnd, L"Начислить проценты всем", ID_BTN_INTEREST_ALL, rightX, buttonY + buttonGap * 3, buttonWidth);
        createButton(hwnd, L"Показать все счета", ID_BTN_SHOW_ALL, leftX, buttonY + buttonGap * 4, buttonWidth);
        createButton(hwnd, L"Показать историю", ID_BTN_HISTORY, rightX, buttonY + buttonGap * 4, buttonWidth);
        createButton(hwnd, L"Сохранить в файл", ID_BTN_SAVE, leftX, buttonY + buttonGap * 5, buttonWidth);
        createButton(hwnd, L"Загрузить из файла", ID_BTN_LOAD, rightX, buttonY + buttonGap * 5, buttonWidth);
        createButton(hwnd, L"Сгенерировать выписку", ID_BTN_STATEMENT, leftX, buttonY + buttonGap * 6, buttonWidth);
        createButton(hwnd, L"Показать таблицу счетов", ID_BTN_TABLE, rightX, buttonY + buttonGap * 6, buttonWidth);
        createButton(hwnd, L"Показать график балансов", ID_BTN_BALANCE_CHART, leftX, buttonY + buttonGap * 7, buttonWidth);
        createButton(hwnd, L"Показать статистику типов счетов", ID_BTN_TYPE_CHART, rightX, buttonY + buttonGap * 7, buttonWidth);
        createButton(hwnd, L"Очистить вывод", ID_BTN_CLEAR, leftX, buttonY + buttonGap * 8, buttonWidth);
        createButton(hwnd, L"Выход", ID_BTN_EXIT, rightX, buttonY + buttonGap * 8, buttonWidth);

        g_outputEdit = createControl(
            hwnd,
            L"EDIT",
            L"",
            WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            670,
            15,
            610,
            680,
            ID_EDIT_OUTPUT);

        appendOutput(L"Банковская система готова к работе.");
    }

    void resizeOutput(HWND hwnd) {
        if (!g_outputEdit) {
            return;
        }

        RECT rect {};
        GetClientRect(hwnd, &rect);

        const int leftPanelWidth = 670;
        const int margin = 15;
        const int outputX = leftPanelWidth;
        const int outputY = margin;
        const int outputWidth = std::max<int>(300, static_cast<int>(rect.right - outputX - margin));
        const int outputHeight = std::max<int>(200, static_cast<int>(rect.bottom - rect.top - 20));

        MoveWindow(g_outputEdit, outputX, outputY, outputWidth, outputHeight, TRUE);
    }

    void processCommand(HWND hwnd, int commandId) {
        const std::string dataFile = "bank_data.txt";

        try {
            switch (commandId) {
            case ID_BTN_CREATE_SAVINGS: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                const std::string owner = readOwnerName(hwnd);
                const double balance = readDouble(hwnd, ID_EDIT_INITIAL_BALANCE, L"Начальный баланс");
                const double rate = readDouble(hwnd, ID_EDIT_INTEREST_RATE, L"Процентная ставка");
                g_bank.createSavingsAccount(id, owner, balance, rate);
                appendOutput(L"Счёт успешно создан.");
                break;
            }
            case ID_BTN_CREATE_CHECKING: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                const std::string owner = readOwnerName(hwnd);
                const double balance = readDouble(hwnd, ID_EDIT_INITIAL_BALANCE, L"Начальный баланс");
                const double overdraft = readDouble(hwnd, ID_EDIT_OVERDRAFT_LIMIT, L"Лимит овердрафта");
                g_bank.createCheckingAccount(id, owner, balance, overdraft);
                appendOutput(L"Счёт успешно создан.");
                break;
            }
            case ID_BTN_CREATE_CREDIT: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                const std::string owner = readOwnerName(hwnd);
                const double balance = readDouble(hwnd, ID_EDIT_INITIAL_BALANCE, L"Начальный баланс");
                const double creditLimit = readDouble(hwnd, ID_EDIT_CREDIT_LIMIT, L"Кредитный лимит");
                const double rate = readDouble(hwnd, ID_EDIT_INTEREST_RATE, L"Процентная ставка");
                g_bank.createCreditAccount(id, owner, balance, creditLimit, rate);
                appendOutput(L"Счёт успешно создан.");
                break;
            }
            case ID_BTN_DEPOSIT: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                const double amount = readDouble(hwnd, ID_EDIT_AMOUNT, L"Сумма");
                g_bank.deposit(id, amount);
                appendOutput(L"Деньги успешно зачислены.");
                break;
            }
            case ID_BTN_WITHDRAW: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                const double amount = readDouble(hwnd, ID_EDIT_AMOUNT, L"Сумма");
                g_bank.withdraw(id, amount);
                appendOutput(L"Деньги успешно сняты.");
                break;
            }
            case ID_BTN_TRANSFER: {
                const int fromId = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                const int toId = readInt(hwnd, ID_EDIT_TARGET_ID, L"ID получателя");
                const double amount = readDouble(hwnd, ID_EDIT_AMOUNT, L"Сумма");
                g_bank.transfer(fromId, toId, amount);
                appendOutput(L"Перевод успешно выполнен.");
                break;
            }
            case ID_BTN_INTEREST_ONE: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                g_bank.applyInterestToAccount(id);
                appendOutput(L"Операция выполнена успешно.");
                break;
            }
            case ID_BTN_INTEREST_ALL:
                g_bank.applyInterestToAllAccounts();
                appendOutput(L"Операция выполнена успешно.");
                break;
            case ID_BTN_SHOW_ALL:
                appendOutputUtf8(g_bank.getAllAccountsText());
                break;
            case ID_BTN_HISTORY: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                appendOutputUtf8(g_bank.getAccountHistoryText(id));
                break;
            }
            case ID_BTN_SAVE:
                FileManager::saveBank(g_bank, dataFile);
                appendOutput(L"Данные успешно сохранены.");
                break;
            case ID_BTN_LOAD:
                FileManager::loadBank(g_bank, dataFile);
                appendOutput(L"Данные успешно загружены.");
                break;
            case ID_BTN_STATEMENT: {
                const int id = readInt(hwnd, ID_EDIT_ACCOUNT_ID, L"ID счёта");
                const std::string filename = "statement_" + std::to_string(id) + ".txt";
                g_bank.generateStatement(id, filename);
                appendOutput(L"Выписка успешно создана.");
                break;
            }
            case ID_BTN_TABLE:
                appendOutputUtf8(StatisticsVisualizer::getAccountTable(g_bank.getAccounts()));
                break;
            case ID_BTN_BALANCE_CHART:
                appendOutputUtf8(StatisticsVisualizer::getBalanceChart(g_bank.getAccounts()));
                break;
            case ID_BTN_TYPE_CHART:
                appendOutputUtf8(StatisticsVisualizer::getAccountTypeChart(g_bank.getAccounts()));
                break;
            case ID_BTN_CLEAR:
                SetWindowTextW(g_outputEdit, L"");
                break;
            case ID_BTN_EXIT:
                PostMessageW(hwnd, WM_CLOSE, 0, 0);
                break;
            default:
                break;
            }
        } catch (const std::exception& ex) {
            handleException(hwnd, ex);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        createMainControls(hwnd);
        return 0;
    case WM_SIZE:
        resizeOutput(hwnd);
        return 0;
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            processCommand(hwnd, LOWORD(wParam));
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t className[] = L"ConsoleBankWindowClass";

    WNDCLASSW windowClass {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&windowClass)) {
        MessageBoxW(nullptr, L"Не удалось зарегистрировать класс окна.", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExW(
        0,
        className,
        L"Банковская система",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1320,
        760,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hwnd) {
        MessageBoxW(nullptr, L"Не удалось создать главное окно.", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG message {};
    while (GetMessageW(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
