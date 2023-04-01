#include <Windows.h>
#include <Vfw.h>
#include "aviCom.h"
#include "macros.h"

_Success_(SUCCEEDED(return))

HRESULT WINAPI CloseAVI(_In_ HAVI hAvi)
{
	if (hAvi == NULL || hAvi == INVALID_HANDLE_VALUE) return AVIERR_BADHANDLE;
	AVI_INFO *ai = (AVI_INFO*)hAvi;
	if (ai->aStream) AVIStreamRelease(ai->aStream);
	if (ai->pStCmp) AVIStreamRelease(ai->pStCmp);
	if (ai->pStream) AVIStreamRelease(ai->pStream);
	if (ai->pFile) AVIFileRelease(ai->pFile);
	AVIFileExit();
	delete ai;
	return S_OK;
}

_Success_(SUCCEEDED(return))

HAVI WINAPI CreateAVI(_In_z_ WCHAR *fileName, _In_ INT period)
{
	IAVIFile *pFile;
	HRESULT hr;
	AVI_INFO *ai = new AVI_INFO;

	AVIFileInit();
	hr = AVIFileOpenW(&pFile, fileName, OF_WRITE | OF_CREATE, NULL);
	if (hr != AVIERR_OK)
	{
		AVIFileExit();
		return (HAVI)INVALID_HANDLE_VALUE;
	}
	ai->pFile = pFile;

	ai->period = period;
	ai->aStream = NULL;
	ai->pStream = NULL;
	ai->pStCmp = NULL;
	ai->nFrame = 0;
	ai->nSample = 0;
	return (HAVI)ai;
}

HRESULT WINAPI AVIAddFrame(_In_ HAVI hAvi, _In_ HBITMAP hBitmap, _In_ DWORD dwRate)
{
	DIBSECTION dbs;
	INT sBitmap = GetObjectW(hBitmap, sizeof(DIBSECTION), &dbs);
	AVI_INFO *ai = (AVI_INFO*)hAvi;
	HRESULT hr;

	if (ai->pStream == NULL)
	{
		AVISTREAMINFOW asInfo;
		ZeroMemory(&asInfo, sizeof(AVISTREAMINFOW));
		asInfo.fccType = streamtypeVIDEO;
		asInfo.fccHandler = CODEC_CHARS; // Originally 0
		asInfo.dwScale = ai->period;
		//asInfo.dwRate = (DWORD)24;
		asInfo.dwRate = dwRate;
		asInfo.dwSuggestedBufferSize = dbs.dsBmih.biSizeImage;
		SetRect(&asInfo.rcFrame, 0, 0, dbs.dsBmih.biWidth, dbs.dsBmih.biHeight);
		hr = AVIFileCreateStreamW(ai->pFile, &ai->pStream, &asInfo);
		if (hr != AVIERR_OK) return hr;
	}

	if (ai->pStCmp == NULL)
	{
		AVICOMPRESSOPTIONS acOpt;
		HRESULT hr;
		ZeroMemory(&acOpt, sizeof(AVICOMPRESSOPTIONS));
		acOpt.fccHandler = CODEC_CHARS;
		hr = AVIMakeCompressedStream(&ai->pStCmp, ai->pStream, &acOpt, NULL);
		if (hr != AVIERR_OK) return hr;
		hr = AVIStreamSetFormat(ai->pStCmp, 0, &dbs.dsBmih, dbs.dsBmih.biSize + dbs.dsBmih.biClrUsed*sizeof(RGBQUAD));
		if (hr != AVIERR_OK) return hr;
	}

	hr = AVIStreamWrite(ai->pStCmp, ai->nFrame, 1, dbs.dsBm.bmBits, dbs.dsBmih.biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
	if (hr != AVIERR_OK) return hr;
	ai->nFrame++;

	return S_OK; 
}

_Success_(SUCCEEDED(return))

HRESULT WINAPI AVISetCompressionMode(_In_ HAVI hAvi, _In_ HBITMAP hBitmap, _In_ AVICOMPRESSOPTIONS *acOpt, _In_ DWORD dwRate)
{
	DIBSECTION dbs;
	INT sBitmap = GetObjectW(hBitmap, sizeof(dbs), &dbs);
	AVI_INFO *ai = (AVI_INFO*)hAvi;
	HRESULT hr;

	if (ai->pStream == NULL)
	{
		AVISTREAMINFOW asInfo;
		ZeroMemory(&asInfo, sizeof(AVISTREAMINFOW));
		asInfo.fccType = streamtypeVIDEO;
		asInfo.fccHandler = CODEC_CHARS; // Originally 0
		asInfo.dwScale = ai->period;
		//asInfo.dwRate = (DWORD)24;
		asInfo.dwRate = dwRate;
		asInfo.dwSuggestedBufferSize = dbs.dsBmih.biSizeImage;
		SetRect(&asInfo.rcFrame, 0, 0, dbs.dsBmih.biWidth, dbs.dsBmih.biHeight);
		hr = AVIFileCreateStreamW(ai->pFile, &ai->pStream, &asInfo);
		if (hr != AVIERR_OK)
		{
			return hr;
		}
	}

	if (ai->pStCmp != NULL) return AVIERR_COMPRESSOR;
	else
	{
		AVICOMPRESSOPTIONS newOpt, *farOpt[1];
		ZeroMemory(&newOpt, sizeof(AVICOMPRESSOPTIONS));
		if (acOpt != NULL)
		{
			farOpt[0] = acOpt;
		}
		else
		{
			farOpt[0] = &newOpt;
		}
		hr = AVIMakeCompressedStream(&ai->pStCmp, ai->pStream, farOpt[0], NULL);
		AVISaveOptionsFree(1, farOpt);
		if (hr != AVIERR_OK)
		{
			return hr;
		}
		hr = AVIStreamSetFormat(ai->pStCmp, 0, &dbs.dsBmih, dbs.dsBmih.biSize + dbs.dsBmih.biClrUsed*sizeof(RGBQUAD));
		if (hr != AVIERR_OK)
		{
			return hr;
		}
	}

	return AVIERR_OK;
}