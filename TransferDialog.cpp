#include "TransferDialog.h"

#include "DialogUtils.h"

#include <exception>
#include <iomanip>
#include <sstream>

namespace {
    constexpr int ID_EDIT_FROM = 4001;
    constexpr int ID_EDIT_TO = 4002;
    constexpr int ID_EDIT_AMOUNT = 4003;
    constexpr int ID_BTN_TRANSFER = 4004;
    constexpr int ID_BTN_CANCEL = 4005;

    const wchar_t* ClassName() {
        return L"BankVSTransferDialog";
    }

    std::wstring FormatAmount(double amount) {
        std::wostringstream oss;
        oss << std::fixed << std::setprecision(2) << amount;
        return oss.str();
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank,
               const std::function<void(const std::wstring&)>& outputCallback)
            : hInstance(hInstance), owner(owner), bank(bank), outputCallback(outputCallback), hwnd(nullptr) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, ClassName(), L"Перевод денег",
                                   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 470, 225,
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
            DialogUtils::CreateLabelAndEdit(hwnd, L"ID счёта отправителя:", ID_EDIT_FROM, 20, 24, 180, 220);
            DialogUtils::CreateLabelAndEdit(hwnd, L"ID счёта получателя:", ID_EDIT_TO, 20, 60, 180, 220);
            DialogUtils::CreateLabelAndEdit(hwnd, L"Сумма:", ID_EDIT_AMOUNT, 20, 96, 180, 220);
            DialogUtils::CreateButton(hwnd, L"Перевести", ID_BTN_TRANSFER, 200, 145, 120, 32);
            DialogUtils::CreateButton(hwnd, L"Отмена", ID_BTN_CANCEL, 335, 145, 95, 32);
        }

        void Transfer() {
            try {
                const int fromId = DialogUtils::ReadInt(hwnd, ID_EDIT_FROM, L"ID счёта отправителя");
                const int toId = DialogUtils::ReadInt(hwnd, ID_EDIT_TO, L"ID счёта получателя");
                const double amount = DialogUtils::ReadDouble(hwnd, ID_EDIT_AMOUNT, L"Сумма");

                bank.transfer(fromId, toId, amount);
                outputCallback(L"Перевод " + FormatAmount(amount) + L" со счёта " +
                               std::to_wstring(fromId) + L" на счёт " + std::to_wstring(toId) +
                               L" успешно выполнен.");
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
            case WM_COMMAND:
                if (LOWORD(wParam) == ID_BTN_TRANSFER && HIWORD(wParam) == BN_CLICKED) {
                    Transfer();
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

void TransferDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                          const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно перевода денег.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
