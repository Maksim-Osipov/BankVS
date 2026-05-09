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

#include <commctrl.h>

#include <exception>
#include <iomanip>
#include <sstream>
#include <string>

#pragma comment(lib, "Comctl32.lib")

namespace {
    const std::string DATA_FILE = "bank_data.txt";

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
    constexpr int ID_MENU_ABOUT = 1014;

    constexpr int ID_EDIT_OUTPUT = 1100;
    constexpr int ID_LIST_ACCOUNTS = 1101;
    constexpr int ID_STATUS_TEXT = 1102;
    constexpr int ID_GROUP_STATS = 1103;
    constexpr int ID_GROUP_TABLE = 1104;
    constexpr int ID_GROUP_LOG = 1105;

    const wchar_t* MainWindowClassName() {
        return L"BankVSMainWindowClass";
    }

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

    std::wstring FirstLine(const std::wstring& text) {
        const std::size_t position = text.find_first_of(L"\r\n");
        if (position == std::wstring::npos) {
            return text;
        }

        return text.substr(0, position);
    }

    bool IsErrorMessage(const std::wstring& text) {
        return text.rfind(L"Ошибка:", 0) == 0 || text.rfind(L"Не удалось", 0) == 0;
    }

    bool DataFileExists() {
        const DWORD attributes = GetFileAttributesW(L"bank_data.txt");
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    void InsertColumn(HWND listView, int index, int width, const wchar_t* title) {
        LVCOLUMNW column {};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = const_cast<LPWSTR>(title);
        column.cx = width;
        column.iSubItem = index;
        ListView_InsertColumn(listView, index, &column);
    }

    void SetItemText(HWND listView, int row, int column, const std::wstring& text) {
        ListView_SetItemText(listView, row, column, const_cast<LPWSTR>(text.c_str()));
    }
}

MainWindow::MainWindow(HINSTANCE hInstance, Bank& bank)
    : hInstance(hInstance),
      bank(bank),
      hwnd(nullptr),
      outputEdit(nullptr),
      accountListView(nullptr),
      statusText(nullptr),
      statsGroup(nullptr),
      tableGroup(nullptr),
      logGroup(nullptr),
      statsLabels {},
      closeAutoSaved(false) {
}

bool MainWindow::Create(int nCmdShow) {
    INITCOMMONCONTROLSEX controls {};
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);

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
        1220,
        760,
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

void MainWindow::CreateMainMenu() {
    HMENU mainMenu = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();
    HMENU operationsMenu = CreatePopupMenu();
    HMENU viewMenu = CreatePopupMenu();
    HMENU helpMenu = CreatePopupMenu();

    AppendMenuW(fileMenu, MF_STRING, ID_BTN_SAVE, L"Сохранить");
    AppendMenuW(fileMenu, MF_STRING, ID_BTN_LOAD, L"Загрузить");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(fileMenu, MF_STRING, ID_BTN_EXIT, L"Выход");

    AppendMenuW(operationsMenu, MF_STRING, ID_BTN_CREATE_ACCOUNT, L"Создать счёт");
    AppendMenuW(operationsMenu, MF_STRING, ID_BTN_DEPOSIT, L"Пополнить");
    AppendMenuW(operationsMenu, MF_STRING, ID_BTN_WITHDRAW, L"Снять");
    AppendMenuW(operationsMenu, MF_STRING, ID_BTN_TRANSFER, L"Перевести");

    AppendMenuW(viewMenu, MF_STRING, ID_BTN_SHOW_ALL, L"Счета");
    AppendMenuW(viewMenu, MF_STRING, ID_BTN_STATISTICS, L"Статистика");

    AppendMenuW(helpMenu, MF_STRING, ID_MENU_ABOUT, L"О программе");

    AppendMenuW(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu), L"Файл");
    AppendMenuW(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(operationsMenu), L"Операции");
    AppendMenuW(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(viewMenu), L"Вид");
    AppendMenuW(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(helpMenu), L"Справка");

    SetMenu(hwnd, mainMenu);
}

