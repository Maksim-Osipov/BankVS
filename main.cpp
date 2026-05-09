#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>

#include "Bank.h"
#include "MainWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    Bank bank;
    MainWindow mainWindow(hInstance, bank);

    if (!mainWindow.Create(nCmdShow)) {
        return 1;
    }

    return mainWindow.Run();
}
