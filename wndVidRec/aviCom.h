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

	HAVI CreateAVI(WCHAR *fileName, INT period);
	HRESULT AVIAddFrame(HAVI hAvi, HBITMAP hBitmap, DWORD dwRate);
	HRESULT AVISetCompressionMode(HAVI hAvi, HBITMAP hBitmap, AVICOMPRESSOPTIONS *acOpt, DWORD dwRate);
	HRESULT CloseAVI(HAVI hAvi);

#ifdef __cplusplus
}
#endif