void MainWindow::CreateControls() {
    CreateMainMenu();

    const int leftX = 20;
    const int topY = 46;
    const int buttonWidth = 250;
    const int buttonHeight = 34;
    const int gap = 40;

    DialogUtils::CreateLabel(hwnd, L"Операции", leftX, 16, 250, 22);
    DialogUtils::CreateControl(hwnd, L"STATIC", L"", SS_ETCHEDFRAME, 12, 38, 274, 560, 0);

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

    statsGroup = DialogUtils::CreateControl(hwnd, L"BUTTON", L"Краткая статистика", BS_GROUPBOX, 310, 14, 860, 112, ID_GROUP_STATS);
    tableGroup = DialogUtils::CreateControl(hwnd, L"BUTTON", L"Счета", BS_GROUPBOX, 310, 134, 860, 330, ID_GROUP_TABLE);
    logGroup = DialogUtils::CreateControl(hwnd, L"BUTTON", L"Журнал", BS_GROUPBOX, 310, 474, 860, 180, ID_GROUP_LOG);

    for (std::size_t i = 0; i < statsLabels.size(); ++i) {
        const int x = i < 3 ? 330 : 720;
        const int y = 40 + static_cast<int>(i % 3) * 26;
        statsLabels[i] = DialogUtils::CreateLabel(hwnd, L"", x, y, 360, 22);
    }

    accountListView = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        330,
        158,
        820,
        280,
        hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_LIST_ACCOUNTS)),
        hInstance,
        nullptr);

    if (accountListView) {
        SendMessageW(accountListView, WM_SETFONT, reinterpret_cast<WPARAM>(DialogUtils::DefaultFont()), TRUE);
        ListView_SetExtendedListViewStyle(accountListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
        InsertColumn(accountListView, 0, 90, L"ID");
        InsertColumn(accountListView, 1, 180, L"Тип счёта");
        InsertColumn(accountListView, 2, 330, L"Владелец");
        InsertColumn(accountListView, 3, 160, L"Баланс");
    }

    outputEdit = DialogUtils::CreateOutputEdit(hwnd, ID_EDIT_OUTPUT, 330, 498, 820, 132);
    statusText = DialogUtils::CreateControlEx(hwnd, WS_EX_CLIENTEDGE, L"STATIC", L"",
                                              SS_LEFT | SS_CENTERIMAGE, 0, 0, 0, 0, ID_STATUS_TEXT);

    AppendOutput(L"Банковская система готова к работе.");
    SetStatus(L"Готово.");
    AutoLoad();
    RefreshAccountTable();
    RefreshStatisticsPanel();
}

void MainWindow::ResizeControls() {
    RECT rect {};
    GetClientRect(hwnd, &rect);

    const int margin = 12;
    const int leftWidth = 286;
    const int dataX = leftWidth + 18;
    const int statusHeight = 26;
    const int dataWidth = static_cast<int>(rect.right) - dataX - margin;
    const int bottom = static_cast<int>(rect.bottom) - statusHeight - margin;
    const int statsHeight = 112;
    const int gap = 10;
    const int logHeight = 160;
    const int tableY = 14 + statsHeight + gap;
    const int tableHeight = bottom - tableY - logHeight - gap;
    const int logY = tableY + tableHeight + gap;

    if (statusText) {
        MoveWindow(statusText, 0, rect.bottom - statusHeight, rect.right, statusHeight, TRUE);
    }
    if (statsGroup) {
        MoveWindow(statsGroup, dataX, 14, dataWidth, statsHeight, TRUE);
    }
    if (tableGroup) {
        MoveWindow(tableGroup, dataX, tableY, dataWidth, tableHeight, TRUE);
    }
    if (logGroup) {
        MoveWindow(logGroup, dataX, logY, dataWidth, logHeight, TRUE);
    }

    for (std::size_t i = 0; i < statsLabels.size(); ++i) {
        const int columnWidth = (dataWidth - 60) / 2;
        const int x = dataX + 20 + (i < 3 ? 0 : columnWidth + 20);
        const int y = 40 + static_cast<int>(i % 3) * 26;
        MoveWindow(statsLabels[i], x, y, columnWidth, 22, TRUE);
    }

    if (accountListView) {
        MoveWindow(accountListView, dataX + 20, tableY + 26, dataWidth - 40, tableHeight - 46, TRUE);
        ListView_SetColumnWidth(accountListView, 0, 90);
        ListView_SetColumnWidth(accountListView, 1, 180);
        ListView_SetColumnWidth(accountListView, 2, dataWidth > 700 ? dataWidth - 500 : 220);
        ListView_SetColumnWidth(accountListView, 3, 160);
    }

    if (outputEdit) {
        MoveWindow(outputEdit, dataX + 20, logY + 26, dataWidth - 40, logHeight - 46, TRUE);
    }
}

