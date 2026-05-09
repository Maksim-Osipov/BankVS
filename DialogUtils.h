#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <exception>
#include <functional>
#include <string>

namespace DialogUtils {
    std::wstring Utf8ToWide(const std::string& text);
    std::string WideToUtf8(const std::wstring& text);
    std::wstring NormalizeLineBreaks(const std::wstring& text);

    std::wstring GetText(HWND hwnd);
    std::wstring GetControlText(HWND parent, int id);
    void SetText(HWND hwnd, const std::wstring& text);

    int ReadInt(HWND parent, int id, const wchar_t* fieldName);
    double ReadDouble(HWND parent, int id, const wchar_t* fieldName);
    double ParsePercentInput(const std::wstring& text);
    std::string ReadRequiredUtf8Text(HWND parent, int id, const wchar_t* fieldName);

    HFONT DefaultFont();
    HWND CreateControl(HWND parent, const wchar_t* className, const wchar_t* text, DWORD style,
                       int x, int y, int width, int height, int id);
    HWND CreateControlEx(HWND parent, DWORD exStyle, const wchar_t* className, const wchar_t* text,
                         DWORD style, int x, int y, int width, int height, int id);
    HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int width, int height);
    HWND CreateEdit(HWND parent, int id, int x, int y, int width, int height);
    HWND CreateButton(HWND parent, const wchar_t* text, int id, int x, int y, int width, int height);
    HWND CreateOutputEdit(HWND parent, int id, int x, int y, int width, int height);
    void CreateLabelAndEdit(HWND parent, const wchar_t* label, int editId,
                            int x, int y, int labelWidth, int editWidth);

    void AppendOutput(HWND edit, const std::wstring& text);
    void AppendOutputUtf8(HWND edit, const std::string& text);
    void CenterWindow(HWND hwnd, HWND owner);

    std::wstring BuildErrorMessage(const std::exception& ex);
    void ShowError(HWND hwnd, const std::wstring& message);
    void ShowException(HWND hwnd, const std::exception& ex,
                       const std::function<void(const std::wstring&)>& outputCallback);
}
