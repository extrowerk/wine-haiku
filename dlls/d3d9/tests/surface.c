/*
 * Copyright 2006-2007 Henri Verbeet
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
 */
#define COBJMACROS
#include <d3d9.h>
#include "wine/test.h"

static HWND create_window(void)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = DefWindowProc;
    wc.lpszClassName = "d3d9_test_wc";
    RegisterClass(&wc);

    return CreateWindow("d3d9_test_wc", "d3d9_test",
            0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static IDirect3DDevice9 *init_d3d9(HMODULE d3d9_handle)
{
    IDirect3D9 * (__stdcall * d3d9_create)(UINT SDKVersion) = 0;
    IDirect3D9 *d3d9_ptr = 0;
    IDirect3DDevice9 *device_ptr = 0;
    D3DPRESENT_PARAMETERS present_parameters;
    HRESULT hr;

    d3d9_create = (void *)GetProcAddress(d3d9_handle, "Direct3DCreate9");
    ok(d3d9_create != NULL, "Failed to get address of Direct3DCreate9\n");
    if (!d3d9_create) return NULL;

    d3d9_ptr = d3d9_create(D3D_SDK_VERSION);
    if (!d3d9_ptr)
    {
        skip("could not create D3D9\n");
        return NULL;
    }

    ZeroMemory(&present_parameters, sizeof(present_parameters));
    present_parameters.Windowed = TRUE;
    present_parameters.hDeviceWindow = create_window();
    present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;

    hr = IDirect3D9_CreateDevice(d3d9_ptr, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
            NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &present_parameters, &device_ptr);

    if(FAILED(hr))
    {
        skip("could not create device, IDirect3D9_CreateDevice returned %#x\n", hr);
        return NULL;
    }

    return device_ptr;
}

static void test_surface_get_container(IDirect3DDevice9 *device_ptr)
{
    IDirect3DTexture9 *texture_ptr = 0;
    IDirect3DSurface9 *surface_ptr = 0;
    void *container_ptr;
    HRESULT hr;

    hr = IDirect3DDevice9_CreateTexture(device_ptr, 128, 128, 1, 0,
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture_ptr, 0);
    ok(SUCCEEDED(hr) && texture_ptr != NULL, "CreateTexture returned: hr %#x, texture_ptr %p. "
        "Expected hr %#x, texture_ptr != %p\n", hr, texture_ptr, D3D_OK, NULL);
    if (!texture_ptr || FAILED(hr)) goto cleanup;

    hr = IDirect3DTexture9_GetSurfaceLevel(texture_ptr, 0, &surface_ptr);
    ok(SUCCEEDED(hr) && surface_ptr != NULL, "GetSurfaceLevel returned: hr %#x, surface_ptr %p. "
        "Expected hr %#x, surface_ptr != %p\n", hr, surface_ptr, D3D_OK, NULL);
    if (!surface_ptr || FAILED(hr)) goto cleanup;

    /* These should work... */
    container_ptr = (void *)0x1337c0d3;
    hr = IDirect3DSurface9_GetContainer(surface_ptr, &IID_IUnknown, &container_ptr);
    ok(SUCCEEDED(hr) && container_ptr == texture_ptr, "GetContainer returned: hr %#x, container_ptr %p. "
        "Expected hr %#x, container_ptr %p\n", hr, container_ptr, S_OK, texture_ptr);
    if (container_ptr && container_ptr != (void *)0x1337c0d3) IUnknown_Release((IUnknown *)container_ptr);

    container_ptr = (void *)0x1337c0d3;
    hr = IDirect3DSurface9_GetContainer(surface_ptr, &IID_IDirect3DResource9, &container_ptr);
    ok(SUCCEEDED(hr) && container_ptr == texture_ptr, "GetContainer returned: hr %#x, container_ptr %p. "
        "Expected hr %#x, container_ptr %p\n", hr, container_ptr, S_OK, texture_ptr);
    if (container_ptr && container_ptr != (void *)0x1337c0d3) IUnknown_Release((IUnknown *)container_ptr);

    container_ptr = (void *)0x1337c0d3;
    hr = IDirect3DSurface9_GetContainer(surface_ptr, &IID_IDirect3DBaseTexture9, &container_ptr);
    ok(SUCCEEDED(hr) && container_ptr == texture_ptr, "GetContainer returned: hr %#x, container_ptr %p. "
        "Expected hr %#x, container_ptr %p\n", hr, container_ptr, S_OK, texture_ptr);
    if (container_ptr && container_ptr != (void *)0x1337c0d3) IUnknown_Release((IUnknown *)container_ptr);

    container_ptr = (void *)0x1337c0d3;
    hr = IDirect3DSurface9_GetContainer(surface_ptr, &IID_IDirect3DTexture9, &container_ptr);
    ok(SUCCEEDED(hr) && container_ptr == texture_ptr, "GetContainer returned: hr %#x, container_ptr %p. "
        "Expected hr %#x, container_ptr %p\n", hr, container_ptr, S_OK, texture_ptr);
    if (container_ptr && container_ptr != (void *)0x1337c0d3) IUnknown_Release((IUnknown *)container_ptr);

    /* ...and this one shouldn't. This should return E_NOINTERFACE and set container_ptr to NULL */
    container_ptr = (void *)0x1337c0d3;
    hr = IDirect3DSurface9_GetContainer(surface_ptr, &IID_IDirect3DSurface9, &container_ptr);
    ok(hr == E_NOINTERFACE && container_ptr == NULL, "GetContainer returned: hr %#x, container_ptr %p. "
        "Expected hr %#x, container_ptr %p\n", hr, container_ptr, E_NOINTERFACE, NULL);
    if (container_ptr && container_ptr != (void *)0x1337c0d3) IUnknown_Release((IUnknown *)container_ptr);

cleanup:
    if (texture_ptr) IDirect3DTexture9_Release(texture_ptr);
    if (surface_ptr) IDirect3DSurface9_Release(surface_ptr);
}

static void test_surface_alignment(IDirect3DDevice9 *device_ptr)
{
    IDirect3DSurface9 *surface_ptr = 0;
    HRESULT hr;
    int i;

    /* Test a sysmem surface as those aren't affected by the hardware's np2 restrictions */
    hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device_ptr, 5, 5, D3DFMT_R5G6B5, D3DPOOL_SYSTEMMEM, &surface_ptr, 0);
    ok(hr == D3D_OK, "IDirect3DDevice9_CreateOffscreenPlainSurface returned %08x\n", hr);

    if(surface_ptr)
    {
        D3DLOCKED_RECT lockedRect;
        hr = IDirect3DSurface9_LockRect(surface_ptr, &lockedRect, NULL, 0);
        ok(hr == D3D_OK, "IDirect3DSurface9_LockRect returned %08x\n", hr);
        ok(!(lockedRect.Pitch & 3), "Surface pitch %d is not 32-bit aligned\n", lockedRect.Pitch);
        /* Some applications also depend on the exact pitch, rather than just
         * the alignment.
         */
        ok(lockedRect.Pitch == 12, "Got pitch %d, expected 12\n", lockedRect.Pitch);
        hr = IDirect3DSurface9_UnlockRect(surface_ptr);
        ok(SUCCEEDED(hr), "IDirect3DSurface9_UnlockRect returned %#x.\n", hr);
        IDirect3DSurface9_Release(surface_ptr);
    }

    for (i = 0; i < 5; i++)
    {
        IDirect3DTexture9 *pTexture;
        int j, pitch;

        hr = IDirect3DDevice9_CreateTexture(device_ptr, 64, 64, 0, 0, MAKEFOURCC('D', 'X', 'T', '1'+i),
                                            D3DPOOL_MANAGED, &pTexture, NULL);
        ok(SUCCEEDED(hr) || hr == D3DERR_INVALIDCALL, "IDirect3DDevice9_CreateTexture: %08x\n", hr);
        if (FAILED(hr)) {
            skip("DXT%d surfaces are not supported\n", i + 1);
            continue;
        }

        for (j = IDirect3DBaseTexture9_GetLevelCount(pTexture) - 1; j >= 0; j--)
        {
            D3DLOCKED_RECT rc;
            D3DSURFACE_DESC descr;

            IDirect3DTexture9_GetLevelDesc(pTexture, j, &descr);
            hr = IDirect3DTexture9_LockRect(pTexture, j, &rc, NULL, 0);
            ok(SUCCEEDED(hr), "IDirect3DTexture9_LockRect: %08x\n", hr);
            hr = IDirect3DTexture9_UnlockRect(pTexture, j);
            ok(SUCCEEDED(hr), "IDirect3DTexture9_UnLockRect: %08x\n", hr);
            /* Windows XP returns D3D_OK when calling UnlockRect on an unlocked surface,
             * windows 7 returns an error.
             */

            pitch = ((descr.Width + 3) >> 2) << 3;
            if (i > 0) pitch <<= 1;
            ok(rc.Pitch == pitch, "Wrong pitch for DXT%d lvl[%d (%dx%d)]: expected %d got %d\n",
               i + 1, j, descr.Width, descr.Height, pitch, rc.Pitch);
        }
        IUnknown_Release( pTexture );
    }
}

