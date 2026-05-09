#include "DialogUtils.h"

#include "Exceptions.h"

#include <algorithm>

namespace DialogUtils {
    std::wstring Utf8ToWide(const std::string& text) {
        if (text.empty()) {
            return L"";
        }

        const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        if (size <= 0) {
            return L"";
        }

        std::wstring result(static_cast<std::size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &result[0], size);
        result.resize(static_cast<std::size_t>(size - 1));
        return result;
    }

    std::string WideToUtf8(const std::wstring& text) {
        if (text.empty()) {
            return "";
        }

        const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0) {
            return "";
        }

        std::string result(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, &result[0], size, nullptr, nullptr);
        result.resize(static_cast<std::size_t>(size - 1));
        return result;
    }

    std::wstring NormalizeLineBreaks(const std::wstring& text) {
        std::wstring result;
        result.reserve(text.size() * 2);

        for (std::size_t i = 0; i < text.size(); ++i) {
            if (text[i] == L'\r') {
                result += L"\r\n";
                if (i + 1 < text.size() && text[i + 1] == L'\n') {
                    ++i;
                }
            } else if (text[i] == L'\n') {
                result += L"\r\n";
            } else {
                result += text[i];
            }
        }

        return result;
    }

    std::wstring GetText(HWND hwnd) {
        const int length = GetWindowTextLengthW(hwnd);
        if (length == 0) {
            return L"";
        }

        std::wstring text(static_cast<std::size_t>(length + 1), L'\0');
        GetWindowTextW(hwnd, &text[0], length + 1);
        text.resize(static_cast<std::size_t>(length));
        return text;
    }

    std::wstring GetControlText(HWND parent, int id) {
        return GetText(GetDlgItem(parent, id));
    }

    void SetText(HWND hwnd, const std::wstring& text) {
        SetWindowTextW(hwnd, text.c_str());
    }

    int ReadInt(HWND parent, int id, const wchar_t* fieldName) {
        const std::wstring text = GetControlText(parent, id);
        if (text.empty()) {
            throw InvalidAmountException(WideToUtf8(std::wstring(L"Поле \"") + fieldName + L"\" не заполнено."));
        }

        std::size_t processed = 0;
        int value = 0;
        try {
            value = std::stoi(text, &processed);
        } catch (...) {
            throw InvalidAmountException(WideToUtf8(std::wstring(L"Введите корректное целое число в поле \"") + fieldName + L"\"."));
        }

        if (processed != text.size()) {
            throw InvalidAmountException(WideToUtf8(std::wstring(L"Введите корректное целое число в поле \"") + fieldName + L"\"."));
        }

        return value;
    }

    double ReadDouble(HWND parent, int id, const wchar_t* fieldName) {
        std::wstring text = GetControlText(parent, id);
        std::replace(text.begin(), text.end(), L',', L'.');

        if (text.empty()) {
            throw InvalidAmountException(WideToUtf8(std::wstring(L"Поле \"") + fieldName + L"\" не заполнено."));
        }

        std::size_t processed = 0;
        double value = 0.0;
        try {
            value = std::stod(text, &processed);
        } catch (...) {
            throw InvalidAmountException(WideToUtf8(std::wstring(L"Введите корректное число в поле \"") + fieldName + L"\"."));
        }

        if (processed != text.size()) {
            throw InvalidAmountException(WideToUtf8(std::wstring(L"Введите корректное число в поле \"") + fieldName + L"\"."));
        }

        return value;
    }

    std::string ReadRequiredUtf8Text(HWND parent, int id, const wchar_t* fieldName) {
        const std::wstring text = GetControlText(parent, id);
        if (text.empty()) {
            throw InvalidAccountException(WideToUtf8(std::wstring(L"Поле \"") + fieldName + L"\" не заполнено."));
        }

        return WideToUtf8(text);
    }

    HFONT DefaultFont() {
        return static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    }

    static void ApplyDefaultFont(HWND hwnd) {
        if (hwnd) {
            SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(DefaultFont()), TRUE);
        }
    }

    HWND CreateControl(HWND parent, const wchar_t* className, const wchar_t* text, DWORD style,
                       int x, int y, int width, int height, int id) {
        HWND hwnd = CreateWindowW(
            className,
            text,
            WS_CHILD | WS_VISIBLE | style,
            x,
            y,
            width,
            height,
            parent,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            GetModuleHandleW(nullptr),
            nullptr);

        ApplyDefaultFont(hwnd);
        return hwnd;
    }

    HWND CreateControlEx(HWND parent, DWORD exStyle, const wchar_t* className, const wchar_t* text,
                         DWORD style, int x, int y, int width, int height, int id) {
        HWND hwnd = CreateWindowExW(
            exStyle,
            className,
            text,
            WS_CHILD | WS_VISIBLE | style,
            x,
            y,
            width,
            height,
            parent,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            GetModuleHandleW(nullptr),
            nullptr);

        ApplyDefaultFont(hwnd);
        return hwnd;
    }

    HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int width, int height) {
        return CreateControl(parent, L"STATIC", text, 0, x, y, width, height, 0);
    }

    HWND CreateEdit(HWND parent, int id, int x, int y, int width, int height) {
        return CreateControlEx(parent, WS_EX_CLIENTEDGE, L"EDIT", L"", ES_AUTOHSCROLL,
                               x, y, width, height, id);
    }

    HWND CreateButton(HWND parent, const wchar_t* text, int id, int x, int y, int width, int height) {
        return CreateControl(parent, L"BUTTON", text, BS_PUSHBUTTON,
                             x, y, width, height, id);
    }

    HWND CreateOutputEdit(HWND parent, int id, int x, int y, int width, int height) {
        return CreateControlEx(
            parent,
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            x,
            y,
            width,
            height,
            id);
    }

    void CreateLabelAndEdit(HWND parent, const wchar_t* label, int editId,
                            int x, int y, int labelWidth, int editWidth) {
        CreateLabel(parent, label, x, y + 4, labelWidth, 22);
        CreateEdit(parent, editId, x + labelWidth + 10, y, editWidth, 24);
    }

    void AppendOutput(HWND edit, const std::wstring& text) {
        if (!edit) {
            return;
        }

        const std::wstring normalized = NormalizeLineBreaks(text);
        const int length = GetWindowTextLengthW(edit);
        SendMessageW(edit, EM_SETSEL, length, length);
        SendMessageW(edit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(normalized.c_str()));
        SendMessageW(edit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L"\r\n"));
    }

    void AppendOutputUtf8(HWND edit, const std::string& text) {
        AppendOutput(edit, Utf8ToWide(text));
    }

    void CenterWindow(HWND hwnd, HWND owner) {
        RECT windowRect {};
        RECT ownerRect {};
        GetWindowRect(hwnd, &windowRect);

        if (owner) {
            GetWindowRect(owner, &ownerRect);
        } else {
            ownerRect.left = 0;
            ownerRect.top = 0;
            ownerRect.right = GetSystemMetrics(SM_CXSCREEN);
            ownerRect.bottom = GetSystemMetrics(SM_CYSCREEN);
        }

        const int width = windowRect.right - windowRect.left;
        const int height = windowRect.bottom - windowRect.top;
        const int x = ownerRect.left + ((ownerRect.right - ownerRect.left) - width) / 2;
        const int y = ownerRect.top + ((ownerRect.bottom - ownerRect.top) - height) / 2;
        SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    std::wstring BuildErrorMessage(const std::exception& ex) {
        std::wstring message = L"Ошибка: операция не выполнена.";

        if (dynamic_cast<const InvalidAccountException*>(&ex)) {
            message = L"Ошибка: счёт не найден или указан неверный ID.";
        } else if (dynamic_cast<const InvalidAmountException*>(&ex)) {
            message = L"Ошибка: введите корректную сумму или параметры счёта.";
        } else if (dynamic_cast<const InsufficientFundsException*>(&ex)) {
            message = L"Ошибка: недостаточно средств или превышен лимит.";
        }

        const std::wstring details = Utf8ToWide(ex.what());
        if (!details.empty()) {
            message += L"\r\n";
            message += details;
        }

        return message;
    }

    void ShowError(HWND hwnd, const std::wstring& message) {
        MessageBoxW(hwnd, message.c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
    }

    void ShowException(HWND hwnd, const std::exception& ex,
                       const std::function<void(const std::wstring&)>& outputCallback) {
        const std::wstring message = BuildErrorMessage(ex);
        if (outputCallback) {
            outputCallback(message);
        }
        ShowError(hwnd, message);
    }
}
