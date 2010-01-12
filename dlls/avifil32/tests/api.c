/*
 * Unit test suite for AVI Functions
 *
 * Copyright 2008 Detlef Riekenberg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 */

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "wingdi.h"
#include "vfw.h"
#include "wine/test.h"

/* ########################### */

static const CHAR winetest0[] = "winetest0";
static const CHAR winetest1[] = "winetest1";
static const CHAR testfilename[]  = "wine_avifil32_test.avi";

/* ########################### */

static const DWORD deffh[] = /* file_header */
{
    FOURCC_RIFF, 0x34c6 /* length */, formtypeAVI,
    FOURCC_LIST, 0x1ac /* length */,
    listtypeAVIHEADER, ckidAVIMAINHDR, sizeof(MainAVIHeader),
};

static const MainAVIHeader defmah =
{
    0x00008256, /* dwMicroSecPerFrame   */
    0x000080e8, /* dwMaxBytesPerSec     */
    0x00000000, /* dwPaddingGranularity */
    0x00000910, /* dwFlags              */
    1,          /* dwTotalFrames        */
    0,          /* dwInitialFrames      */
    2,          /* dwStreams            */
    0x00100000, /* dwSuggestedBufferSize*/
    8,          /* dwWidth              */
    6,          /* dwHeight             */
    { 0, 0, 0, 0 } /* dwReserved[4] */
};

static const AVIStreamHeader defash0 =
{
    streamtypeVIDEO, /* fccType              */
    0x30323449,      /* fccHandler           */
    0x00000000,      /* dwFlags              */
    0,               /* wPriority            */
    0,               /* wLanguage            */
    0,               /* dwInitialFrames      */
    0x000003e9,      /* dwScale              */
    0x00007530,      /* dwRate               */
    0,               /* dwStart              */
    1,               /* dwLength             */
    0x00100000,      /* dwSuggestedBufferSize*/
    0xffffffff,      /* dwQuality            */
    0,               /* dwSampleSize         */
    { 0, 0, 0, 0 }   /* short left right top bottom */
};

static const AVIStreamHeader defash1 =
{
    /* AVIStreamHeader */
    streamtypeAUDIO, /* fccType              */
    1,               /* fccHandler           */
    0,               /* dwFlags              */
    0,               /* wPriority            */
    0,               /* wLanguage            */
    0,               /* dwInitialFrames      */
    1,               /* dwScale              */
    0x00002b11,      /* dwRate               */
    0,               /* dwStart              */
    0x00000665,      /* dwLength             */
    0x00003000,      /* dwSuggestedBufferSize*/
    0xffffffff,      /* dwQuality            */
    2,               /* dwSampleSize         */
    { 0, 0, 0, 0 }   /* short left right top bottom */
};

static const PCMWAVEFORMAT defpcmwf =
{
    {
        1,      /* wFormatTag      */
        2,      /* nChannels       */
        11025,  /* nSamplesPerSec  */
        22050,  /* nAvgBytesPerSec */
        2,      /* nBlockAlign     */
    },
    8,      /* wBitsPerSample  */
};

typedef struct common_avi_headers {
    DWORD           fh[sizeof(deffh)];
    MainAVIHeader   mah;
    AVIStreamHeader ash0;
    AVIStreamHeader ash1;
    PCMWAVEFORMAT   pcmwf;
} COMMON_AVI_HEADERS;