/* Since the DXT formats are based on 4x4 blocks, locking works slightly
 * different than with regular formats. This patch verifies we return the
 * correct memory offsets */
static void test_lockrect_offset(IDirect3DDevice9 *device)
{
    IDirect3DSurface9 *surface = 0;
    IDirect3D9 *d3d;
    const RECT rect = {60, 60, 68, 68};
    D3DLOCKED_RECT locked_rect;
    int expected_pitch;
    unsigned int expected_offset;
    unsigned int offset;
    unsigned int i;
    BYTE *base;
    HRESULT hr;

    const struct {
        D3DFORMAT fmt;
        const char *name;
        unsigned int block_width;
        unsigned int block_height;
        unsigned int block_size;
    } dxt_formats[] = {
        {D3DFMT_DXT1,                 "D3DFMT_DXT1", 4, 4, 8},
        {D3DFMT_DXT2,                 "D3DFMT_DXT2", 4, 4, 16},
        {D3DFMT_DXT3,                 "D3DFMT_DXT3", 4, 4, 16},
        {D3DFMT_DXT4,                 "D3DFMT_DXT4", 4, 4, 16},
        {D3DFMT_DXT5,                 "D3DFMT_DXT5", 4, 4, 16},
        {MAKEFOURCC('A','T','I','2'), "ATI2N",       1, 1,  1},
    };
    hr = IDirect3DDevice9_GetDirect3D(device, &d3d);
    ok(SUCCEEDED(hr), "IDirect3DDevice9_GetDirect3D failed (%08x)\n", hr);

    for (i = 0; i < (sizeof(dxt_formats) / sizeof(*dxt_formats)); ++i) {
        hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, dxt_formats[i].fmt);
        if(FAILED(hr))
        {
            skip("Format %s not supported, skipping lockrect offset test\n", dxt_formats[i].name);
            continue;
        }

        hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 128, 128, dxt_formats[i].fmt, D3DPOOL_SCRATCH, &surface, 0);
        ok(SUCCEEDED(hr), "CreateOffscreenPlainSurface failed (%08x)\n", hr);

        hr = IDirect3DSurface9_LockRect(surface, &locked_rect, NULL, 0);
        ok(SUCCEEDED(hr), "LockRect failed (%08x)\n", hr);

        base = locked_rect.pBits;
        expected_pitch = (128 + dxt_formats[i].block_height - 1) / dxt_formats[i].block_width
                         * dxt_formats[i].block_size;
        ok(locked_rect.Pitch == expected_pitch, "Got pitch %d, expected pitch %d for format %s\n", locked_rect.Pitch, expected_pitch, dxt_formats[i].name);

        hr = IDirect3DSurface9_UnlockRect(surface);
        ok(SUCCEEDED(hr), "UnlockRect failed (%08x)\n", hr);
        hr = IDirect3DSurface9_UnlockRect(surface);
        ok(hr == D3DERR_INVALIDCALL, "Double UnlockRect returned %08x, expected D3DERR_INVALIDCALL\n", hr);

        hr = IDirect3DSurface9_LockRect(surface, &locked_rect, &rect, 0);
        ok(SUCCEEDED(hr), "LockRect failed (%08x)\n", hr);

        offset = (BYTE *)locked_rect.pBits - base;
        expected_offset = (rect.top / dxt_formats[i].block_height) * expected_pitch
                        + (rect.left / dxt_formats[i].block_width) * dxt_formats[i].block_size;
        ok(offset == expected_offset, "Got offset %u, expected offset %u for format %s\n", offset, expected_offset, dxt_formats[i].name);

        hr = IDirect3DSurface9_UnlockRect(surface);
        ok(SUCCEEDED(hr), "UnlockRect failed (%08x)\n", hr);

        IDirect3DSurface9_Release(surface);
    }
    IDirect3D9_Release(d3d);
}

