#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <Vfw.h>
#include <strsafe.h>
#include "aviCom.h"
#include "MainWindow.h"

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define IDT_TIMER1 1001

CONST WCHAR g_wszClassName[] = L"AVIRecorderWndClass2";
CONST WCHAR g_wszMutexName[] = L"AVIRecordMutexGP";
BOOL g_fFirstFrame = TRUE;
HWND hCapTargetWindow;
INT msInt = 0;
HAVI hAvi;

ATOM WINAPI RegisterWCEX(_In_ HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEXW));

	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.hbrBackground = (HBRUSH) COLOR_WINDOW;
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hIcon =
	wcex.hIconSm = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = WindowProc;
	wcex.lpszClassName = g_wszClassName;

	return RegisterClassExW(&wcex);
}

INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nShowCmd)
{
	HWND hWnd;
	MSG Msg;
	NONCLIENTMETRICSW ncm;
	HFONT hfDefault;

	ZeroMemory(&ncm, sizeof(NONCLIENTMETRICSW));
	ncm.cbSize = sizeof(NONCLIENTMETRICSW);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, FALSE);
	hfDefault = CreateFontIndirectW(&ncm.lfMessageFont);
	if (RegisterWCEX(hInstance) == (ATOM)0)
	{
		MessageBoxW(NULL, L"Window registration failed", L"Window recorder", MB_OK | MB_ICONSTOP);
		return -1;
	}
	hWnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, g_wszClassName, L"Window Recorder", WS_VISIBLE | WS_SYSMENU, 100, 100, 350, 260, NULL, NULL, hInstance, NULL);
	if (NULL == hWnd)
	{
		MessageBoxW(NULL, L"Window creation failed", L"Window recorder", MB_OK | MB_ICONSTOP);
		return -1;
	}

	ShowWindow(hWnd, nShowCmd);
	EnumChildWindows(hWnd, EnumChildProc, (LPARAM)&hfDefault);
	UpdateWindow(hWnd);


	while (GetMessageW(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessageW(&Msg);
	}

	return (INT) Msg.wParam;

}

VOID GenBitmap(_In_ HWND hWnd, _In_ BOOL first)
{
	BITMAPINFOHEADER biHeader;
	BITMAPINFO bInfo;
	HBITMAP hBitmap;
	HDC wDC, tempDC;
	INT xDir, yDir;
	INT bLength;
	BYTE *bmpBits;
	RECT wRect;
	
	ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
	ZeroMemory(&bInfo, sizeof(BITMAPINFO));

	GetWindowRect(hWnd, &wRect);
	yDir = wRect.bottom - wRect.top;
	xDir = wRect.right - wRect.left;
	
	bLength = ((((24 * xDir + 31)&(~31)) / 8)*yDir);

	biHeader.biSize = sizeof(biHeader);
	biHeader.biBitCount = 24;
	biHeader.biCompression = BI_RGB; 
	biHeader.biHeight = yDir; 
	biHeader.biWidth = xDir;
	biHeader.biPlanes = 1;

	bInfo.bmiHeader = biHeader;

	wDC = GetWindowDC(hWnd);
	tempDC = CreateCompatibleDC(wDC);
	hBitmap = CreateDIBSection(wDC, &bInfo, DIB_RGB_COLORS, (void**)&bmpBits, NULL, 0);
	if (NULL == hBitmap)
	{
		return;
	}
	SelectObject(tempDC, hBitmap);
	BitBlt(tempDC, 0, 0, xDir, yDir, wDC, 0, 0, SRCCOPY);
	if (first)
	{
		AVICOMPRESSOPTIONS acOpt;
		//HRESULT hr;
		ZeroMemory(&acOpt, sizeof(AVICOMPRESSOPTIONS));
		acOpt.fccHandler = mmioFOURCC('M', 'S', 'V', 'C');
		/*hr = */AVISetCompressionMode(hAvi, hBitmap, &acOpt, msInt);
		AVIAddFrame(hAvi, hBitmap, msInt);
		
	}
	else
	{
		/*HRESULT hr = */ AVIAddFrame(hAvi, hBitmap, msInt);
		
	}
	DeleteDC(tempDC);
	ReleaseDC(hWnd, wDC);
	DeleteObject(hBitmap);
}

VOID CALLBACK TimerProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ UINT_PTR idEvent, _In_ DWORD dwTime)
{
	GenBitmap(hCapTargetWindow, g_fFirstFrame);
	if (g_fFirstFrame)
	{
		g_fFirstFrame = FALSE;
	}
}

BOOL CALLBACK EnumChildProc(_In_ HWND hWnd, _In_ LPARAM lParam)
{
	HFONT hfDefault = *(HFONT *) lParam;
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(TRUE, 0));
	return TRUE;
}

BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ LPCREATESTRUCTW lpCreateStruct)
{
	INITCOMMONCONTROLSEX iccx;
	HINSTANCE hInstance = lpCreateStruct->hInstance;
	HWND hEditMS, hChooseWnd, hStart, hStop, hStatic, hStatic2;

	hStatic = CreateWindowW(L"Static", L"Resolution of capture in milliseconds: ", WS_VISIBLE | WS_CHILD | SS_LEFT, 10, 10, 330, 20, hWnd, NULL, hInstance, NULL);
	hEditMS = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"500", WS_VISIBLE | WS_CHILD | ES_NUMBER, 10, 30, 310, 20, hWnd, NULL, hInstance, NULL);
	hChooseWnd = CreateWindowW(L"Button", L"Select Target Window...", WS_VISIBLE | WS_CHILD | BS_TEXT | BS_PUSHBUTTON, 10, 90, 310, 30, hWnd, NULL, hInstance, NULL);
	hStart = CreateWindowW(L"Button", L"Start", WS_VISIBLE | WS_CHILD | BS_TEXT | BS_PUSHBUTTON | WS_DISABLED, 10, 120, 155, 30, hWnd, NULL, hInstance, NULL);
	hStop = CreateWindowW(L"Button", L"Stop", WS_VISIBLE | WS_CHILD | BS_TEXT | BS_PUSHBUTTON | WS_DISABLED, 165, 120, 155, 30, hWnd, NULL, hInstance, NULL);
	hStatic2 = CreateWindowW(L"Static", L"No window selected.", WS_VISIBLE | WS_CHILD | SS_LEFT, 10, 150, 310, 40, hWnd, NULL, hInstance, NULL);
	iccx.dwICC = ICC_STANDARD_CLASSES;
	iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&iccx);

	return TRUE;
}

VOID WINAPI OnCommand(_In_ HWND hWnd, _In_ INT nID, _In_ HWND hwSource, _In_ UINT uNotify)
{
	HWND hEditMS, hChooseWnd, hStart, hStop, hStatic, hStatic2;
	hEditMS = FindWindowExW(hWnd, NULL, L"Edit", NULL);
	hChooseWnd = FindWindowExW(hWnd, NULL, L"Button", NULL);
	hStart = FindWindowExW(hWnd, hChooseWnd, L"Button", NULL);
	hStop = FindWindowExW(hWnd, hStart, L"Button", NULL);
	hStatic = FindWindowExW(hWnd, NULL, L"Static", NULL);
	hStatic2 = FindWindowExW(hWnd, hStatic, L"Static", NULL);

	if (hwSource == hChooseWnd)
	{
		POINT p;
		RECT rect;
		WCHAR wszInfo[1024];
		WCHAR wszClass[64];

		MessageBoxW(
			NULL,
			L"After dismissing this message box, the target window will be chosen as the window under the mouse pointer after two seconds.  You may move this message box closer to the target window before closing it if need be.\n\nIf you are targeting a usual Windows application, hover the mouse over the *title bar* of the application.  If you are targeting a Java or Flash applet target anywhere within the applet.",
			L"Pre-Capture Information",
			MB_OK | MB_ICONINFORMATION);
		Sleep(2000);
		GetCursorPos(&p);
		hCapTargetWindow = WindowFromPoint(p);
		SetFocus(hCapTargetWindow);
		BringWindowToTop(hCapTargetWindow);
		GetWindowRect(hCapTargetWindow, &rect);
		GetClassNameW(hCapTargetWindow, wszClass, 64);
		StringCchPrintfW(wszInfo, 1024, L"Target: 0x%.8X \'%s\' (%d x %d)", (INT) hCapTargetWindow, wszClass, rect.right - rect.left, rect.bottom - rect.top);
		SetWindowTextW(hStatic2, wszInfo);
		EnableWindow(hStart, TRUE);
		EnableWindow(hStop, FALSE);


		
	}
	else if (hwSource == hStart)
	{
		HANDLE hHeap;
		WCHAR *eTxt, fName[48];
		INT msLen;
		SYSTEMTIME st;

		GetLocalTime(&st);
		hHeap = GetProcessHeap();
		EnableWindow(hStart, FALSE);
		EnableWindow(hStop, TRUE);
		EnableWindow(hChooseWnd, FALSE);
		msLen = GetWindowTextLengthW(hEditMS) + 1;
		eTxt = (WCHAR *) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (msLen + 1) * sizeof(WCHAR));
		if (NULL == eTxt)
		{
			ExitProcess(ERROR_OUTOFMEMORY);
		}
		GetWindowTextW(hEditMS, eTxt, msLen);
		if (swscanf_s(eTxt, L"%I32d", &msInt) != 1)
		{
			MessageBoxW(NULL, L"Invalid millisecond value (possibly too large)", L"Window Recorder", MB_OK | MB_ICONWARNING);
			HeapFree(hHeap, 0, eTxt);
			eTxt = NULL;

			return;
		}
		HeapFree(hHeap, 0, eTxt);
		eTxt = NULL;

		StringCchPrintfW(fName, 48, L"%.4hu-%.2hu-%.2hu %.2hu-%.2hu-%.2hu.avi", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		hAvi = CreateAVI(fName, 1);

		msInt = max(msInt, 1);
		SetTimer(hWnd, IDT_TIMER1, msInt, TimerProc);

	} 
	else if (hwSource == hStop)
	{
		EnableWindow(hStart, TRUE);
		EnableWindow(hStop, FALSE);
		EnableWindow(hChooseWnd, TRUE);
		//SendMessageW(hWnd, WM_USER + 1, 0, 0);
		KillTimer(hWnd, IDT_TIMER1);
		Sleep(10);
		CloseAVI(hAvi);
	}

	return;
}

VOID WINAPI OnClose(_In_ HWND hWnd)
{
	DestroyWindow(hWnd);
}

VOID WINAPI OnDestroy(_In_ HWND hWnd)
{
	PostQuitMessage(ERROR_SUCCESS);
}

VOID WINAPI OnPaint(_In_ HWND hWnd)
{
	PAINTSTRUCT ps;

	BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);

	return;
}

LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (Msg)
	{
		HANDLE_MSG(hWnd, WM_CLOSE, OnClose);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
	default:
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
	return 0;
}
