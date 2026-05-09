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

#include "Bank.h"

#include <array>
#include <string>

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance, Bank& bank);

    bool Create(int nCmdShow);
    int Run();

    void AppendOutput(const std::wstring& text);
    void AppendOutputUtf8(const std::string& text);

private:
    HINSTANCE hInstance;
    Bank& bank;
    HWND hwnd;
    HWND outputEdit;
    HWND accountListView;
    HWND statusText;
    HWND statsGroup;
    HWND tableGroup;
    HWND logGroup;
    std::array<HWND, 7> statsLabels;
    bool closeAutoSaved;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    bool RegisterWindowClass();
    void CreateMainMenu();
    void CreateControls();
    void ResizeControls();
    void ProcessCommand(int commandId);
    void RefreshAccountTable();
    void RefreshStatisticsPanel();
    void SetStatus(const std::wstring& text);
    void ShowAboutDialog();
    void AutoSave();
    void AutoLoad();
};
