#include "CreateAccountDialog.h"

#include "DialogUtils.h"

#include <exception>

namespace {
    constexpr int ID_COMBO_TYPE = 2001;
    constexpr int ID_EDIT_ID = 2002;
    constexpr int ID_EDIT_OWNER = 2003;
    constexpr int ID_EDIT_BALANCE = 2004;
    constexpr int ID_EDIT_RATE = 2005;
    constexpr int ID_EDIT_OVERDRAFT = 2006;
    constexpr int ID_EDIT_CREDIT = 2007;
    constexpr int ID_BTN_CREATE = 2008;
    constexpr int ID_BTN_CANCEL = 2009;

    const wchar_t* ClassName() {
        return L"BankVSCreateAccountDialog";
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
                L"Создание счёта",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                510,
                355,
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
            DialogUtils::CreateLabel(hwnd, L"Тип счёта:", 20, 22, 155, 22);
            HWND combo = DialogUtils::CreateControl(hwnd, L"COMBOBOX", L"",
                                                    CBS_DROPDOWNLIST | WS_TABSTOP,
                                                    185, 20, 280, 200, ID_COMBO_TYPE);
            SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Накопительный"));
            SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Расчётный"));
            SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Кредитный"));
            SendMessageW(combo, CB_SETCURSEL, 0, 0);

            DialogUtils::CreateLabelAndEdit(hwnd, L"ID:", ID_EDIT_ID, 20, 60, 155, 280);
            DialogUtils::CreateLabelAndEdit(hwnd, L"ФИО владельца:", ID_EDIT_OWNER, 20, 92, 155, 280);
            DialogUtils::CreateLabelAndEdit(hwnd, L"Начальный баланс:", ID_EDIT_BALANCE, 20, 124, 155, 280);
            DialogUtils::CreateLabelAndEdit(hwnd, L"Процентная ставка:", ID_EDIT_RATE, 20, 156, 155, 280);
            DialogUtils::CreateLabelAndEdit(hwnd, L"Лимит овердрафта:", ID_EDIT_OVERDRAFT, 20, 188, 155, 280);
            DialogUtils::CreateLabelAndEdit(hwnd, L"Кредитный лимит:", ID_EDIT_CREDIT, 20, 220, 155, 280);

            DialogUtils::CreateButton(hwnd, L"Создать", ID_BTN_CREATE, 185, 265, 130, 32);
            DialogUtils::CreateButton(hwnd, L"Отмена", ID_BTN_CANCEL, 335, 265, 130, 32);
            UpdateFieldAvailability();
        }

        int SelectedType() const {
            HWND combo = GetDlgItem(hwnd, ID_COMBO_TYPE);
            return static_cast<int>(SendMessageW(combo, CB_GETCURSEL, 0, 0));
        }

        void UpdateFieldAvailability() {
            const int type = SelectedType();
            EnableWindow(GetDlgItem(hwnd, ID_EDIT_RATE), type == 0 || type == 2);
            EnableWindow(GetDlgItem(hwnd, ID_EDIT_OVERDRAFT), type == 1);
            EnableWindow(GetDlgItem(hwnd, ID_EDIT_CREDIT), type == 2);
        }

        void CreateAccount() {
            try {
                const int type = SelectedType();
                const int id = DialogUtils::ReadInt(hwnd, ID_EDIT_ID, L"ID");
                const std::string ownerName = DialogUtils::ReadRequiredUtf8Text(hwnd, ID_EDIT_OWNER, L"ФИО владельца");
                const double balance = DialogUtils::ReadDouble(hwnd, ID_EDIT_BALANCE, L"Начальный баланс");

                if (type == 0) {
                    const double rate = DialogUtils::ReadDouble(hwnd, ID_EDIT_RATE, L"Процентная ставка");
                    bank.createSavingsAccount(id, ownerName, balance, rate);
                    outputCallback(L"Накопительный счёт успешно создан. ID: " + std::to_wstring(id) + L".");
                } else if (type == 1) {
                    const double overdraft = DialogUtils::ReadDouble(hwnd, ID_EDIT_OVERDRAFT, L"Лимит овердрафта");
                    bank.createCheckingAccount(id, ownerName, balance, overdraft);
                    outputCallback(L"Расчётный счёт успешно создан. ID: " + std::to_wstring(id) + L".");
                } else {
                    const double creditLimit = DialogUtils::ReadDouble(hwnd, ID_EDIT_CREDIT, L"Кредитный лимит");
                    const double rate = DialogUtils::ReadDouble(hwnd, ID_EDIT_RATE, L"Процентная ставка");
                    bank.createCreditAccount(id, ownerName, balance, creditLimit, rate);
                    outputCallback(L"Кредитный счёт успешно создан. ID: " + std::to_wstring(id) + L".");
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
            case WM_COMMAND:
                if (LOWORD(wParam) == ID_COMBO_TYPE && HIWORD(wParam) == CBN_SELCHANGE) {
                    UpdateFieldAvailability();
                    return 0;
                }
                if (LOWORD(wParam) == ID_BTN_CREATE && HIWORD(wParam) == BN_CLICKED) {
                    CreateAccount();
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

void CreateAccountDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                               const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно создания счёта.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