struct rect_test
{
    RECT rect;
    HRESULT win7_result;
};

static void test_lockrect_invalid(IDirect3DDevice9 *device)
{
    IDirect3DSurface9 *surface = 0;
    D3DLOCKED_RECT locked_rect;
    unsigned int i;
    BYTE *base;
    HRESULT hr;

    const RECT test_rect_2 = { 0, 0, 8, 8 };
    const struct rect_test test_data[] = {
        {{60, 60, 68, 68},      D3D_OK},                /* Valid */
        {{60, 60, 60, 68},      D3DERR_INVALIDCALL},    /* 0 height */
        {{60, 60, 68, 60},      D3DERR_INVALIDCALL},    /* 0 width */
        {{68, 60, 60, 68},      D3DERR_INVALIDCALL},    /* left > right */
        {{60, 68, 68, 60},      D3DERR_INVALIDCALL},    /* top > bottom */
        {{-8, 60,  0, 68},      D3DERR_INVALIDCALL},    /* left < surface */
        {{60, -8, 68,  0},      D3DERR_INVALIDCALL},    /* top < surface */
        {{-16, 60, -8, 68},     D3DERR_INVALIDCALL},    /* right < surface */
        {{60, -16, 68, -8},     D3DERR_INVALIDCALL},    /* bottom < surface */
        {{60, 60, 136, 68},     D3DERR_INVALIDCALL},    /* right > surface */
        {{60, 60, 68, 136},     D3DERR_INVALIDCALL},    /* bottom > surface */
        {{136, 60, 144, 68},    D3DERR_INVALIDCALL},    /* left > surface */
        {{60, 136, 68, 144},    D3DERR_INVALIDCALL},    /* top > surface */
    };

    hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 128, 128, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, 0);
    ok(SUCCEEDED(hr), "CreateOffscreenPlainSurface failed (0x%08x)\n", hr);

    hr = IDirect3DSurface9_LockRect(surface, &locked_rect, NULL, 0);
    ok(SUCCEEDED(hr), "LockRect failed (0x%08x)\n", hr);

    base = locked_rect.pBits;

    hr = IDirect3DSurface9_UnlockRect(surface);
    ok(SUCCEEDED(hr), "UnlockRect failed (0x%08x)\n", hr);

    for (i = 0; i < (sizeof(test_data) / sizeof(*test_data)); ++i)
    {
        unsigned int offset, expected_offset;
        const RECT *rect = &test_data[i].rect;

        locked_rect.pBits = (BYTE *)0xdeadbeef;
        locked_rect.Pitch = 0xdeadbeef;

        hr = IDirect3DSurface9_LockRect(surface, &locked_rect, rect, 0);
        /* Windows XP accepts invalid locking rectangles, windows 7 rejects them.
         * Some games(C&C3) depend on the XP behavior, mark the Win7 one broken */
        ok(SUCCEEDED(hr) || broken(hr == test_data[i].win7_result),
                "LockRect failed (0x%08x) for rect [%d, %d]->[%d, %d]\n",
                hr, rect->left, rect->top, rect->right, rect->bottom);
        if(FAILED(hr)) continue;

        offset = (BYTE *)locked_rect.pBits - base;
        expected_offset = rect->top * locked_rect.Pitch + rect->left * 4;
        ok(offset == expected_offset, "Got offset %u, expected offset %u for rect [%d, %d]->[%d, %d]\n",
                offset, expected_offset, rect->left, rect->top, rect->right, rect->bottom);

        hr = IDirect3DSurface9_UnlockRect(surface);
        ok(SUCCEEDED(hr), "UnlockRect failed (0x%08x)\n", hr);
    }

    hr = IDirect3DSurface9_LockRect(surface, &locked_rect, NULL, 0);
    ok(SUCCEEDED(hr), "LockRect failed (0x%08x) for rect NULL\n", hr);
    hr = IDirect3DSurface9_LockRect(surface, &locked_rect, NULL, 0);
    ok(hr == D3DERR_INVALIDCALL, "Double LockRect for rect NULL returned 0x%08x\n", hr);
    hr = IDirect3DSurface9_UnlockRect(surface);
    ok(SUCCEEDED(hr), "UnlockRect failed (0x%08x)\n", hr);

    hr = IDirect3DSurface9_LockRect(surface, &locked_rect, &test_data[0].rect, 0);
    ok(hr == D3D_OK, "LockRect failed (0x%08x) for rect [%d, %d]->[%d, %d]"
            ", expected D3D_OK (0x%08x)\n", hr, test_data[0].rect.left, test_data[0].rect.top,
            test_data[0].rect.right, test_data[0].rect.bottom, D3D_OK);
    hr = IDirect3DSurface9_LockRect(surface, &locked_rect, &test_data[0].rect, 0);
    ok(hr == D3DERR_INVALIDCALL, "Double LockRect failed (0x%08x) for rect [%d, %d]->[%d, %d]"
            ", expected D3DERR_INVALIDCALL (0x%08x)\n", hr, test_data[0].rect.left, test_data[0].rect.top,
            test_data[0].rect.right, test_data[0].rect.bottom, D3DERR_INVALIDCALL);
    hr = IDirect3DSurface9_LockRect(surface, &locked_rect, &test_rect_2, 0);
    ok(hr == D3DERR_INVALIDCALL, "Double LockRect failed (0x%08x) for rect [%d, %d]->[%d, %d]"
            ", expected D3DERR_INVALIDCALL (0x%08x)\n", hr, test_rect_2.left, test_rect_2.top,
            test_rect_2.right, test_rect_2.bottom, D3DERR_INVALIDCALL);
    hr = IDirect3DSurface9_UnlockRect(surface);
    ok(SUCCEEDED(hr), "UnlockRect failed (0x%08x)\n", hr);

    IDirect3DSurface9_Release(surface);
}

