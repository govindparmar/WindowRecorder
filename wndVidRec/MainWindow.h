#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <Vfw.h>
#include <strsafe.h>
#include "macros.h"

BOOL CALLBACK EnumChildProc(_In_ HWND hWnd, _In_ LPARAM lParam);
VOID GenBitmap(_In_ HWND hWnd, _In_ BOOL first);
VOID WINAPI OnClose(_In_ HWND hWnd);
VOID WINAPI OnCommand(_In_ HWND hWnd, _In_ INT nID, _In_ HWND hwSource, _In_ UINT uNotify);
BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ LPCREATESTRUCTW lpCreateStruct);
VOID WINAPI OnDestroy(_In_ HWND hWnd);
VOID WINAPI OnPaint(_In_ HWND hWnd);
ATOM WINAPI RegisterWCEX(_In_ HINSTANCE hInstance);
VOID CALLBACK TimerProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ UINT_PTR idEvent, _In_ DWORD dwTime);
LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);
INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nShowCmd);

EXTERN CONST WCHAR g_wszClassName[];
EXTERN CONST WCHAR g_wszMutexName[];
EXTERN BOOL g_fFirstFrame;