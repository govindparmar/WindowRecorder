#pragma once
#include <Windows.h>
#include <Vfw.h>

#ifdef __cplusplus
extern "C"
{
#endif

	DECLARE_HANDLE(HAVI);

	typedef struct _AVI_INFO
	{
		IAVIFile *pFile;
		INT period;
		IAVIStream *aStream;
		IAVIStream *pStream;
		IAVIStream *pStCmp;
		ULONG nFrame;
		ULONG nSample;
	} AVI_INFO, *PAVI_INFO;

	_Success_(SUCCEEDED(return))

	HAVI CreateAVI(_In_z_ WCHAR *fileName, _In_ INT period);
	HRESULT AVIAddFrame(_In_ HAVI hAvi, _In_ HBITMAP hBitmap, _In_ DWORD dwRate);
	
	_Success_(SUCCEEDED(return))

	HRESULT AVISetCompressionMode(_In_ HAVI hAvi, _In_ HBITMAP hBitmap, _In_ AVICOMPRESSOPTIONS *acOpt, _In_ DWORD dwRate);

	_Success_(SUCCEEDED(return))

	HRESULT CloseAVI(_In_ HAVI hAvi);

#ifdef __cplusplus
}
#endif