static ULONG getref(IUnknown *iface)
{
    IUnknown_AddRef(iface);
    return IUnknown_Release(iface);
}

static void test_private_data(IDirect3DDevice9 *device)
{
    HRESULT hr;
    IDirect3DSurface9 *surface;
    ULONG ref, ref2;
    IUnknown *ptr;
    DWORD size = sizeof(IUnknown *);

    hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 4, 4, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, 0);
    ok(SUCCEEDED(hr), "CreateImageSurface failed (0x%08x)\n", hr);
    if(!surface)
    {
        return;
    }

    /* This fails */
    hr = IDirect3DSurface9_SetPrivateData(surface, &IID_IDirect3DSurface9 /* Abuse this tag */, device, 0, D3DSPD_IUNKNOWN);
    ok(hr == D3DERR_INVALIDCALL, "IDirect3DSurface9_SetPrivateData failed with %08x\n", hr);
    hr = IDirect3DSurface9_SetPrivateData(surface, &IID_IDirect3DSurface9 /* Abuse this tag */, device, 5, D3DSPD_IUNKNOWN);
    ok(hr == D3DERR_INVALIDCALL, "IDirect3DSurface9_SetPrivateData failed with %08x\n", hr);
    hr = IDirect3DSurface9_SetPrivateData(surface, &IID_IDirect3DSurface9 /* Abuse this tag */, device, sizeof(IUnknown *) * 2, D3DSPD_IUNKNOWN);
    ok(hr == D3DERR_INVALIDCALL, "IDirect3DSurface9_SetPrivateData failed with %08x\n", hr);

    ref = getref((IUnknown *) device);
    hr = IDirect3DSurface9_SetPrivateData(surface, &IID_IDirect3DSurface9 /* Abuse this tag */, device, sizeof(IUnknown *), D3DSPD_IUNKNOWN);
    ok(hr == D3D_OK, "IDirect3DSurface9_SetPrivateData failed with %08x\n", hr);
    ref2 = getref((IUnknown *) device);
    ok(ref2 == ref + 1, "Object reference is %d, expected %d\n", ref2, ref + 1);
    hr = IDirect3DSurface9_FreePrivateData(surface, &IID_IDirect3DSurface9);
    ok(hr == D3D_OK, "IDirect3DSurface9_FreePrivateData returned %08x\n", hr);
    ref2 = getref((IUnknown *) device);
    ok(ref2 == ref, "Object reference is %d, expected %d\n", ref2, ref);

    hr = IDirect3DSurface9_SetPrivateData(surface, &IID_IDirect3DSurface9, device, sizeof(IUnknown *), D3DSPD_IUNKNOWN);
    ok(hr == D3D_OK, "IDirect3DSurface9_SetPrivateData failed with %08x\n", hr);
    hr = IDirect3DSurface9_SetPrivateData(surface, &IID_IDirect3DSurface9, surface, sizeof(IUnknown *), D3DSPD_IUNKNOWN);
    ok(hr == D3D_OK, "IDirect3DSurface9_SetPrivateData failed with %08x\n", hr);
    ref2 = getref((IUnknown *) device);
    ok(ref2 == ref, "Object reference is %d, expected %d\n", ref2, ref);

    hr = IDirect3DSurface9_SetPrivateData(surface, &IID_IDirect3DSurface9, device, sizeof(IUnknown *), D3DSPD_IUNKNOWN);
    ok(hr == D3D_OK, "IDirect3DSurface9_SetPrivateData failed with %08x\n", hr);
    hr = IDirect3DSurface9_GetPrivateData(surface, &IID_IDirect3DSurface9, &ptr, &size);
    ok(hr == D3D_OK, "IDirect3DSurface9_GetPrivateData failed with %08x\n", hr);
    ref2 = getref((IUnknown *) device);
    /* Object is addrefed */
    ok(ptr == (IUnknown *) device, "Returned interface pointer is %p, expected %p\n", ptr, device);
    ok(ref2 == ref + 2, "Object reference is %d, expected %d. ptr at %p, orig at %p\n", ref2, ref + 2, ptr, device);
    IUnknown_Release(ptr);

    IDirect3DSurface9_Release(surface);

    /* Destroying the surface frees the held reference */
    ref2 = getref((IUnknown *) device);
    /* -1 because the surface was released and held a reference before */
    ok(ref2 == (ref - 1), "Object reference is %d, expected %d\n", ref2, (ref - 1));
}

