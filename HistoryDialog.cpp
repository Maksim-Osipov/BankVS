#include "HistoryDialog.h"

#include "DialogUtils.h"

#include <exception>

namespace {
    constexpr int ID_EDIT_ID = 6001;
    constexpr int ID_BTN_SHOW = 6002;
    constexpr int ID_EDIT_OUTPUT = 6003;
    constexpr int ID_BTN_CLOSE = 6004;

    const wchar_t* ClassName() {
        return L"BankVSHistoryDialog";
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
            hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, ClassName(), L"История операций",
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 760, 520,
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
            DialogUtils::CreateLabelAndEdit(hwnd, L"ID счёта:", ID_EDIT_ID, 20, 20, 90, 180);
            DialogUtils::CreateButton(hwnd, L"Показать историю", ID_BTN_SHOW, 320, 18, 160, 30);
            DialogUtils::CreateButton(hwnd, L"Закрыть", ID_BTN_CLOSE, 500, 18, 100, 30);
            outputEdit = DialogUtils::CreateOutputEdit(hwnd, ID_EDIT_OUTPUT, 20, 65, 700, 390);
        }

        void ResizeControls() {
            if (!outputEdit) {
                return;
            }

            RECT rect {};
            GetClientRect(hwnd, &rect);
            const int calculatedWidth = static_cast<int>(rect.right) - 40;
            const int calculatedHeight = static_cast<int>(rect.bottom) - 85;
            const int width = calculatedWidth > 300 ? calculatedWidth : 300;
            const int height = calculatedHeight > 180 ? calculatedHeight : 180;
            MoveWindow(outputEdit, 20, 65, width, height, TRUE);
        }

        void ShowHistory() {
            try {
                const int id = DialogUtils::ReadInt(hwnd, ID_EDIT_ID, L"ID счёта");
                DialogUtils::SetText(outputEdit, DialogUtils::NormalizeLineBreaks(DialogUtils::Utf8ToWide(bank.getAccountHistoryText(id))));
                outputCallback(L"История операций для счёта " + std::to_wstring(id) + L" показана.");
            } catch (const std::exception& ex) {
                DialogUtils::ShowException(hwnd, ex, outputCallback);
            }
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
                if (LOWORD(wParam) == ID_BTN_SHOW && HIWORD(wParam) == BN_CLICKED) {
                    ShowHistory();
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

void HistoryDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                         const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно истории операций.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
