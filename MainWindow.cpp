#include "MainWindow.h"

#include "CreateAccountDialog.h"
#include "DialogUtils.h"
#include "FileManager.h"
#include "HistoryDialog.h"
#include "InterestDialog.h"
#include "MoneyOperationDialog.h"
#include "StatementDialog.h"
#include "StatisticsDialog.h"
#include "TransferDialog.h"

#include <exception>

namespace {
    constexpr int ID_BTN_CREATE_ACCOUNT = 1001;
    constexpr int ID_BTN_DEPOSIT = 1002;
    constexpr int ID_BTN_WITHDRAW = 1003;
    constexpr int ID_BTN_TRANSFER = 1004;
    constexpr int ID_BTN_INTEREST = 1005;
    constexpr int ID_BTN_SHOW_ALL = 1006;
    constexpr int ID_BTN_HISTORY = 1007;
    constexpr int ID_BTN_SAVE = 1008;
    constexpr int ID_BTN_LOAD = 1009;
    constexpr int ID_BTN_STATEMENT = 1010;
    constexpr int ID_BTN_STATISTICS = 1011;
    constexpr int ID_BTN_CLEAR = 1012;
    constexpr int ID_BTN_EXIT = 1013;
    constexpr int ID_EDIT_OUTPUT = 1100;

    const wchar_t* MainWindowClassName() {
        return L"BankVSMainWindowClass";
    }
}

MainWindow::MainWindow(HINSTANCE hInstance, Bank& bank)
    : hInstance(hInstance), bank(bank), hwnd(nullptr), outputEdit(nullptr) {
}

bool MainWindow::Create(int nCmdShow) {
    if (!RegisterWindowClass()) {
        MessageBoxW(nullptr, L"Не удалось зарегистрировать класс главного окна.", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    hwnd = CreateWindowExW(
        0,
        MainWindowClassName(),
        L"Банковская система",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1180,
        720,
        nullptr,
        nullptr,
        hInstance,
        this);

    if (!hwnd) {
        MessageBoxW(nullptr, L"Не удалось создать главное окно.", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return true;
}

int MainWindow::Run() {
    MSG message {};
    while (GetMessageW(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}

void MainWindow::AppendOutput(const std::wstring& text) {
    DialogUtils::AppendOutput(outputEdit, text);
}

void MainWindow::AppendOutputUtf8(const std::string& text) {
    DialogUtils::AppendOutputUtf8(outputEdit, text);
}

bool MainWindow::RegisterWindowClass() {
    WNDCLASSEXW windowClass {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = MainWindow::WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = MainWindowClassName();
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    return RegisterClassExW(&windowClass) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

void MainWindow::CreateControls() {
    const int leftX = 20;
    const int topY = 20;
    const int buttonWidth = 250;
    const int buttonHeight = 36;
    const int gap = 44;

    DialogUtils::CreateButton(hwnd, L"Создать счёт", ID_BTN_CREATE_ACCOUNT, leftX, topY + gap * 0, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Пополнить счёт", ID_BTN_DEPOSIT, leftX, topY + gap * 1, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Снять деньги", ID_BTN_WITHDRAW, leftX, topY + gap * 2, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Перевести деньги", ID_BTN_TRANSFER, leftX, topY + gap * 3, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Начислить проценты", ID_BTN_INTEREST, leftX, topY + gap * 4, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Показать все счета", ID_BTN_SHOW_ALL, leftX, topY + gap * 5, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"История операций", ID_BTN_HISTORY, leftX, topY + gap * 6, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Сохранить данные", ID_BTN_SAVE, leftX, topY + gap * 7, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Загрузить данные", ID_BTN_LOAD, leftX, topY + gap * 8, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Выписка", ID_BTN_STATEMENT, leftX, topY + gap * 9, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Статистика", ID_BTN_STATISTICS, leftX, topY + gap * 10, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Очистить вывод", ID_BTN_CLEAR, leftX, topY + gap * 11, buttonWidth, buttonHeight);
    DialogUtils::CreateButton(hwnd, L"Выход", ID_BTN_EXIT, leftX, topY + gap * 12, buttonWidth, buttonHeight);

    outputEdit = DialogUtils::CreateOutputEdit(hwnd, ID_EDIT_OUTPUT, 300, 20, 840, 620);
    AppendOutput(L"Банковская система готова к работе.");
}

void MainWindow::ResizeControls() {
    if (!outputEdit) {
        return;
    }

    RECT rect {};
    GetClientRect(hwnd, &rect);

    const int outputX = 300;
    const int margin = 20;
    const int calculatedWidth = static_cast<int>(rect.right) - outputX - margin;
    const int calculatedHeight = static_cast<int>(rect.bottom) - margin * 2;
    const int width = calculatedWidth > 320 ? calculatedWidth : 320;
    const int height = calculatedHeight > 240 ? calculatedHeight : 240;
    MoveWindow(outputEdit, outputX, margin, width, height, TRUE);
}

void MainWindow::ProcessCommand(int commandId) {
    const auto outputCallback = [this](const std::wstring& text) {
        AppendOutput(text);
    };

    try {
        switch (commandId) {
        case ID_BTN_CREATE_ACCOUNT:
            CreateAccountDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_DEPOSIT:
            MoneyOperationDialog::Show(hInstance, hwnd, bank, MoneyOperationType::Deposit, outputCallback);
            break;
        case ID_BTN_WITHDRAW:
            MoneyOperationDialog::Show(hInstance, hwnd, bank, MoneyOperationType::Withdraw, outputCallback);
            break;
        case ID_BTN_TRANSFER:
            TransferDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_INTEREST:
            InterestDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_SHOW_ALL:
            AppendOutputUtf8(bank.getAllAccountsText());
            break;
        case ID_BTN_HISTORY:
            HistoryDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_SAVE:
            FileManager::saveBank(bank, "bank_data.txt");
            AppendOutput(L"Данные успешно сохранены в bank_data.txt.");
            break;
        case ID_BTN_LOAD:
            FileManager::loadBank(bank, "bank_data.txt");
            AppendOutput(L"Данные успешно загружены из bank_data.txt.");
            break;
        case ID_BTN_STATEMENT:
            StatementDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_STATISTICS:
            StatisticsDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_CLEAR:
            SetWindowTextW(outputEdit, L"");
            break;
        case ID_BTN_EXIT:
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
            break;
        default:
            break;
        }
    } catch (const std::exception& ex) {
        DialogUtils::ShowException(hwnd, ex, outputCallback);
    }
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* window = nullptr;

    if (message == WM_NCCREATE) {
        CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = static_cast<MainWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->hwnd = hwnd;
    } else {
        window = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->HandleMessage(message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls();
        return 0;
    case WM_SIZE:
        ResizeControls();
        return 0;
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            ProcessCommand(LOWORD(wParam));
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}