static void test_getdc(IDirect3DDevice9 *device)
{
    IDirect3DSurface9 *surface;
    IDirect3DTexture9 *texture;
    HRESULT hr;
    unsigned int i;
    HDC dc;

    struct
    {
        const char *name;
        D3DFORMAT fmt;
        BOOL getdc_supported;
    } testdata[] = {
        { "D3DFMT_A8R8G8B8",    D3DFMT_A8R8G8B8,    TRUE    },
        { "D3DFMT_X8R8G8B8",    D3DFMT_X8R8G8B8,    TRUE    },
        { "D3DFMT_R5G6B5",      D3DFMT_R5G6B5,      TRUE    },
        { "D3DFMT_X1R5G5B5",    D3DFMT_X1R5G5B5,    TRUE    },
        { "D3DFMT_A1R5G5B5",    D3DFMT_A1R5G5B5,    TRUE    },
        { "D3DFMT_R8G8B8",      D3DFMT_R8G8B8,      TRUE    },
        { "D3DFMT_A2R10G10B10", D3DFMT_A2R10G10B10, FALSE   }, /* Untested, card on windows didn't support it */
        { "D3DFMT_V8U8",        D3DFMT_V8U8,        FALSE   },
        { "D3DFMT_Q8W8V8U8",    D3DFMT_Q8W8V8U8,    FALSE   },
        { "D3DFMT_A8B8G8R8",    D3DFMT_A8B8G8R8,    FALSE   },
        { "D3DFMT_X8B8G8R8",    D3DFMT_A8B8G8R8,    FALSE   },
        { "D3DFMT_R3G3B2",      D3DFMT_R3G3B2,      FALSE   },
        { "D3DFMT_P8",          D3DFMT_P8,          FALSE   },
        { "D3DFMT_L8",          D3DFMT_L8,          FALSE   },
        { "D3DFMT_A8L8",        D3DFMT_A8L8,        FALSE   },
        { "D3DFMT_DXT1",        D3DFMT_DXT1,        FALSE   },
        { "D3DFMT_DXT2",        D3DFMT_DXT2,        FALSE   },
        { "D3DFMT_DXT3",        D3DFMT_DXT3,        FALSE   },
        { "D3DFMT_DXT4",        D3DFMT_DXT4,        FALSE   },
        { "D3DFMT_DXT5",        D3DFMT_DXT5,        FALSE   },
    };

    for(i = 0; i < (sizeof(testdata) / sizeof(testdata[0])); i++)
    {
        texture = NULL;
        hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 64, 64, testdata[i].fmt, D3DPOOL_SYSTEMMEM, &surface, NULL);
        if(FAILED(hr))
        {
            hr = IDirect3DDevice9_CreateTexture(device, 64, 64, 1, 0, testdata[i].fmt, D3DPOOL_MANAGED, &texture, NULL);
            if(FAILED(hr))
            {
                skip("IDirect3DDevice9_CreateOffscreenPlainSurface failed, hr = 0x%08x, fmt %s\n", hr, testdata[i].name);
                continue;
            }
            IDirect3DTexture9_GetSurfaceLevel(texture, 0, &surface);
        }

        dc = (void *) 0x1234;
        hr = IDirect3DSurface9_GetDC(surface, &dc);

        if(testdata[i].getdc_supported)
        {
            ok(SUCCEEDED(hr), "GetDC on format %s failed(hr=0x%08x), but was expected to work\n",
               testdata[i].name, hr);
        }
        else
        {
            ok(FAILED(hr), "GetDC on format %s worked(hr=0x%08x), but was expected to fail\n",
               testdata[i].name, hr);
        }

        if(SUCCEEDED(hr))
        {
            hr = IDirect3DSurface9_ReleaseDC(surface, dc);
            ok(hr == D3D_OK, "IDirect3DSurface9_ReleaseDC failed, hr = 0x%08x\n", hr);
        }
        else
        {
            ok(dc == (void *) 0x1234, "After failed getDC dc is %p\n", dc);
        }

        IDirect3DSurface9_Release(surface);
        if(texture) IDirect3DTexture9_Release(texture);
    }
}

