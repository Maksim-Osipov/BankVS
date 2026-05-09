#include "StatementDialog.h"

#include "DialogUtils.h"

#include <exception>
#include <string>

namespace {
    constexpr int ID_EDIT_ID = 7001;
    constexpr int ID_BTN_GENERATE = 7002;
    constexpr int ID_BTN_CANCEL = 7003;

    const wchar_t* ClassName() {
        return L"BankVSStatementDialog";
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank,
               const std::function<void(const std::wstring&)>& outputCallback)
            : hInstance(hInstance), owner(owner), bank(bank), outputCallback(outputCallback), hwnd(nullptr) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, ClassName(), L"Выписка",
                                   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 430, 170,
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
            DialogUtils::CreateLabelAndEdit(hwnd, L"ID счёта:", ID_EDIT_ID, 20, 24, 120, 240);
            DialogUtils::CreateButton(hwnd, L"Сгенерировать выписку", ID_BTN_GENERATE, 145, 75, 180, 32);
            DialogUtils::CreateButton(hwnd, L"Отмена", ID_BTN_CANCEL, 335, 75, 70, 32);
        }

        void Generate() {
            try {
                const int id = DialogUtils::ReadInt(hwnd, ID_EDIT_ID, L"ID счёта");
                const std::string filename = "statement_" + std::to_string(id) + ".txt";
                bank.generateStatement(id, filename);
                outputCallback(L"Выписка успешно создана: statement_" + std::to_wstring(id) + L".txt.");
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
                if (LOWORD(wParam) == ID_BTN_GENERATE && HIWORD(wParam) == BN_CLICKED) {
                    Generate();
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

void StatementDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                           const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно выписки.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
