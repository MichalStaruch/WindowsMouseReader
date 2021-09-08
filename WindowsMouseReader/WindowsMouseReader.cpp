// WindowsMouseReader.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsMouseReader.h"
#include <string>
#include <vector>

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


const UINT DISPLAY_REFRESH_RATE = 60;
const UINT_PTR IDT_REDRAW_TIMER = 666;
const UINT REDRAW_INTERVAL = 1000 / DISPLAY_REFRESH_RATE;
const COLORREF COLOR_BLACK = 0x00000000;
const std::string TEXT_HELP = "LMB: paint, RMB: points / lines, SPACE: reset. WM_MOUSEMOVE counter: ";

ULONG mouseMoveCounter = 0;
BOOL paintActive = FALSE;
BOOL linesMode = FALSE;
POINT last = { 0, 0 };
std::vector<POINT> points(0);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSMOUSEREADER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSMOUSEREADER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSMOUSEREADER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSMOUSEREADER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (hWnd == NULL)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   SetTimer(hWnd, IDT_REDRAW_TIMER, REDRAW_INTERVAL, (TIMERPROC) NULL);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_MOUSEMOVE:
        {
            mouseMoveCounter++;
            int x = (int)(short) LOWORD(lParam);
            int y = (int)(short) HIWORD(lParam);
            POINT point = { x, y };
            points.push_back(point);
        }
        break;
    case WM_RBUTTONDOWN:
        linesMode = !linesMode;
        break;
    case WM_LBUTTONDOWN:
        paintActive = TRUE;
        break;
    case WM_LBUTTONUP:
        paintActive = FALSE;
        break;
    case WM_KEYDOWN:
        {
            if (wParam == VK_SPACE)
            {
                mouseMoveCounter = 0;
                RECT rect;
                GetClientRect(hWnd, &rect);
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        break;
    case WM_SIZE:
        {
            mouseMoveCounter = 0;
            RECT rect;
            GetClientRect(hWnd, &rect);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    case WM_TIMER:
        switch (wParam)
        {
            case IDT_REDRAW_TIMER:
            {
                RECT rect;
                GetClientRect(hWnd, &rect);
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
        }
        break;
    case WM_PAINT:
        {
            RECT rect;
            GetClientRect(hWnd, &rect);

            std::string text = TEXT_HELP + std::to_string(mouseMoveCounter);
            LPCSTR winText = text.c_str();

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            DrawTextA(hdc, winText, -1, &rect, DT_LEFT | DT_TOP);
            if (paintActive) {
                if (linesMode) {
                    MoveToEx(hdc, last.x, last.y, NULL);
                    for (UINT i = 0; i < points.size(); i++) {
                        POINT point = points.at(i);
                        LineTo(hdc, point.x, point.y);
                        MoveToEx(hdc, point.x, point.y, NULL);
                    }
                } else {
                    for (UINT i = 0; i < points.size(); i++) {
                        POINT point = points.at(i);
                        SetPixel(hdc, point.x, point.y, COLOR_BLACK);
                    }
                }
            }
            EndPaint(hWnd, &ps);

            if (!points.empty()) {
                last = points.back();
            }
            points.clear();
        }
        break;
    case WM_DESTROY:
        KillTimer(hWnd, IDT_REDRAW_TIMER);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
