#include <Windows.h>
#include <Vfw.h>
#include "aviCom.h"
#include "macros.h"

_Success_(SUCCEEDED(return))

HRESULT WINAPI CloseAVI(_In_ HAVI hAvi)
{
	AVI_INFO *ai = nullptr;
	if (NULL == hAvi || INVALID_HANDLE_VALUE == hAvi)
	{
		return AVIERR_BADHANDLE;
	}
	ai = (AVI_INFO*)hAvi;
	if (ai->aStream)
	{
		AVIStreamRelease(ai->aStream);
	}
	if (ai->pStCmp)
	{
		AVIStreamRelease(ai->pStCmp);
	}
	if (ai->pStream)
	{
		AVIStreamRelease(ai->pStream);
	}
	if (ai->pFile)
	{
		AVIFileRelease(ai->pFile);
	}
	AVIFileExit();
	delete ai;
	return S_OK;
}

_Success_(return != (HAVI)INVALID_HANDLE_VALUE)

HAVI WINAPI CreateAVI(_In_reads_or_z_(MAX_PATH) WCHAR *fileName, _In_ INT period)
{
	IAVIFile *pFile = nullptr;
	HRESULT hr;
	AVI_INFO *ai = new AVI_INFO;

	AVIFileInit();
	hr = AVIFileOpenW(&pFile, fileName, OF_WRITE | OF_CREATE, NULL);
	if (FAILED(hr))
	{
		AVIFileExit();
		return (HAVI)INVALID_HANDLE_VALUE;
	}
	ai->pFile = pFile;
	ai->period = period;
	ai->aStream = nullptr;
	ai->pStream = nullptr;
	ai->pStCmp = nullptr;
	ai->nFrame = 0;
	ai->nSample = 0;

	return (HAVI)ai;
}

_Success_(SUCCEEDED(return))

HRESULT WINAPI AVIAddFrame(_In_ HAVI hAvi, _In_ HBITMAP hBitmap, _In_ DWORD dwRate)
{
	DIBSECTION dbs;
	INT sBitmap = GetObjectW(hBitmap, sizeof(DIBSECTION), &dbs);
	AVI_INFO *ai = (AVI_INFO*)hAvi;
	HRESULT hr;

	if (ai->pStream == nullptr)
	{
		AVISTREAMINFOW asInfo;
		ZeroMemory(&asInfo, sizeof(AVISTREAMINFOW));

		asInfo.fccType = streamtypeVIDEO;
		asInfo.fccHandler = CODEC_CHARS;
		asInfo.dwScale = ai->period;
		asInfo.dwRate = dwRate;
		asInfo.dwSuggestedBufferSize = dbs.dsBmih.biSizeImage;
		SetRect(&asInfo.rcFrame, 0, 0, dbs.dsBmih.biWidth, dbs.dsBmih.biHeight);
		hr = AVIFileCreateStreamW(ai->pFile, &ai->pStream, &asInfo);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	if (ai->pStCmp == nullptr)
	{
		AVICOMPRESSOPTIONS acOpt;
		HRESULT hr;
		ZeroMemory(&acOpt, sizeof(AVICOMPRESSOPTIONS));
		acOpt.fccHandler = CODEC_CHARS;
		hr = AVIMakeCompressedStream(&ai->pStCmp, ai->pStream, &acOpt, nullptr);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = AVIStreamSetFormat(ai->pStCmp, 0, &dbs.dsBmih, dbs.dsBmih.biSize + dbs.dsBmih.biClrUsed * sizeof(RGBQUAD));
		if (FAILED(hr))
		{
			return hr;
		}
	}

	hr = AVIStreamWrite(ai->pStCmp, ai->nFrame, 1, dbs.dsBm.bmBits, dbs.dsBmih.biSizeImage, AVIIF_KEYFRAME, nullptr, nullptr);
	if (FAILED(hr))
	{
		return hr;
	}
	ai->nFrame++;

	return S_OK; 
}

_Success_(SUCCEEDED(return))

HRESULT WINAPI AVISetCompressionMode(_In_ HAVI hAvi, _In_ HBITMAP hBitmap, _In_ AVICOMPRESSOPTIONS *acOpt, _In_ DWORD dwRate)
{
	DIBSECTION dbs;
	INT sBitmap = GetObjectW(hBitmap, sizeof(dbs), &dbs);
	AVI_INFO *ai = (AVI_INFO *)hAvi;
	HRESULT hr;

	if (ai->pStream == nullptr)
	{
		AVISTREAMINFOW asInfo;
		ZeroMemory(&asInfo, sizeof(AVISTREAMINFOW));
		asInfo.fccType = streamtypeVIDEO;
		asInfo.fccHandler = CODEC_CHARS;
		asInfo.dwScale = ai->period;
		asInfo.dwRate = dwRate;
		asInfo.dwSuggestedBufferSize = dbs.dsBmih.biSizeImage;
		SetRect(&asInfo.rcFrame, 0, 0, dbs.dsBmih.biWidth, dbs.dsBmih.biHeight);
		hr = AVIFileCreateStreamW(ai->pFile, &ai->pStream, &asInfo);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	if (ai->pStCmp != nullptr)
	{
		return AVIERR_COMPRESSOR;
	}
	else
	{
		AVICOMPRESSOPTIONS newOpt, *farOpt[1];
		ZeroMemory(&newOpt, sizeof(AVICOMPRESSOPTIONS));
		if (acOpt != nullptr)
		{
			farOpt[0] = acOpt;
		}
		else
		{
			farOpt[0] = &newOpt;
		}
		hr = AVIMakeCompressedStream(&ai->pStCmp, ai->pStream, farOpt[0], nullptr);
		if (FAILED(hr))
		{
			return hr;
		}
		AVISaveOptionsFree(1, farOpt);
		hr = AVIStreamSetFormat(ai->pStCmp, 0, &dbs.dsBmih, dbs.dsBmih.biSize + dbs.dsBmih.biClrUsed * sizeof(RGBQUAD));
		if (FAILED(hr))
		{
			return hr;
		}
	}

	return AVIERR_OK;
}