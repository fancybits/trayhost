#define NTDDI_VERSION 0x06000000
#include <stdio.h>
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

#define WM_MYMESSAGE (WM_USER + 1)

#define MAX_LOADSTRING 100

HINSTANCE hInst;
HWND hWnd;
HMENU hSubMenu;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
NOTIFYICONDATA nid;

ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int prevent_shutdown;
extern void tray_callback(int itemId);

void set_prevent_shutdown(int val)
{
    prevent_shutdown = val;
}

void add_separator_item()
{
    AppendMenuW(hSubMenu, MF_SEPARATOR, 0, NULL);
}

void add_menu_item(int id, const char* title, int disabled)
{
    UINT uFlags = MF_STRING;
    if (disabled == TRUE) {
        uFlags |= MF_GRAYED;
    }
    AppendMenuW(hSubMenu, uFlags, id, (wchar_t*)title);
}

void clear_menu_items()
{
    int count = GetMenuItemCount(hSubMenu);
    UINT i = 0;
    for (; i < count; i++) {
        // always remove at 0 because they shift every time
        RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
    }
}

void native_loop()
{
    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int init(const char * title, struct image img) {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    strcpy(szTitle, title);
    strcpy(szWindowClass, TEXT("TrayHostClass"));
    MyRegisterClass(hInstance);

    hWnd = InitInstance(hInstance, FALSE); // Don't show window
    if (!hWnd)
    {
        return -1;
    }

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 100;
    nid.uCallbackMessage = WM_MYMESSAGE;
    LoadIconMetric(hInstance, MAKEINTRESOURCEW(7), LIM_SMALL, &(nid.hIcon));

    strcpy(nid.szTip, title); // MinGW seems to use ANSI
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);

    hSubMenu = CreatePopupMenu();
    return 0;
}

void exit_loop() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
    PostQuitMessage(0);
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(7));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(7));

    return RegisterClassEx(&wcex);
}

HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance;

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd)
    {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return hWnd;
}

void ShowMenu(HWND hWnd)
{
    POINT p;
    GetCursorPos(&p);
    SetForegroundWindow(hWnd); // Win32 bug work-around
    TrackPopupMenu(hSubMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_QUERYENDSESSION:
            if (prevent_shutdown)
                return FALSE;
            else
                return DefWindowProc(hWnd, message, wParam, lParam);
            break;
        case WM_COMMAND:
            tray_callback(LOWORD(wParam));
            break;
        case WM_CREATE:
            ShutdownBlockReasonCreate(hWnd, L"The recording engine is busy.");
            break;
        case WM_DESTROY:
            ShutdownBlockReasonDestroy(hWnd);
            exit_loop();
            break;
        case WM_MYMESSAGE:
            switch(lParam)
            {
                case WM_RBUTTONUP:
                    ShowMenu(hWnd);
                    break;
                case WM_LBUTTONUP:
                    tray_callback(-1);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            };
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void set_clipboard_string(const char * string) {
    // TODO: Implement.
}

struct clipboard_content get_clipboard_content() {
    // TODO: Implement.
    struct clipboard_content cc;
    return cc;
}

void display_notification(int notificationId, const char * title, const char * body, struct image img, double timeout) {
    // TODO: Implement.
}
