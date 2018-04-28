#pragma once
#include <Windows.h>
#include <Vfw.h>

#ifdef __cplusplus
extern "C"
{
#endif

	DECLARE_HANDLE(HAVI);

	struct AVI_INFO
	{
		IAVIFile *pFile;
		INT period;
		IAVIStream *aStream;
		IAVIStream *pStream;
		IAVIStream *pStCmp;
		ULONG nFrame;
		ULONG nSample;
	};

	HAVI CreateAVI(WCHAR *fileName, INT period);
	HRESULT AVIAddFrame(HAVI hAvi, HBITMAP hBitmap, DWORD dwRate);
	HRESULT AVISetCompressionMode(HAVI hAvi, HBITMAP hBitmap, AVICOMPRESSOPTIONS *acOpt, DWORD dwRate);
	HRESULT CloseAVI(HAVI hAvi);

#ifdef __cplusplus
}
#endif