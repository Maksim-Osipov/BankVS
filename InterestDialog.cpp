#include "InterestDialog.h"

#include "DialogUtils.h"

#include <exception>

namespace {
    constexpr int ID_EDIT_ID = 5001;
    constexpr int ID_BTN_ONE = 5002;
    constexpr int ID_BTN_ALL = 5003;
    constexpr int ID_BTN_CANCEL = 5004;

    const wchar_t* ClassName() {
        return L"BankVSInterestDialog";
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank,
               const std::function<void(const std::wstring&)>& outputCallback)
            : hInstance(hInstance), owner(owner), bank(bank), outputCallback(outputCallback), hwnd(nullptr) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, ClassName(), L"Начисление процентов",
                                   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 470, 205,
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
            DialogUtils::CreateLabelAndEdit(hwnd, L"ID счёта:", ID_EDIT_ID, 20, 24, 110, 300);
            DialogUtils::CreateButton(hwnd, L"Начислить по одному счёту", ID_BTN_ONE, 130, 72, 210, 32);
            DialogUtils::CreateButton(hwnd, L"Начислить всем счетам", ID_BTN_ALL, 130, 110, 210, 32);
            DialogUtils::CreateButton(hwnd, L"Отмена", ID_BTN_CANCEL, 355, 110, 80, 32);
        }

        void ApplyOne() {
            try {
                const int id = DialogUtils::ReadInt(hwnd, ID_EDIT_ID, L"ID счёта");
                bank.applyInterestToAccount(id);
                outputCallback(L"Проценты успешно начислены по счёту " + std::to_wstring(id) + L".");
                DestroyWindow(hwnd);
            } catch (const std::exception& ex) {
                DialogUtils::ShowException(hwnd, ex, outputCallback);
            }
        }

        void ApplyAll() {
            try {
                bank.applyInterestToAllAccounts();
                outputCallback(L"Проценты успешно начислены всем счетам.");
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
                if (LOWORD(wParam) == ID_BTN_ONE && HIWORD(wParam) == BN_CLICKED) {
                    ApplyOne();
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_ALL && HIWORD(wParam) == BN_CLICKED) {
                    ApplyAll();
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

void InterestDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                          const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно начисления процентов.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