void MainWindow::RefreshAccountTable() {
    if (!accountListView) {
        return;
    }

    ListView_DeleteAllItems(accountListView);

    int row = 0;
    for (const auto& pair : bank.getAccounts()) {
        const std::shared_ptr<Account>& account = pair.second;
        const std::wstring idText = std::to_wstring(account->getId());

        LVITEMW item {};
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(idText.c_str());
        ListView_InsertItem(accountListView, &item);

        SetItemText(accountListView, row, 1, AccountTypeToRussian(account->getType()));
        SetItemText(accountListView, row, 2, DialogUtils::Utf8ToWide(account->getOwnerName()));
        SetItemText(accountListView, row, 3, FormatMoney(account->getBalance()));
        ++row;
    }
}

void MainWindow::RefreshStatisticsPanel() {
    int savingsCount = 0;
    int checkingCount = 0;
    int creditCount = 0;
    double positiveBalance = 0.0;
    double creditDebt = 0.0;

    for (const auto& pair : bank.getAccounts()) {
        const std::shared_ptr<Account>& account = pair.second;
        const std::string type = account->getType();
        const double balance = account->getBalance();

        if (type == "Savings") {
            ++savingsCount;
        } else if (type == "Checking") {
            ++checkingCount;
        } else if (type == "Credit") {
            ++creditCount;
            if (balance < 0.0) {
                creditDebt += -balance;
            }
        }

        if (balance > 0.0) {
            positiveBalance += balance;
        }
    }

    const int totalCount = static_cast<int>(bank.getAccounts().size());
    DialogUtils::SetText(statsLabels[0], L"Всего счетов: " + std::to_wstring(totalCount));
    DialogUtils::SetText(statsLabels[1], L"Накопительных счетов: " + std::to_wstring(savingsCount));
    DialogUtils::SetText(statsLabels[2], L"Расчётных счетов: " + std::to_wstring(checkingCount));
    DialogUtils::SetText(statsLabels[3], L"Кредитных счетов: " + std::to_wstring(creditCount));
    DialogUtils::SetText(statsLabels[4], L"Общий положительный баланс: " + FormatMoney(positiveBalance));
    DialogUtils::SetText(statsLabels[5], L"Общий долг по кредитным счетам: " + FormatMoney(creditDebt));
}

void MainWindow::SetStatus(const std::wstring& text) {
    if (statusText) {
        SetWindowTextW(statusText, (L"  " + FirstLine(text)).c_str());
    }
}

void MainWindow::ShowAboutDialog() {
    const wchar_t* text =
        L"Банковская система\r\n"
        L"Учебный проект по информатике\r\n"
        L"C++17, WinAPI\r\n\r\n"
        L"Использованные темы:\r\n"
        L"ООП, наследование, полиморфизм, шаблоны, исключения,\r\n"
        L"STL, файлы, перегрузка операторов.";

    MessageBoxW(hwnd, text, L"О программе", MB_OK | MB_ICONINFORMATION);
    SetStatus(L"Открыта справка о программе.");
}

void MainWindow::AutoSave() {
    try {
        FileManager::saveBank(bank, DATA_FILE);
    } catch (const std::exception& ex) {
        std::wstring message = L"Не удалось автоматически сохранить данные в bank_data.txt.";
        const std::wstring details = DialogUtils::Utf8ToWide(ex.what());
        if (!details.empty()) {
            message += L"\r\n";
            message += details;
        }

        AppendOutput(message);
        SetStatus(L"Ошибка автоматического сохранения.");
        MessageBoxW(hwnd, message.c_str(), L"Ошибка сохранения", MB_OK | MB_ICONERROR);
    }
}

