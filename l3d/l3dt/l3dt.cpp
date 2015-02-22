// l3dt.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "l3dt.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];                    // the main window class name

// Forward declarations of functions included in this code module:
ATOM                            MyRegisterClass(HINSTANCE hInstance);
HWND                            InitInstance(HINSTANCE, int);
LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK        About(HWND, UINT, WPARAM, LPARAM);

#include "os.h"

int L3Start(const char* modelName);
void L3Tick();
void L3HandleInput(int);


void CALLBACK timerHandler(HWND h, UINT m, UINT_PTR id, DWORD t)
{
    L3Tick();
    Bitmap2D* pr = GetScreenBitmap2D();
    
    static BITMAPINFO* bmi = 0;
    if (!bmi)
    {
        size_t l = sizeof(BITMAPINFO) + sizeof(RGBQUAD)*256;
        bmi = (BITMAPINFO*)new char[l];
        memset(bmi, 0, l);

        BITMAPINFOHEADER* bh = &bmi->bmiHeader;
        bh->biSize = sizeof(BITMAPINFOHEADER);
        bh->biWidth = pr->w_;
        bh->biHeight = -pr->h_;
        bh->biPlanes = 1;
        bh->biBitCount = 8;
        bh->biCompression = BI_RGB;
        bh->biSizeImage = 0;
        bh->biXPelsPerMeter = 0;
        bh->biYPelsPerMeter = 0;
        bh->biClrUsed  = 0;
        bh->biClrImportant = 0;

        RGBQUAD* pc = (RGBQUAD*)((char*)bmi + bmi->bmiHeader.biSize);
        int i;
        int n = 16;
        for (i = 0; i < 256; ++i)
        {
            pc[i].rgbBlue = i;
        }

        int d = 256/n;
        int j;
        for (i = 0; i < 256; i += d)
        {
            int t = 0;
            for (j = 0; j < d; ++j) t += pc[i+j].rgbBlue;
            t /= d;
            for (j = 0; j < d; ++j) 
            {
                pc[i+j].rgbBlue = t;
                pc[i+j].rgbGreen = t;
                pc[i+j].rgbRed = t;
            }
        }
    }


    HDC dc = GetDC(h);
    if (dc)
    {
        int scale = 1;
        int v = StretchDIBits(dc,
                              0, 0, // XY dest
                              scale*pr->w_, scale*pr->h_,  // Dest W&H
                              0, 0, // XY src
                              pr->w_, pr->h_, 
                              pr->pix_, // bits
                              bmi,
                              DIB_RGB_COLORS,
                              SRCCOPY);

        ReleaseDC(h, dc);
    }
    clear_pix(pr, 0);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_L3DT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    HWND hWnd = InitInstance (hInstance, nCmdShow);
    if (!hWnd)
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_L3DT));

   
    //if (!L3Start("dumbell")) return 0;
    if (!L3Start("torus")) return 0;
   // if (!L3Start("teapot")) return 0;

    SetTimer(hWnd, 0, 50, timerHandler);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
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
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style                      = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc        = WndProc;
    wcex.cbClsExtra         = 0;
    wcex.cbWndExtra         = 0;
    wcex.hInstance          = hInstance;
    wcex.hIcon                      = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_L3DT));
    wcex.hCursor            = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground      = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName       = MAKEINTRESOURCE(IDC_L3DT);
    wcex.lpszClassName      = szWindowClass;
    wcex.hIconSm            = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
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
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
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
        break;
    case WM_CHAR:
        {
            int a = wParam & 0xff;
            L3HandleInput(a);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
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
