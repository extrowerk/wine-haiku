/*
 * Copyright 2009 Andrew Eikum for CodeWeavers
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
#define CONST_VTABLE

#include <wine/test.h>

#include "mshtml.h"

struct location_test {
    const char *name;
    const WCHAR *url;

    const char *href;
    const BOOL href_ok;

    const char *protocol;
    const BOOL protocol_ok;

    const char *host;
    const BOOL host_ok;

    const char *hostname;
    const BOOL hostname_ok;

    const char *port;
    const BOOL port_ok;

    const char *pathname;
    const BOOL pathname_ok;

    const char *search;
    const BOOL search_ok;

    const char *hash;
    const BOOL hash_ok;
};

static const WCHAR http_url[] = {'h','t','t','p',':','/','/','w','w','w','.','w','i','n','e','h','q','.','o','r','g','?','s','e','a','r','c','h','#','h','a','s','h',0};
static const struct location_test http_test = {
            "HTTP",
            http_url,
            "http://www.winehq.org/?search#hash", FALSE,
            "http:", FALSE,
            "www.winehq.org:80", FALSE,
            "www.winehq.org", FALSE,
            "80", TRUE,
            "", FALSE,
            "?search", FALSE,
            "#hash", FALSE
            };

static const WCHAR http_file_url[] = {'h','t','t','p',':','/','/','w','w','w','.','w','i','n','e','h','q','.','o','r','g','/','f','i','l','e','?','s','e','a','r','c','h','#','h','a','s','h',0};
static const struct location_test http_file_test = {
            "HTTP with file",
            http_file_url,
            "http://www.winehq.org/file?search#hash", TRUE,
            "http:", FALSE,
            "www.winehq.org:80", FALSE,
            "www.winehq.org", FALSE,
            "80", TRUE,
            "file", FALSE,
            "?search", FALSE,
            "#hash", FALSE
            };

static const WCHAR ftp_url[] = {'f','t','p',':','/','/','f','t','p','.','w','i','n','e','h','q','.','o','r','g','/',0};
static const struct location_test ftp_test = {
            "FTP",
            ftp_url,
            "ftp://ftp.winehq.org/", TRUE,
            "ftp:", FALSE,
            "ftp.winehq.org:21", FALSE,
            "ftp.winehq.org", FALSE,
            "21", TRUE,
            "", FALSE,
            NULL, FALSE,
            NULL, FALSE
            };

static const WCHAR ftp_file_url[] = {'f','t','p',':','/','/','f','t','p','.','w','i','n','e','h','q','.','o','r','g','/','f','i','l','e',0};
static const struct location_test ftp_file_test = {
            "FTP with file",
            ftp_file_url,
            "ftp://ftp.winehq.org/file", TRUE,
            "ftp:", FALSE,
            "ftp.winehq.org:21", FALSE,
            "ftp.winehq.org", FALSE,
            "21", TRUE,
            "file", FALSE,
            NULL, FALSE,
            NULL, FALSE
            };

static const WCHAR file_url[] = {'f','i','l','e',':','/','/','C',':','\\','w','i','n','d','o','w','s','\\','w','i','n','.','i','n','i',0};
static const struct location_test file_test = {
            "FILE",
            file_url,
            "file:///C:/windows/win.ini", FALSE,
            "file:", FALSE,
            NULL, FALSE,
            NULL, FALSE,
            "", TRUE,
            "C:\\windows\\win.ini", TRUE,
            NULL, FALSE,
            NULL, FALSE
            };

static int str_eq_wa(LPCWSTR strw, const char *stra)
{
    CHAR buf[512];

    if(strw == NULL || stra == NULL){
        if((void*)strw == (void*)stra)
            return 1;
        return 0;
    }

    WideCharToMultiByte(CP_ACP, 0, strw, -1, buf, sizeof(buf), NULL, NULL);
    return !lstrcmpA(stra, buf);
}

static void test_href(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_href(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_href should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_href(loc, &str);
    ok(hres == S_OK, "%s: get_href failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->href_ok)
            ok(str_eq_wa(str, test->href),
                    "%s: expected retrieved href to be L\"%s\", was: %s\n",
                    test->name, test->href, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->href),
                    "%s: expected retrieved href to be L\"%s\", was: %s\n",
                    test->name, test->href, wine_dbgstr_w(str));
    }
}

static void test_protocol(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_protocol(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_protocol should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_protocol(loc, &str);
    todo_wine ok(hres == S_OK, "%s: get_protocol failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->protocol_ok)
            ok(str_eq_wa(str, test->protocol),
                    "%s: expected retrieved protocol to be L\"%s\", was: %s\n",
                    test->name, test->protocol, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->protocol),
                    "%s: expected retrieved protocol to be L\"%s\", was: %s\n",
                    test->name, test->protocol, wine_dbgstr_w(str));
    }
}

static void test_host(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_host(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_host should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_host(loc, &str);
    todo_wine ok(hres == S_OK, "%s: get_host failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->host_ok)
            ok(str_eq_wa(str, test->host),
                    "%s: expected retrieved host to be L\"%s\", was: %s\n",
                    test->name, test->host, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->host),
                    "%s: expected retrieved host to be L\"%s\", was: %s\n",
                    test->name, test->host, wine_dbgstr_w(str));
    }
}

static void test_hostname(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_hostname(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_hostname should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_hostname(loc, &str);
    todo_wine ok(hres == S_OK, "%s: get_hostname failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->hostname_ok)
            ok(str_eq_wa(str, test->hostname),
                    "%s: expected retrieved hostname to be L\"%s\", was: %s\n",
                    test->name, test->hostname, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->hostname),
                    "%s: expected retrieved hostname to be L\"%s\", was: %s\n",
                    test->name, test->hostname, wine_dbgstr_w(str));
    }
}

static void test_port(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_port(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_port should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_port(loc, &str);
    ok(hres == S_OK, "%s: get_port failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->port_ok)
            ok(str_eq_wa(str, test->port),
                    "%s: expected retrieved port to be L\"%s\", was: %s\n",
                    test->name, test->port, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->port),
                    "%s: expected retrieved port to be L\"%s\", was: %s\n",
                    test->name, test->port, wine_dbgstr_w(str));
    }
}

static void test_pathname(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_pathname(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_pathname should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_pathname(loc, &str);
    ok(hres == S_OK, "%s: get_pathname failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->pathname_ok)
            ok(str_eq_wa(str, test->pathname),
                    "%s: expected retrieved pathname to be L\"%s\", was: %s\n",
                    test->name, test->pathname, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->pathname),
                    "%s: expected retrieved pathname to be L\"%s\", was: %s\n",
                    test->name, test->pathname, wine_dbgstr_w(str));
    }
}

static void test_search(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_search(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_search should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_search(loc, &str);
    todo_wine ok(hres == S_OK, "%s: get_search failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->search_ok)
            ok(str_eq_wa(str, test->search),
                    "%s: expected retrieved search to be L\"%s\", was: %s\n",
                    test->name, test->search, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->search),
                    "%s: expected retrieved search to be L\"%s\", was: %s\n",
                    test->name, test->search, wine_dbgstr_w(str));
    }
}

static void test_hash(IHTMLLocation *loc, const struct location_test *test)
{
    HRESULT hres;
    BSTR str;

    hres = IHTMLLocation_get_hash(loc, NULL);
    ok(hres == E_POINTER,
            "%s: get_hash should have failed with E_POINTER (0x%08x), was: 0x%08x\n",
            test->name, E_POINTER, hres);

    hres = IHTMLLocation_get_hash(loc, &str);
    todo_wine ok(hres == S_OK, "%s: get_hash failed: 0x%08x\n", test->name, hres);
    if(hres == S_OK){
        if(test->hash_ok)
            ok(str_eq_wa(str, test->hash),
                    "%s: expected retrieved hash to be L\"%s\", was: %s\n",
                    test->name, test->hash, wine_dbgstr_w(str));
        else
            todo_wine ok(str_eq_wa(str, test->hash),
                    "%s: expected retrieved hash to be L\"%s\", was: %s\n",
                    test->name, test->hash, wine_dbgstr_w(str));
    }
}

static void perform_test(const struct location_test* test)
{
    HRESULT hres;
    IBindCtx *bc;
    IMoniker *url_mon;
    IPersistMoniker *persist_mon;
    IHTMLDocument2 *doc;
    IHTMLDocument6 *doc6;
    IHTMLLocation *location;

    hres = CreateBindCtx(0, &bc);
    ok(hres == S_OK, "%s: CreateBindCtx failed: 0x%08x\n", test->name, hres);
    if(FAILED(hres))
        return;

    hres = CreateURLMoniker(NULL, test->url, &url_mon);
    ok(hres == S_OK, "%s: CreateURLMoniker failed: 0x%08x\n", test->name, hres);
    if(FAILED(hres)){
        IBindCtx_Release(bc);
        return;
    }

    hres = CoCreateInstance(&CLSID_HTMLDocument, NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, &IID_IHTMLDocument2,
            (void**)&doc);
    ok(hres == S_OK, "%s: CoCreateInstance failed: 0x%08x\n", test->name, hres);
    if(FAILED(hres)){
        IMoniker_Release(url_mon);
        IBindCtx_Release(bc);
        return;
    }

    hres = IHTMLDocument2_QueryInterface(doc, &IID_IHTMLDocument6, (void**)&doc6);
    if(hres == S_OK){
        IHTMLDocument6_Release(doc6);
    }else{
        win_skip("%s: Could not get IHTMLDocument6, probably too old IE. Requires IE 8+\n", test->name);
        IMoniker_Release(url_mon);
        IBindCtx_Release(bc);
        return;
    }

    hres = IHTMLDocument2_QueryInterface(doc, &IID_IPersistMoniker,
            (void**)&persist_mon);
    ok(hres == S_OK, "%s: IHTMlDocument2_QueryInterface failed: 0x%08x\n", test->name, hres);
    if(FAILED(hres)){
        IHTMLDocument2_Release(doc);
        IMoniker_Release(url_mon);
        IBindCtx_Release(bc);
        return;
    }

    hres = IPersistMoniker_Load(persist_mon, FALSE, url_mon, bc,
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE);
    ok(hres == S_OK, "%s: IPersistMoniker_Load failed: 0x%08x\n", test->name, hres);
    if(FAILED(hres)){
        IPersistMoniker_Release(persist_mon);
        IHTMLDocument2_Release(doc);
        IMoniker_Release(url_mon);
        IBindCtx_Release(bc);
        return;
    }

    hres = IHTMLDocument2_get_location(doc, &location);
    ok(hres == S_OK, "%s: IHTMLDocument2_get_location failed: 0x%08x\n", test->name, hres);
    if(FAILED(hres)){
        IPersistMoniker_Release(persist_mon);
        IHTMLDocument2_Release(doc);
        IMoniker_Release(url_mon);
        IBindCtx_Release(bc);
        return;
    }

    test_href(location, test);
    test_protocol(location, test);
    test_host(location, test);
    test_hostname(location, test);
    test_port(location, test);
    test_pathname(location, test);
    test_search(location, test);
    test_hash(location, test);

    IHTMLLocation_Release(location);
    IPersistMoniker_Release(persist_mon);
    IHTMLDocument2_Release(doc);
    IMoniker_Release(url_mon);
    IBindCtx_Release(bc);
}

START_TEST(htmllocation)
{
    CoInitialize(NULL);

    perform_test(&http_test);
    perform_test(&http_file_test);
    perform_test(&ftp_test);
    perform_test(&ftp_file_test);
    perform_test(&file_test);

    CoUninitialize();
}