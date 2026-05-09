#include "MoneyOperationDialog.h"

#include "DialogUtils.h"

#include <exception>
#include <iomanip>
#include <sstream>

namespace {
    constexpr int ID_EDIT_ID = 3001;
    constexpr int ID_EDIT_AMOUNT = 3002;
    constexpr int ID_BTN_APPLY = 3003;
    constexpr int ID_BTN_CANCEL = 3004;

    const wchar_t* ClassName() {
        return L"BankVSMoneyOperationDialog";
    }

    std::wstring FormatAmount(double amount) {
        std::wostringstream oss;
        oss << std::fixed << std::setprecision(2) << amount;
        return oss.str();
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank, MoneyOperationType operationType,
               const std::function<void(const std::wstring&)>& outputCallback,
               int prefilledAccountId)
            : hInstance(hInstance), owner(owner), bank(bank), operationType(operationType),
              outputCallback(outputCallback), hwnd(nullptr), prefilledAccountId(prefilledAccountId) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(
                WS_EX_DLGMODALFRAME,
                ClassName(),
                operationType == MoneyOperationType::Deposit ? L"Пополнение счёта" : L"Снятие денег",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                430,
                190,
                owner,
                nullptr,
                hInstance,
                this);

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
        MoneyOperationType operationType;
        std::function<void(const std::wstring&)> outputCallback;
        HWND hwnd;
        int prefilledAccountId;

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
            DialogUtils::CreateLabelAndEdit(hwnd, L"ID счёта:", ID_EDIT_ID, 20, 24, 120, 240);
            DialogUtils::CreateLabelAndEdit(hwnd, L"Сумма:", ID_EDIT_AMOUNT, 20, 60, 120, 240);
            DialogUtils::CreateButton(hwnd,
                                      operationType == MoneyOperationType::Deposit ? L"Пополнить" : L"Снять",
                                      ID_BTN_APPLY, 150, 108, 120, 32);
            DialogUtils::CreateButton(hwnd, L"Отмена", ID_BTN_CANCEL, 285, 108, 95, 32);
            if (prefilledAccountId > 0) {
                SetWindowTextW(GetDlgItem(hwnd, ID_EDIT_ID), std::to_wstring(prefilledAccountId).c_str());
                SetFocus(GetDlgItem(hwnd, ID_EDIT_AMOUNT));
            } else {
                SetFocus(GetDlgItem(hwnd, ID_EDIT_ID));
            }
        }

        void Apply() {
            try {
                const int id = DialogUtils::ReadInt(hwnd, ID_EDIT_ID, L"ID счёта");
                const double amount = DialogUtils::ReadDouble(hwnd, ID_EDIT_AMOUNT, L"Сумма");

                if (operationType == MoneyOperationType::Deposit) {
                    bank.deposit(id, amount);
                    outputCallback(L"Счёт " + std::to_wstring(id) + L" пополнен на " + FormatAmount(amount) + L".");
                } else {
                    bank.withdraw(id, amount);
                    outputCallback(L"Со счёта " + std::to_wstring(id) + L" снято " + FormatAmount(amount) + L".");
                }

                DestroyWindow(hwnd);
            } catch (const std::exception& ex) {
                DialogUtils::ShowException(hwnd, ex, outputCallback);
            }
        }

        LRESULT Handle(UINT message, WPARAM wParam, LPARAM lParam) {
            switch (message) {
            case WM_CREATE:
                CreateControls();
                return 0;
            case WM_KEYDOWN:
                if (wParam == VK_RETURN) {
                    Apply();
                    return 0;
                }
                if (wParam == VK_ESCAPE) {
                    DestroyWindow(hwnd);
                    return 0;
                }
                break;
            case WM_COMMAND:
                if (LOWORD(wParam) == ID_BTN_APPLY && HIWORD(wParam) == BN_CLICKED) {
                    Apply();
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_CANCEL && HIWORD(wParam) == BN_CLICKED) {
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

void MoneyOperationDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank, MoneyOperationType operationType,
                                const std::function<void(const std::wstring&)>& outputCallback,
                                int prefilledAccountId) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, operationType, outputCallback, prefilledAccountId);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно операции со счётом.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
