#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <Vfw.h>
#include <strsafe.h>

#define EXTERN extern

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
VOID GenBitmap(HWND hWnd, BOOL first);
VOID WINAPI OnClose(_In_ HWND hWnd);
VOID WINAPI OnCommand(_In_ HWND hWnd, _In_ INT nID, _In_ HWND hwSource, _In_ UINT uNotify);
BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ LPCREATESTRUCTW lpCreateStruct);
VOID WINAPI OnDestroy(_In_ HWND hWnd);
VOID WINAPI OnPaint(_In_ HWND hWnd);
ATOM WINAPI RegisterWCEX(_In_ HINSTANCE hInstance);
VOID CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nShowCmd);

EXTERN CONST WCHAR g_wszClassName[];
EXTERN CONST WCHAR g_wszMutexName[];