void MainWindow::AutoLoad() {
    if (!DataFileExists()) {
        return;
    }

    try {
        FileManager::loadBank(bank, DATA_FILE);
        AppendOutput(L"Данные автоматически загружены из bank_data.txt.");
        SetStatus(L"Данные автоматически загружены.");
    } catch (const std::exception& ex) {
        std::wstring message = L"Не удалось автоматически загрузить данные из bank_data.txt. Возможно, файл повреждён.";
        const std::wstring details = DialogUtils::Utf8ToWide(ex.what());
        if (!details.empty()) {
            message += L"\r\n";
            message += details;
        }

        AppendOutput(message);
        SetStatus(L"Ошибка автоматической загрузки.");
        MessageBoxW(hwnd, message.c_str(), L"Ошибка загрузки", MB_OK | MB_ICONWARNING);
    }
}

void MainWindow::ProcessCommand(int commandId) {
    const auto outputCallback = [this](const std::wstring& text) {
        AppendOutput(text);
        SetStatus(text);
        RefreshAccountTable();
        RefreshStatisticsPanel();
    };

    const auto mutatingOutputCallback = [this](const std::wstring& text) {
        AppendOutput(text);
        SetStatus(text);
        RefreshAccountTable();
        RefreshStatisticsPanel();
        if (!IsErrorMessage(text)) {
            AutoSave();
        }
    };

    try {
        switch (commandId) {
        case ID_BTN_CREATE_ACCOUNT:
            CreateAccountDialog::Show(hInstance, hwnd, bank, mutatingOutputCallback);
            break;
        case ID_BTN_DEPOSIT:
            MoneyOperationDialog::Show(hInstance, hwnd, bank, MoneyOperationType::Deposit, mutatingOutputCallback);
            break;
        case ID_BTN_WITHDRAW:
            MoneyOperationDialog::Show(hInstance, hwnd, bank, MoneyOperationType::Withdraw, mutatingOutputCallback);
            break;
        case ID_BTN_TRANSFER:
            TransferDialog::Show(hInstance, hwnd, bank, mutatingOutputCallback);
            break;
        case ID_BTN_INTEREST:
            InterestDialog::Show(hInstance, hwnd, bank, mutatingOutputCallback);
            break;
        case ID_BTN_SHOW_ALL:
            RefreshAccountTable();
            RefreshStatisticsPanel();
            AppendOutputUtf8(bank.getAllAccountsText());
            SetStatus(L"Таблица счетов обновлена.");
            break;
        case ID_BTN_HISTORY:
            HistoryDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_SAVE:
            FileManager::saveBank(bank, DATA_FILE);
            AppendOutput(L"Данные успешно сохранены в bank_data.txt.");
            SetStatus(L"Данные успешно сохранены.");
            break;
        case ID_BTN_LOAD:
            FileManager::loadBank(bank, DATA_FILE);
            RefreshAccountTable();
            RefreshStatisticsPanel();
            AppendOutput(L"Данные успешно загружены из bank_data.txt.");
            SetStatus(L"Данные успешно загружены.");
            break;
        case ID_BTN_STATEMENT:
            StatementDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_STATISTICS:
            StatisticsDialog::Show(hInstance, hwnd, bank, outputCallback);
            break;
        case ID_BTN_CLEAR:
            SetWindowTextW(outputEdit, L"");
            SetStatus(L"Журнал очищен.");
            break;
        case ID_MENU_ABOUT:
            ShowAboutDialog();
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
        ProcessCommand(LOWORD(wParam));
        return 0;
    case WM_CLOSE:
        if (!closeAutoSaved) {
            AutoSave();
            closeAutoSaved = true;
        }
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        if (!closeAutoSaved) {
            AutoSave();
            closeAutoSaved = true;
        }
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}