/* Extra data needed to get the VFW API to load the file */
/* DWORD deffh */
/* MainAVIHeader mah */
static const DWORD streamlist[] =
{
    FOURCC_LIST, 0xd4 /* length */,
    listtypeSTREAMHEADER, ckidSTREAMHEADER, 0x38 /* length */,
};
/* AVIStreamHeader ash0 */
static const DWORD videostreamformat[] =
{
    ckidSTREAMFORMAT, 0x28 /* length */,
    0x00000028, 0x00000008, 0x00000006, 0x00180001,
    0x30323449, 0x00000090, 0x00000000, 0x00000000,
    0x00000000, 0x00000000,
};
static const DWORD padding1[] =
{
    ckidAVIPADDING, 0xc /* length */,
    0x00000004, 0x00000000, 0x63643030
};
static const DWORD videopropheader[] =
{
    0x70727076, 0x44 /* length */,
    0x00000000, 0x00000000,
    0x0000001e, 0x00000008, 0x00000006, 0x00100009,
    0x00000008, 0x00000006, 0x00000001, 0x00000006,
    0x00000008, 0x00000006, 0x00000008, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    FOURCC_LIST, 0x70 /* length */,
    listtypeSTREAMHEADER, ckidSTREAMHEADER, 0x38 /* length */,
};
/* AVIStreamHeader ash1 */
static const DWORD audiostreamformat_pre[] =
{
    ckidSTREAMFORMAT, sizeof(PCMWAVEFORMAT) /* length */,
};
/* PCMWAVEFORMAT pcmwf */
static DWORD data[] =
{
    ckidAVIPADDING, 0xc /* length */,
    0x00000004, 0x00000000, 0x62773130,
    ckidAVIPADDING, 0xc /* length */,
    0x6c6d646f, 0x686c6d64, 0x000000f8,
    FOURCC_LIST, 0x18 /* length */,
    0x4f464e49,
    0x54465349, 0xc /* length */,
    0x6676614c, 0x332e3235, 0x00302e37,
    ckidAVIPADDING, 0x4 /* length */,
    0,
    FOURCC_LIST, 0xd1b /* length */, listtypeAVIMOVIE,
    0, 0
};

/* ########################### */

static void test_AVISaveOptions(void)
{
    AVICOMPRESSOPTIONS options[2];
    LPAVICOMPRESSOPTIONS poptions[2];
    PAVISTREAM streams[2] = {NULL, NULL};
    HRESULT hres;
    DWORD   res;
    LONG    lres;

    poptions[0] = &options[0];
    poptions[1] = &options[1];
    ZeroMemory(options, sizeof(options));

    SetLastError(0xdeadbeef);
    hres = CreateEditableStream(&streams[0], NULL);
    ok(hres == AVIERR_OK, "0: got 0x%x and %p (expected AVIERR_OK)\n", hres, streams[0]);

    SetLastError(0xdeadbeef);
    hres = CreateEditableStream(&streams[1], NULL);
    ok(hres == AVIERR_OK, "1: got 0x%x and %p (expected AVIERR_OK)\n", hres, streams[1]);

    SetLastError(0xdeadbeef);
    hres = EditStreamSetNameA(streams[0], winetest0);
    todo_wine ok(hres == AVIERR_OK, "0: got 0x%x (expected AVIERR_OK)\n", hres);

    SetLastError(0xdeadbeef);
    hres = EditStreamSetNameA(streams[1], winetest1);
    todo_wine ok(hres == AVIERR_OK, "1: got 0x%x (expected AVIERR_OK)\n", hres);

    if (winetest_interactive) {
        SetLastError(0xdeadbeef);
        res = AVISaveOptions(0, ICMF_CHOOSE_DATARATE |ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_ALLCOMPRESSORS,
                             2, streams, poptions);
        trace("got %u with 0x%x/%u\n", res, GetLastError(), GetLastError());
    }

    SetLastError(0xdeadbeef);
    lres = AVISaveOptionsFree(2, poptions);
    ok(lres == AVIERR_OK, "got 0x%x with 0x%x/%u\n", lres, GetLastError(), GetLastError());

    SetLastError(0xdeadbeef);
    res = AVIStreamRelease(streams[0]);
    ok(res == 0, "0: got refcount %u (expected 0)\n", res);

    SetLastError(0xdeadbeef);
    res = AVIStreamRelease(streams[1]);
    ok(res == 0, "1: got refcount %u (expected 0)\n", res);

}

/* ########################### */

static void init_test_struct(COMMON_AVI_HEADERS *cah)
{
    memcpy(cah->fh, deffh, sizeof(deffh));
    cah->mah = defmah;
    cah->ash0 = defash0;
    cah->ash1 = defash1;
    cah->pcmwf = defpcmwf;
}

