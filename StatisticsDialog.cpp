#include "StatisticsDialog.h"

#include "DialogUtils.h"
#include "StatisticsVisualizer.h"

#include <exception>

namespace {
    constexpr int ID_BTN_TABLE = 8001;
    constexpr int ID_BTN_BALANCE = 8002;
    constexpr int ID_BTN_TYPES = 8003;
    constexpr int ID_EDIT_OUTPUT = 8004;
    constexpr int ID_BTN_CLOSE = 8005;

    const wchar_t* ClassName() {
        return L"BankVSStatisticsDialog";
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank,
               const std::function<void(const std::wstring&)>& outputCallback)
            : hInstance(hInstance), owner(owner), bank(bank), outputCallback(outputCallback),
              hwnd(nullptr), outputEdit(nullptr) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, ClassName(), L"Статистика",
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 820, 540,
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
            outputEdit = DialogUtils::CreateOutputEdit(hwnd, ID_EDIT_OUTPUT, 20, 70, 760, 390);
        }

        void ResizeControls() {
            if (!outputEdit) {
                return;
            }

            RECT rect {};
            GetClientRect(hwnd, &rect);
            const int calculatedWidth = static_cast<int>(rect.right) - 40;
            const int calculatedHeight = static_cast<int>(rect.bottom) - 90;
            const int width = calculatedWidth > 300 ? calculatedWidth : 300;
            const int height = calculatedHeight > 180 ? calculatedHeight : 180;
            MoveWindow(outputEdit, 20, 70, width, height, TRUE);
        }

        void SetOutput(const std::string& text, const std::wstring& message) {
            DialogUtils::SetText(outputEdit, DialogUtils::NormalizeLineBreaks(DialogUtils::Utf8ToWide(text)));
            outputCallback(message);
        }

        LRESULT Handle(UINT message, WPARAM wParam, LPARAM lParam) {
            switch (message) {
            case WM_CREATE:
                CreateControls();
                return 0;
            case WM_SIZE:
                ResizeControls();
                return 0;
            case WM_COMMAND:
                if (LOWORD(wParam) == ID_BTN_TABLE && HIWORD(wParam) == BN_CLICKED) {
                    SetOutput(StatisticsVisualizer::getAccountTable(bank.getAccounts()), L"Показана таблица счетов.");
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_BALANCE && HIWORD(wParam) == BN_CLICKED) {
                    SetOutput(StatisticsVisualizer::getBalanceChart(bank.getAccounts()), L"Показан график балансов.");
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_TYPES && HIWORD(wParam) == BN_CLICKED) {
                    SetOutput(StatisticsVisualizer::getAccountTypeChart(bank.getAccounts()), L"Показана статистика типов счетов.");
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
