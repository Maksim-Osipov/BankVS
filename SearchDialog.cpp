#include "SearchDialog.h"

#include "DialogUtils.h"
#include "Exceptions.h"
#include "UiHelpers.h"

#include <exception>
#include <sstream>
#include <stdexcept>

namespace {
    constexpr int ID_EDIT_QUERY = 9001;
    constexpr int ID_RADIO_ID = 9002;
    constexpr int ID_RADIO_OWNER = 9003;
    constexpr int ID_RADIO_TYPE = 9004;
    constexpr int ID_BTN_SEARCH = 9005;
    constexpr int ID_BTN_CLOSE = 9006;
    constexpr int ID_EDIT_RESULTS = 9007;

    const wchar_t* ClassName() {
        return L"BankVSSearchDialog";
    }

    std::wstring NormalizeTypeQuery(const std::wstring& text) {
        const std::wstring lowered = UiHelpers::ToLower(text);
        if (lowered == L"savings" || lowered == L"накопительный") {
            return L"savings";
        }
        if (lowered == L"checking" || lowered == L"расчётный" || lowered == L"расчетный") {
            return L"checking";
        }
        if (lowered == L"credit" || lowered == L"кредитный") {
            return L"credit";
        }

        return lowered;
    }

    class Dialog {
    public:
        Dialog(HINSTANCE hInstance, HWND owner, Bank& bank,
               const std::function<void(const std::wstring&)>& outputCallback)
            : hInstance(hInstance), owner(owner), bank(bank), outputCallback(outputCallback),
              hwnd(nullptr), resultsEdit(nullptr) {
        }

        bool Create() {
            Register();
            hwnd = CreateWindowExW(
                WS_EX_DLGMODALFRAME,
                ClassName(),
                L"Поиск счёта",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                760,
                520,
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
        HWND resultsEdit;

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
            DialogUtils::CreateLabelAndEdit(hwnd, L"Что искать:", ID_EDIT_QUERY, 20, 20, 100, 360);
            DialogUtils::CreateControl(hwnd, L"BUTTON", L"по ID", BS_AUTORADIOBUTTON | WS_GROUP, 20, 62, 90, 24, ID_RADIO_ID);
            DialogUtils::CreateControl(hwnd, L"BUTTON", L"по ФИО владельца", BS_AUTORADIOBUTTON, 120, 62, 160, 24, ID_RADIO_OWNER);
            DialogUtils::CreateControl(hwnd, L"BUTTON", L"по типу счёта", BS_AUTORADIOBUTTON, 290, 62, 140, 24, ID_RADIO_TYPE);
            SendMessageW(GetDlgItem(hwnd, ID_RADIO_ID), BM_SETCHECK, BST_CHECKED, 0);

            DialogUtils::CreateButton(hwnd, L"Найти", ID_BTN_SEARCH, 455, 20, 110, 32);
            DialogUtils::CreateButton(hwnd, L"Закрыть", ID_BTN_CLOSE, 580, 20, 110, 32);
            resultsEdit = DialogUtils::CreateOutputEdit(hwnd, ID_EDIT_RESULTS, 20, 100, 700, 360);
        }

        void ResizeControls() {
            if (!resultsEdit) {
                return;
            }

            RECT rect {};
            GetClientRect(hwnd, &rect);
            const int width = static_cast<int>(rect.right) - 40;
            const int height = static_cast<int>(rect.bottom) - 120;
            MoveWindow(resultsEdit, 20, 100, width > 300 ? width : 300, height > 160 ? height : 160, TRUE);
        }

        int CheckedMode() const {
            if (SendMessageW(GetDlgItem(hwnd, ID_RADIO_OWNER), BM_GETCHECK, 0, 0) == BST_CHECKED) {
                return ID_RADIO_OWNER;
            }
            if (SendMessageW(GetDlgItem(hwnd, ID_RADIO_TYPE), BM_GETCHECK, 0, 0) == BST_CHECKED) {
                return ID_RADIO_TYPE;
            }

            return ID_RADIO_ID;
        }

        void Search() {
            try {
                const std::wstring query = DialogUtils::GetControlText(hwnd, ID_EDIT_QUERY);
                if (query.empty()) {
                    DialogUtils::SetText(resultsEdit, L"Счета не найдены.");
                    outputCallback(L"Поиск не выполнен: пустой запрос.");
                    return;
                }

                std::wostringstream result;
                int count = 0;
                const int mode = CheckedMode();

                if (mode == ID_RADIO_ID) {
                    std::size_t processed = 0;
                    const int id = std::stoi(query, &processed);
                    if (processed != query.size()) {
                        throw InvalidAmountException(DialogUtils::WideToUtf8(L"Ошибка: введите корректный ID счёта."));
                    }

                    auto account = bank.getAccount(id);
                    result << UiHelpers::GetAccountSummaryText(account) << L"\r\n";
                    count = 1;
                } else if (mode == ID_RADIO_OWNER) {
                    const std::wstring loweredQuery = UiHelpers::ToLower(query);
                    for (const auto& pair : bank.getAccounts()) {
                        const std::wstring ownerName = UiHelpers::ToLower(DialogUtils::Utf8ToWide(pair.second->getOwnerName()));
                        if (ownerName.find(loweredQuery) != std::wstring::npos) {
                            result << UiHelpers::GetAccountSummaryText(pair.second) << L"\r\n";
                            ++count;
                        }
                    }
                } else {
                    const std::wstring typeQuery = NormalizeTypeQuery(query);
                    for (const auto& pair : bank.getAccounts()) {
                        if (NormalizeTypeQuery(DialogUtils::Utf8ToWide(pair.second->getType())) == typeQuery ||
                            NormalizeTypeQuery(UiHelpers::AccountTypeToRussian(pair.second->getType())) == typeQuery) {
                            result << UiHelpers::GetAccountSummaryText(pair.second) << L"\r\n";
                            ++count;
                        }
                    }
                }

                if (count == 0) {
                    DialogUtils::SetText(resultsEdit, L"Счета не найдены.");
                    outputCallback(L"Счета не найдены.");
                } else {
                    DialogUtils::SetText(resultsEdit, result.str());
                    outputCallback(L"Поиск выполнен. Найдено счетов: " + std::to_wstring(count) + L".");
                }
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
                if (LOWORD(wParam) == ID_BTN_SEARCH && HIWORD(wParam) == BN_CLICKED) {
                    Search();
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

void SearchDialog::Show(HINSTANCE hInstance, HWND owner, Bank& bank,
                        const std::function<void(const std::wstring&)>& outputCallback) {
    Dialog* dialog = new Dialog(hInstance, owner, bank, outputCallback);
    if (!dialog->Create()) {
        delete dialog;
        MessageBoxW(owner, L"Не удалось открыть окно поиска счёта.", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}