static void create_avi_file(const COMMON_AVI_HEADERS *cah, char *filename)
{
    HANDLE hFile;
    DWORD written;

    hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    ok(hFile != INVALID_HANDLE_VALUE, "Couldn't create file\n");

    WriteFile(hFile, &cah->fh, sizeof(deffh), &written, NULL);
    WriteFile(hFile, &cah->mah, sizeof(MainAVIHeader), &written, NULL);
    WriteFile(hFile, streamlist, sizeof(streamlist), &written, NULL);
    WriteFile(hFile, &cah->ash0, 0x38, &written, NULL);
    WriteFile(hFile, videostreamformat, sizeof(videostreamformat), &written, NULL);
    WriteFile(hFile, padding1, sizeof(padding1), &written, NULL);
    WriteFile(hFile, videopropheader, sizeof(videopropheader), &written, NULL);
    WriteFile(hFile, &cah->ash1, 0x38, &written, NULL);
    WriteFile(hFile, audiostreamformat_pre, sizeof(audiostreamformat_pre), &written, NULL);
    WriteFile(hFile, &cah->pcmwf, sizeof(PCMWAVEFORMAT), &written, NULL);
    WriteFile(hFile, data, sizeof(data), &written, NULL);

    CloseHandle(hFile);
}

static void test_default_data(void)
{
    COMMON_AVI_HEADERS cah;
    char filename[MAX_PATH];
    PAVIFILE pFile;
    int res;
    LONG lSize;
    PAVISTREAM pStream0;
    PAVISTREAM pStream1;
    AVISTREAMINFO asi0;
    AVISTREAMINFO asi1;
    WAVEFORMATEX wfx;

    GetTempPath(MAX_PATH, filename);
    strcpy(filename+strlen(filename), testfilename);

    init_test_struct(&cah);
    create_avi_file(&cah, filename);

    res = AVIFileOpen(&pFile, filename, OF_SHARE_DENY_WRITE, 0L);
    ok(res != AVIERR_BADFORMAT, "Unable to open file: error1=%u\n", AVIERR_BADFORMAT);
    ok(res != AVIERR_MEMORY, "Unable to open file: error2=%u\n", AVIERR_MEMORY);
    ok(res != AVIERR_FILEREAD, "Unable to open file: error3=%u\n", AVIERR_FILEREAD);
    ok(res != AVIERR_FILEOPEN, "Unable to open file: error4=%u\n", AVIERR_FILEOPEN);
    ok(res != REGDB_E_CLASSNOTREG, "Unable to open file: error5=%u\n", REGDB_E_CLASSNOTREG);
    ok(res == 0, "Unable to open file: error=%u\n", res);

    res = AVIFileGetStream(pFile, &pStream0, 0, 0);
    ok(res == 0, "Unable to open video stream: error=%u\n", res);

    res = AVIFileGetStream(pFile, &pStream1, 0, 1);
    ok(res == 0, "Unable to open audio stream: error=%u\n", res);

    res = AVIStreamInfo(pStream0, &asi0, sizeof(AVISTREAMINFO));
    ok(res == 0, "Unable to read stream info: error=%u\n", res);

    res = AVIStreamInfo(pStream1, &asi1, sizeof(AVISTREAMINFO));
    ok(res == 0, "Unable to read stream info: error=%u\n", res);

    res = AVIStreamReadFormat(pStream0, AVIStreamStart(pStream1), NULL, &lSize);
    ok(res == 0, "Unable to read format size: error=%u\n", res);

    res = AVIStreamReadFormat(pStream1, AVIStreamStart(pStream1), &wfx, &lSize);
    ok(res == 0, "Unable to read format: error=%u\n", res);

    ok(asi0.fccType == streamtypeVIDEO, "got 0x%x (expected streamtypeVIDEO)\n", asi0.fccType);
    ok(asi0.fccHandler == 0x30323449, "got 0x%x (expected 0x30323449)\n", asi0.fccHandler);
    ok(asi0.dwFlags == 0, "got %u (expected 0)\n", asi0.dwFlags);
    ok(asi0.wPriority == 0, "got %u (expected 0)\n", asi0.wPriority);
    ok(asi0.wLanguage == 0, "got %u (expected 0)\n", asi0.wLanguage);
    ok(asi0.dwScale == 1001, "got %u (expected 1001)\n", asi0.dwScale);
    ok(asi0.dwRate == 30000, "got %u (expected 30000)\n", asi0.dwRate);
    ok(asi0.dwStart == 0, "got %u (expected 0)\n", asi0.dwStart);
    ok(asi0.dwLength == 1, "got %u (expected 1)\n", asi0.dwLength);
    ok(asi0.dwInitialFrames == 0, "got %u (expected 0)\n", asi0.dwInitialFrames);
    ok(asi0.dwSuggestedBufferSize == 0, "got %u (expected 0)\n", asi0.dwSuggestedBufferSize);
    ok(asi0.dwQuality == 0xffffffff, "got 0x%x (expected 0xffffffff)\n", asi0.dwQuality);
    ok(asi0.dwSampleSize == 0, "got %u (expected 0)\n", asi0.dwSampleSize);
    ok(asi0.rcFrame.left == 0, "got %u (expected 0)\n", asi0.rcFrame.left);
    ok(asi0.rcFrame.top == 0, "got %u (expected 0)\n", asi0.rcFrame.top);
    ok(asi0.rcFrame.right == 8, "got %u (expected 8)\n", asi0.rcFrame.right);  /* these are based on the values in the mah and not */
    ok(asi0.rcFrame.bottom == 6, "got %u (expected 6)\n", asi0.rcFrame.bottom);/* on the ones in the ash which are 0 here */
    ok(asi0.dwEditCount == 0, "got %u (expected 0)\n", asi0.dwEditCount);
    ok(asi0.dwFormatChangeCount == 0, "got %u (expected 0)\n)", asi0.dwFormatChangeCount);

    ok(asi1.fccType == streamtypeAUDIO, "got 0x%x (expected streamtypeVIDEO)\n", asi1.fccType);
    ok(asi1.fccHandler == 0x1, "got 0x%x (expected 0x1)\n", asi1.fccHandler);
    ok(asi1.dwFlags == 0, "got %u (expected 0)\n", asi1.dwFlags);
    ok(asi1.wPriority == 0, "got %u (expected 0)\n", asi1.wPriority);
    ok(asi1.wLanguage == 0, "got %u (expected 0)\n", asi1.wLanguage);
    ok(asi1.dwScale == 1, "got %u (expected 1)\n", asi1.dwScale);
    ok(asi1.dwRate == 11025, "got %u (expected 11025)\n", asi1.dwRate);
    ok(asi1.dwStart == 0, "got %u (expected 0)\n", asi1.dwStart);
    ok(asi1.dwLength == 1637, "got %u (expected 1637)\n", asi1.dwLength);
    ok(asi1.dwInitialFrames == 0, "got %u (expected 0)\n", asi1.dwInitialFrames);
    ok(asi1.dwSuggestedBufferSize == 0, "got %u (expected 0)\n", asi1.dwSuggestedBufferSize);
    ok(asi1.dwQuality == 0xffffffff, "got 0x%x (expected 0xffffffff)\n", asi1.dwQuality);
    ok(asi1.dwSampleSize == 2, "got %u (expected 2)\n", asi1.dwSampleSize);
    ok(asi1.rcFrame.left == 0, "got %u (expected 0)\n", asi1.rcFrame.left);
    ok(asi1.rcFrame.top == 0, "got %u (expected 0)\n", asi1.rcFrame.top);
    ok(asi1.rcFrame.right == 0, "got %u (expected 0)\n", asi1.rcFrame.right);
    ok(asi1.rcFrame.bottom == 0, "got %u (expected 0)\n", asi1.rcFrame.bottom);
    ok(asi1.dwEditCount == 0, "got %u (expected 0)\n", asi1.dwEditCount);
    ok(asi1.dwFormatChangeCount == 0, "got %u (expected 0)\n)", asi1.dwFormatChangeCount);

    ok(wfx.wFormatTag == 1, "got %u (expected 1)\n",wfx.wFormatTag);
    ok(wfx.nChannels == 2, "got %u (expected 2)\n",wfx.nChannels);
    ok(wfx.wFormatTag == 1, "got %u (expected 1)\n",wfx.wFormatTag);
    ok(wfx.nSamplesPerSec == 11025, "got %u (expected 11025)\n",wfx.nSamplesPerSec);
    ok(wfx.nAvgBytesPerSec == 22050, "got %u (expected 22050)\n",wfx.nAvgBytesPerSec);
    ok(wfx.nBlockAlign == 2, "got %u (expected 2)\n",wfx.nBlockAlign);

    AVIStreamRelease(pStream0);
    AVIStreamRelease(pStream1);
    AVIFileRelease(pFile);
    ok(DeleteFile(filename) !=0, "Deleting file %s failed", filename);
}

