#include <Windows.h>
#include <Vfw.h>
#include <tchar.h>
#include <strsafe.h>
#include "aviCom.h"
#pragma comment(lib, "Vfw32.lib")

#define IDT_TIMER1 1001

VOID CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
VOID GenBitmap(HWND hWnd, BOOL first);

WCHAR szClassName[] = L"AVIRecorderWndClass2";
HWND hEditMS, hCompressChk, hAutoStopChk, hChooseWnd, hStart, hStop, hStatic, hStatic2;
HWND hCapTargetWindow;
INT msInt = 0;

INT runCount = 0;

HAVI hAvi;

INT APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd)
{
	WNDCLASSEXW wc;
	MSG Msg;
	HWND hWnd;
	
	ZeroMemory(&wc, sizeof(WNDCLASSEXW));

	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hIcon = 
	wc.hIconSm = LoadIconW(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = szClassName;
	wc.lpszMenuName = NULL;

	RegisterClassExW(&wc);
	
	hWnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, szClassName, L"Window Recorder", WS_VISIBLE | WS_SYSMENU, 100, 100, 350, 260, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, SW_SHOW);
	EnumChildWindows(hWnd, EnumChildProc, 0);
	UpdateWindow(hWnd);


	while (GetMessageW(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessageW(&Msg);
	}

	return (INT)Msg.wParam;

}

VOID GenBitmap(HWND hWnd, BOOL first)
{
	BITMAPINFOHEADER biHeader;
	BITMAPINFO bInfo;
	HBITMAP hBitmap;
	HDC wDC, tempDC;
	INT xDir, yDir;
	INT bLength;
	BYTE *bmpBits;
	RECT wRect;
	
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
	biHeader.biSizeImage = bLength; 
	biHeader.biXPelsPerMeter = xDir*50;
	biHeader.biYPelsPerMeter = yDir*50;
	biHeader.biClrImportant = 0;
	biHeader.biClrUsed = 0;

	bInfo.bmiHeader = biHeader;

	wDC = GetWindowDC(hWnd);
	tempDC = CreateCompatibleDC(wDC);
	hBitmap = CreateDIBSection(wDC, &bInfo, DIB_RGB_COLORS, (void**)&bmpBits, NULL, 0);
	SelectObject(tempDC, hBitmap);
	BitBlt(tempDC, 0, 0, xDir, yDir, wDC, 0, 0, SRCCOPY);
	if (first)
	{
		AVICOMPRESSOPTIONS acOpt;
		HRESULT hr;
		SecureZeroMemory(&acOpt, sizeof(AVICOMPRESSOPTIONS));
		acOpt.fccHandler = mmioFOURCC('C', 'V', 'I', 'D');
		hr = AVISetCompressionMode(hAvi, hBitmap, &acOpt, msInt);
		
	}
	else
	{
		HRESULT hr = AVIAddFrame(hAvi, hBitmap, msInt);
		
	}
	DeleteDC(tempDC);
	ReleaseDC(hWnd, wDC);
	DeleteObject(hBitmap);
}

VOID CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (runCount++ == 0) GenBitmap(hCapTargetWindow, TRUE);
	else GenBitmap(hCapTargetWindow, FALSE);
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(TRUE, 0));
	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	WCHAR wszInfo[1024];
	HINSTANCE thisInstance = GetModuleHandleW(NULL);
	switch (Msg)
	{
	case (WM_USER + 1):
		KillTimer(hWnd, IDT_TIMER1);
		Sleep(10);
		CloseAVI(hAvi);
		break;
	case WM_CREATE:
		hStatic = CreateWindowW(L"Static", L"Resolution of capture in milliseconds: ", WS_VISIBLE | WS_CHILD | SS_LEFT, 10, 10, 330, 20, hWnd, NULL, thisInstance, NULL);
		hEditMS = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"500", WS_VISIBLE | WS_CHILD | ES_NUMBER, 10, 30, 310, 20, hWnd, NULL, thisInstance, NULL);
		hChooseWnd = CreateWindowW(L"Button", L"Select Target Window...", WS_VISIBLE | WS_CHILD | BS_TEXT | BS_PUSHBUTTON, 10, 90, 310, 30, hWnd, NULL, thisInstance, NULL);
		hStart = CreateWindowW(L"Button", L"Start", WS_VISIBLE | WS_CHILD | BS_TEXT | BS_PUSHBUTTON | WS_DISABLED, 10, 120, 155, 30, hWnd, NULL, thisInstance, NULL);
		hStop = CreateWindowW(L"Button", L"Stop", WS_VISIBLE | WS_CHILD | BS_TEXT | BS_PUSHBUTTON | WS_DISABLED, 165, 120, 155, 30, hWnd, NULL, thisInstance, NULL);
		hStatic2 = CreateWindowW(L"Static", L"No window selected.", WS_VISIBLE | WS_CHILD | SS_LEFT , 10, 150, 310, 40, hWnd, NULL, thisInstance, NULL);
		break;
	case WM_COMMAND:
		if ((HWND)lParam == hChooseWnd)
		{
			POINT p;
			RECT rect;
			
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
			StringCchPrintfW(wszInfo, 1024, L"Target: 0x%.8X \'%s\' (%d x %d)", (INT)hCapTargetWindow, wszClass, rect.right - rect.left, rect.bottom - rect.top);
			SetWindowTextW(hStatic2, wszInfo);
			EnableWindow(hStart, TRUE);
			EnableWindow(hStop, FALSE);
			
			

		}
		else if ((HWND)lParam == hStart)
		{
			HANDLE hHeap;
			WCHAR *eTxt, fName[48];
			INT msLen;
			SYSTEMTIME st;
			GetSystemTime(&st);
			hHeap = GetProcessHeap();
			EnableWindow(hStart, FALSE);
			EnableWindow(hStop, TRUE);
			EnableWindow(hChooseWnd, FALSE);
			msLen = GetWindowTextLengthW(hEditMS);
			eTxt = (WCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, msLen + 1);
			swscanf_s(eTxt, L"%d", &msInt);
			if (msInt < 1) msInt = 1;
			HeapFree(hHeap, 0, eTxt);
			CloseHandle(hHeap);
			
			StringCchPrintfW(fName, 48, L"%hu-%hu-%hu %hu-%hu-%hu-%hu.avi", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			hAvi = CreateAVI(fName, 1);
			
			SetTimer(hWnd, IDT_TIMER1, msInt, TimerProc);
			
		}
		else if ((HWND)lParam == hStop)
		{
			EnableWindow(hStart, TRUE);
			EnableWindow(hStop, FALSE);
			EnableWindow(hChooseWnd, TRUE);
			SendMessageW(hWnd, WM_USER + 1, 0, 0);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
	return 0;
}
