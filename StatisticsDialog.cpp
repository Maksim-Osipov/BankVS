#include "StatisticsDialog.h"

#include "DialogUtils.h"
#include "StatisticsVisualizer.h"

#include <cmath>
#include <exception>
#include <iomanip>
#include <sstream>

namespace {
    constexpr int ID_BTN_TABLE = 8001;
    constexpr int ID_BTN_BALANCE = 8002;
    constexpr int ID_BTN_TYPES = 8003;
    constexpr int ID_EDIT_OUTPUT = 8004;
    constexpr int ID_BTN_CLOSE = 8005;

    const wchar_t* ClassName() {
        return L"BankVSStatisticsDialog";
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

    void TextOutString(HDC hdc, int x, int y, const std::wstring& text) {
        TextOutW(hdc, x, y, text.c_str(), static_cast<int>(text.size()));
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank,
               const std::function<void(const std::wstring&)>& outputCallback)
            : hInstance(hInstance), owner(owner), bank(bank), outputCallback(outputCallback),
              hwnd(nullptr), outputEdit(nullptr), showBalanceGraphic(false) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, ClassName(), L"Статистика",
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 860, 600,
                                   owner, nullptr, hInstance, this);
            if (!hwnd) {
                return false;
            }

            DialogUtils::CenterWindow(hwnd, owner);
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
            return true;
        }

        static LRESULT CALLBACK Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
            Dialog* dialog = nullptr;
            if (message == WM_NCCREATE) {
                CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
                dialog = static_cast<Dialog*>(createStruct->lpCreateParams);
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dialog));
                dialog->hwnd = hwnd;
            } else {
                dialog = reinterpret_cast<Dialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            }

            if (!dialog) {
                return DefWindowProcW(hwnd, message, wParam, lParam);
            }

            const LRESULT result = dialog->Handle(message, wParam, lParam);
            if (message == WM_NCDESTROY) {
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
                delete dialog;
            }
            return result;
        }

    private:
        HINSTANCE hInstance;
        HWND owner;
        Bank& bank;
        std::function<void(const std::wstring&)> outputCallback;
        HWND hwnd;
        HWND outputEdit;
        RECT chartRect;
        bool showBalanceGraphic;

        void Register() {
            WNDCLASSW windowClass {};
            windowClass.lpfnWndProc = Dialog::Proc;
            windowClass.hInstance = hInstance;
            windowClass.lpszClassName = ClassName();
            windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            RegisterClassW(&windowClass);
        }

        void CreateControls() {
            DialogUtils::CreateButton(hwnd, L"Таблица счетов", ID_BTN_TABLE, 20, 20, 150, 32);
            DialogUtils::CreateButton(hwnd, L"График балансов", ID_BTN_BALANCE, 185, 20, 155, 32);
            DialogUtils::CreateButton(hwnd, L"Статистика типов счетов", ID_BTN_TYPES, 355, 20, 210, 32);
            DialogUtils::CreateButton(hwnd, L"Закрыть", ID_BTN_CLOSE, 585, 20, 100, 32);
            outputEdit = DialogUtils::CreateOutputEdit(hwnd, ID_EDIT_OUTPUT, 20, 70, 800, 220);
            ResizeControls();
        }

        void ResizeControls() {
            if (!outputEdit) {
                return;
            }

            RECT rect {};
            GetClientRect(hwnd, &rect);
            const int width = static_cast<int>(rect.right) - 40;
            const int outputHeight = 210;
            MoveWindow(outputEdit, 20, 70, width > 300 ? width : 300, outputHeight, TRUE);

            chartRect.left = 20;
            chartRect.top = 300;
            chartRect.right = rect.right - 20;
            chartRect.bottom = rect.bottom - 25;
            InvalidateRect(hwnd, &chartRect, TRUE);
        }

        void SetOutput(const std::string& text, const std::wstring& message, bool drawGraphic) {
            showBalanceGraphic = drawGraphic;
            DialogUtils::SetText(outputEdit, DialogUtils::NormalizeLineBreaks(DialogUtils::Utf8ToWide(text)));
            outputCallback(message);
            InvalidateRect(hwnd, &chartRect, TRUE);
        }

        void DrawBalanceChart(HDC hdc) {
            Rectangle(hdc, chartRect.left, chartRect.top, chartRect.right, chartRect.bottom);
            TextOutString(hdc, chartRect.left + 12, chartRect.top + 10, L"Графическая визуализация балансов");

            if (!showBalanceGraphic) {
                TextOutString(hdc, chartRect.left + 12, chartRect.top + 40,
                              L"Нажмите «График балансов», чтобы построить диаграмму.");
                return;
            }

            if (bank.getAccounts().empty()) {
                TextOutString(hdc, chartRect.left + 12, chartRect.top + 40, L"Нет данных для графика.");
                return;
            }

            double maxAbsBalance = 0.0;
            for (const auto& pair : bank.getAccounts()) {
                const double value = std::fabs(pair.second->getBalance());
                if (value > maxAbsBalance) {
                    maxAbsBalance = value;
                }
            }
            if (maxAbsBalance <= 0.0) {
                maxAbsBalance = 1.0;
            }

            const int left = chartRect.left + 150;
            const int top = chartRect.top + 44;
            const int barAreaWidth = chartRect.right - left - 150;
            const int rowHeight = 26;
            const int maxRows = (chartRect.bottom - top - 12) / rowHeight;
            int row = 0;

            HBRUSH positiveBrush = CreateSolidBrush(RGB(78, 151, 96));
            HBRUSH negativeBrush = CreateSolidBrush(RGB(194, 86, 86));
            HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, positiveBrush));
            SetBkMode(hdc, TRANSPARENT);

            for (const auto& pair : bank.getAccounts()) {
                if (row >= maxRows) {
                    TextOutString(hdc, chartRect.left + 12, top + row * rowHeight,
                                  L"Показаны не все счета: увеличьте окно.");
                    break;
                }

                const std::shared_ptr<Account>& account = pair.second;
                const double balance = account->getBalance();
                const int y = top + row * rowHeight;
                const int barWidth = static_cast<int>((std::fabs(balance) / maxAbsBalance) * barAreaWidth);
                const std::wstring label = std::to_wstring(account->getId()) + L"  " +
                                           AccountTypeToRussian(account->getType());
                const std::wstring valueText = FormatMoney(balance);

                TextOutString(hdc, chartRect.left + 12, y + 2, label);
                SelectObject(hdc, balance >= 0.0 ? positiveBrush : negativeBrush);
                Rectangle(hdc, left, y + 3, left + (barWidth > 2 ? barWidth : 2), y + 19);
                TextOutString(hdc, left + barWidth + 8, y + 2, valueText);
                ++row;
            }

            SelectObject(hdc, oldBrush);
            DeleteObject(positiveBrush);
            DeleteObject(negativeBrush);
        }

        LRESULT Handle(UINT message, WPARAM wParam, LPARAM lParam) {
            switch (message) {
            case WM_CREATE:
                CreateControls();
                return 0;
            case WM_SIZE:
                ResizeControls();
                return 0;
            case WM_PAINT: {
                PAINTSTRUCT paint {};
                HDC hdc = BeginPaint(hwnd, &paint);
                DrawBalanceChart(hdc);
                EndPaint(hwnd, &paint);
                return 0;
            }
            case WM_COMMAND:
                if (LOWORD(wParam) == ID_BTN_TABLE && HIWORD(wParam) == BN_CLICKED) {
                    SetOutput(StatisticsVisualizer::getAccountTable(bank.getAccounts()), L"Показана таблица счетов.", false);
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_BALANCE && HIWORD(wParam) == BN_CLICKED) {
                    SetOutput(StatisticsVisualizer::getBalanceChart(bank.getAccounts()), L"Показан график балансов.", true);
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_TYPES && HIWORD(wParam) == BN_CLICKED) {
                    SetOutput(StatisticsVisualizer::getAccountTypeChart(bank.getAccounts()), L"Показана статистика типов счетов.", false);
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_CLOSE && HIWORD(wParam) == BN_CLICKED) {
                    DestroyWindow(hwnd);
                    return 0;
                }
                break;
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;
            default:
                break;
            }
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    };
}

void StatisticsDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                            const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно статистики.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