static void test_surface_dimensions(IDirect3DDevice9 *device)
{
    IDirect3DSurface9 *surface;
    HRESULT hr;

    hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 0, 1, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL);
    ok(hr == D3DERR_INVALIDCALL, "CreateOffscreenPlainSurface returned %#x, expected D3DERR_INVALIDCALL.\n", hr);
    if (SUCCEEDED(hr)) IDirect3DSurface9_Release(surface);

    hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL);
    ok(hr == D3DERR_INVALIDCALL, "CreateOffscreenPlainSurface returned %#x, expected D3DERR_INVALIDCALL.\n", hr);
    if (SUCCEEDED(hr)) IDirect3DSurface9_Release(surface);
}

START_TEST(surface)
{
    HMODULE d3d9_handle;
    IDirect3DDevice9 *device_ptr;
    ULONG refcount;

    d3d9_handle = LoadLibraryA("d3d9.dll");
    if (!d3d9_handle)
    {
        skip("Could not load d3d9.dll\n");
        return;
    }

    device_ptr = init_d3d9(d3d9_handle);
    if (!device_ptr) return;

    test_surface_get_container(device_ptr);
    test_surface_alignment(device_ptr);
    test_lockrect_offset(device_ptr);
    test_lockrect_invalid(device_ptr);
    test_private_data(device_ptr);
    test_getdc(device_ptr);
    test_surface_dimensions(device_ptr);

    refcount = IDirect3DDevice9_Release(device_ptr);
    ok(!refcount, "Device has %u references left\n", refcount);
}
