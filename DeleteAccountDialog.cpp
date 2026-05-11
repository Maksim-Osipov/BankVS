#include "DeleteAccountDialog.h"

#include "DialogUtils.h"

#include <exception>
#include <filesystem>
#include <string>

namespace {
    constexpr int ID_EDIT_ID = 10001;
    constexpr int ID_BTN_DELETE = 10002;
    constexpr int ID_BTN_CANCEL = 10003;

    const wchar_t* ClassName() {
        return L"BankVSDeleteAccountDialog";
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank,
               const std::function<void(const std::wstring&)>& outputCallback)
            : hInstance(hInstance), owner(owner), bank(bank), outputCallback(outputCallback), hwnd(nullptr) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(
                WS_EX_DLGMODALFRAME,
                ClassName(),
                L"Удаление счёта",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                430,
                165,
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
            DialogUtils::CreateLabelAndEdit(hwnd, L"ID счёта:", ID_EDIT_ID, 20, 24, 110, 240);
            DialogUtils::CreateButton(hwnd, L"Удалить", ID_BTN_DELETE, 145, 75, 120, 32);
            DialogUtils::CreateButton(hwnd, L"Отмена", ID_BTN_CANCEL, 280, 75, 100, 32);
            SetFocus(GetDlgItem(hwnd, ID_EDIT_ID));
        }

        void DeleteAccount() {
            try {
                const int id = DialogUtils::ReadInt(hwnd, ID_EDIT_ID, L"ID счёта");
                const std::wstring question = L"Вы действительно хотите удалить счёт " + std::to_wstring(id) + L"?";
                const int answer = MessageBoxW(hwnd, question.c_str(), L"Подтверждение удаления", MB_YESNO | MB_ICONQUESTION);
                if (answer != IDYES) {
                    return;
                }

                bank.removeAccount(id);

                bool statementDeleteFailed = false;
                try {
                    const std::filesystem::path statementPath =
                        std::filesystem::path("statements") /
                        ("statement_" + std::to_string(id) + ".txt");

                    if (std::filesystem::exists(statementPath) && !std::filesystem::remove(statementPath)) {
                        statementDeleteFailed = true;
                    }
                } catch (const std::exception&) {
                    statementDeleteFailed = true;
                }

                std::wstring message = L"Счёт " + std::to_wstring(id) + L" удалён.";
                if (statementDeleteFailed) {
                    message += L"\r\nСчёт удалён, но файл выписки удалить не удалось.";
                }

                outputCallback(message);
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
                if (LOWORD(wParam) == ID_BTN_DELETE && HIWORD(wParam) == BN_CLICKED) {
                    DeleteAccount();
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_CANCEL && HIWORD(wParam) == BN_CLICKED) {
                    DestroyWindow(hwnd);
                    return 0;
                }
                break;
            case WM_KEYDOWN:
                if (wParam == VK_RETURN) {
                    DeleteAccount();
                    return 0;
                }
                if (wParam == VK_ESCAPE) {
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

void DeleteAccountDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                               const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно удаления счёта.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