static void test_amh_corruption(void)
{
    COMMON_AVI_HEADERS cah;
    char filename[MAX_PATH];
    PAVIFILE pFile;
    int res;

    GetTempPath(MAX_PATH, filename);
    strcpy(filename+strlen(filename), testfilename);

    /* Make sure only AVI files with the proper headers will be loaded */
    init_test_struct(&cah);
    cah.fh[3] = mmioFOURCC('A', 'V', 'i', ' ');

    create_avi_file(&cah, filename);
    res = AVIFileOpen(&pFile, filename, OF_SHARE_DENY_WRITE, 0L);
    ok(res != 0, "Able to open file: error=%u\n", res);

    ok(DeleteFile(filename) !=0, "Deleting file %s failed\n", filename);
}

static void test_ash1_corruption(void)
{
    COMMON_AVI_HEADERS cah;
    char filename[MAX_PATH];
    PAVIFILE pFile;
    int res;
    PAVISTREAM pStream1;
    AVISTREAMINFO asi1;

    GetTempPath(MAX_PATH, filename);
    strcpy(filename+strlen(filename), testfilename);

    /* Corrupt the sample size in the audio stream header */
    init_test_struct(&cah);
    cah.ash1.dwSampleSize = 0xdeadbeef;

    create_avi_file(&cah, filename);

    res = AVIFileOpen(&pFile, filename, OF_SHARE_DENY_WRITE, 0L);
    ok(res == 0, "Unable to open file: error=%u\n", res);

    res = AVIFileGetStream(pFile, &pStream1, 0, 1);
    ok(res == 0, "Unable to open audio stream: error=%u\n", res);

    res = AVIStreamInfo(pStream1, &asi1, sizeof(AVISTREAMINFO));
    ok(res == 0, "Unable to read stream info: error=%u\n", res);

    /* The result will still be 2, because the value is dynamically replaced with the nBlockAlign
       value from the stream format header. The next test will prove this */
    ok(asi1.dwSampleSize == 2, "got %u (expected 2)\n", asi1.dwSampleSize);

    AVIStreamRelease(pStream1);
    AVIFileRelease(pFile);
    ok(DeleteFile(filename) !=0, "Deleting file %s failed", filename);
}

static void test_ash1_corruption2(void)
{
    COMMON_AVI_HEADERS cah;
    char filename[MAX_PATH];
    PAVIFILE pFile;
    int res;
    PAVISTREAM pStream1;
    AVISTREAMINFO asi1;

    GetTempPath(MAX_PATH, filename);
    strcpy(filename+strlen(filename), testfilename);

    /* Corrupt the block alignment in the audio format header */
    init_test_struct(&cah);
    cah.pcmwf.wf.nBlockAlign = 0xdead;

    create_avi_file(&cah, filename);

    res = AVIFileOpen(&pFile, filename, OF_SHARE_DENY_WRITE, 0L);
    ok(res == 0, "Unable to open file: error=%u\n", res);

    res = AVIFileGetStream(pFile, &pStream1, 0, 1);
    ok(res == 0, "Unable to open audio stream: error=%u\n", res);

    ok(AVIStreamInfo(pStream1, &asi1, sizeof(AVISTREAMINFO)) == 0, "Unable to read stream info\n");

    /* The result will also be the corrupt value, as explained above. */
    ok(asi1.dwSampleSize == 0xdead, "got 0x%x (expected 0xdead)\n", asi1.dwSampleSize);

    AVIStreamRelease(pStream1);
    AVIFileRelease(pFile);
    ok(DeleteFile(filename) !=0, "Deleting file %s failed", filename);
}

/* ########################### */

START_TEST(api)
{

    AVIFileInit();
    test_AVISaveOptions();
    test_default_data();
    test_amh_corruption();
    test_ash1_corruption();
    test_ash1_corruption2();
    AVIFileExit();

}