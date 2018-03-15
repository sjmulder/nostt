/* api.c - Copyright (c) 2018, Sijmen J. Mulder (see LICENSE.md) */

#define USERAGENT	"nostt (+https://github.com/sjmulder/nostt)"
#define ENDPOINT	"http://teletekst-data.nos.nl/json/"
#define SUBST_CHAR	'%'

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <json-c/json.h>
#include "api.h"

#if USE_WINHTTP

#include <windows.h>
#include <winhttp.h>

#ifndef WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY 4
#endif

#else /* USE_WINHTTP */

#include <curl/curl.h>

#endif /* !USE_WINHTTP */

#define LEN(a)		(sizeof(a)/sizeof(*(a)))
#define MIN(a, b)	((a)>(b)?(b):(a))
#define WIDE_(x)	L ## x
#define WIDE(x)		WIDE_(x)

enum parsest {
	PS_IN_TEXT,
	PS_IN_TAG,
	PS_IN_ATTRQUOTES
};

struct entity {
	char	seq[10];
	wchar_t	wc;
};

static int	lasterr_libc;
#if USE_WINHTTP
static DWORD	lasterr_win32;
#else
static char	curlerrbuf[CURL_ERROR_SIZE];
#endif


/* indexed by ttcolor */
static const char *colornames[] = {
	"black",
	"red",
	"green",
	"yellow",
	"blue",
	"magenta",
	"cyan",
	"white"
};

static const struct entity entities[] = {
	/* HTMLspecial */
	{ "&qout;",	0x22 }, { "&amp;",	0x26 },
	{ "&apos;",	0x27 }, { "&lt;",	0x3C },
	{ "&gt;",	0x3E },
	/* HTMLlat1 */
	{ "&AElig;",	0xC6 }, { "&Aacute;",	0xC1 },
	{ "&Acirc;",	0xC2 }, { "&Agrave;",	0xC0 },
	{ "&Aring;",	0xC5 }, { "&Atilde;",	0xC3 },
	{ "&Auml;",	0xC4 }, { "&Ccedil;",	0xC7 },
	{ "&ETH;",	0xD0 }, { "&Eacute;",	0xC9 },
	{ "&Ecirc;",	0xCA }, { "&Egrave;",	0xC8 },
	{ "&Euml;",	0xCB }, { "&Iacute;",	0xCD },
	{ "&Icirc;",	0xCE }, { "&Igrave;",	0xCC },
	{ "&Iuml;",	0xCF }, { "&Ntilde;",	0xD1 },
	{ "&Oacute;",	0xD3 }, { "&Ocirc;",	0xD4 },
	{ "&Ograve;",	0xD2 }, { "&Oslash;",	0xD8 },
	{ "&Otilde;",	0xD5 }, { "&Ouml;",	0xD6 },
	{ "&THORN;",	0xDE }, { "&Uacute;",	0xDA },
	{ "&Ucirc;",	0xDB }, { "&Ugrave;",	0xD9 },
	{ "&Uuml;",	0xDC }, { "&Yacute;",	0xDD },
	{ "&aacute;",	0xE1 }, { "&acirc;",	0xE2 },
	{ "&aelig;",	0xE6 }, { "&agrave;",	0xE0 },
	{ "&aring;",	0xE5 }, { "&atilde;",	0xE3 },
	{ "&auml;",	0xE4 }, { "&ccedil;",	0xE7 },
	{ "&eacute;",	0xE9 }, { "&ecirc;",	0xEA },
	{ "&egrave;",	0xE8 }, { "&eth;",	0xF0 },
	{ "&euml;",	0xEB }, { "&iacute;",	0xED },
	{ "&icirc;",	0xEE }, { "&igrave;",	0xEC },
	{ "&iuml;",	0xEF }, { "&ntilde;",	0xF1 },
	{ "&oacute;",	0xF3 }, { "&ocirc;",	0xF4 },
	{ "&ograve;",	0xF2 }, { "&oslash;",	0xF8 },
	{ "&otilde;",	0xF5 }, { "&ouml;",	0xF6 },
	{ "&szlig;",	0xDF }, { "&thorn;",	0xFE },
	{ "&uacute;",	0xFA }, { "&ucirc;",	0xFB },
	{ "&ugrave;",	0xF9 }, { "&uuml;",	0xFC },
	{ "&yacute;",	0xFD }, { "&yuml;",	0xFF }
};

static const struct ttattrs defattrs = {
	/* fg */	TT_WHITE,
	/* bg */	TT_BLACK
};

/* does nothing if no match */
static void
parsecolor(const char *str, const char *end, enum ttcolor *color)
{
	size_t	i, len;

	for (i = 0; i < LEN(colornames); i++) {
		len = strlen(colornames[i]);
		if ((ptrdiff_t)len >= end - str &&
		    !strncmp(str, colornames[i], len)) {
			*color = (enum ttcolor)i;
			return;
		}
	}
}

/* Unescapes HTML entities. *endp will point to the first character past the
   escape sequence. */
static wchar_t
unescape(const char *sequence, const char **endp)
{
	wchar_t	wc;
	size_t	len, i;

	if (*sequence != '&') {
		/* assign wc first in case sequence=*endp */
		wc = *sequence;
		*endp = sequence + 1;
		return wc;
	}

	if (!strncmp("&#x", sequence, 3)) {
		wc = (wchar_t)strtol(sequence+3, (char **)endp, 16);
		if (**endp == ';')
			(*endp)++;
		return wc;
	}

	for (i = 0; i < LEN(entities); i++) {
		len = strlen(entities[i].seq);
		if (!strncmp(entities[i].seq, sequence, len)) {
			*endp = sequence + len;
			return entities[i].wc;
		}
	}

	*endp = sequence + 1;
	return '&';
}

/* parses class name lists like "red bg-white" into attrs->fg and attrs->bg,
   or leaves them untouched if not specified */
static void
parsecolors(const char *str, const char *end, struct ttattrs *attrs)
{
	enum ttcolor	*color;
	const char	*wordend;

	while (end - str > 3) {
		if (memcmp("bg-", str, 3) == 0) {
			color = &attrs->bg;
			str += 3;
		} else
			color = &attrs->fg;

		wordend = str+1;
		while (wordend < end && !isspace(*wordend))
			wordend++;

		parsecolor(str, wordend, color);

		str = wordend;
		while (str < end && isspace(*str))
			str++;
	}
}

/* Very simple HTML parser. Only accepts the following sort of input:

     <span class="red bg-white">NOS</span> TELETEKST
     Nieuws  <span class="cyan"><a href="#101">101</a></span>
     Sport   <span class="cyan"><a href="#102">102</a></span>

   Every cell in the page is assigned, either with content, or with a space
   character.

   HTML element and attribute names themselves are ignored; if a tag contains
   quotes it is assumed to be a class list. Any tag with a '/' in it is
   considered a closing tag. Nesting is supported, but no self-closing tags
   and such.

   This may all seem horribly limited but it's only meant to parse the HTML
   output from the API, which it does. */
static enum tterr
parse(const char *html, struct ttpage *page)
{
	const char	*p;
	wchar_t		 wc;
	int		 line		= 0;
	int		 col		= 0;
	const char	*openquote	= NULL;
	const char	*closequote	= NULL;
	enum parsest	 state		= PS_IN_TEXT;
	struct ttattrs	 curattrs	= defattrs;
	struct ttattrs	 attrstack[8];
	int		 attrdepth	= 0;

	attrstack[0] = defattrs;

	p = html;
	while (line < TT_NLINES) {
		/* clear rest of line if EOL or EOF */
		if (*p == '\0' || *p == '\n') {
			if (*p == '\n')
				p++;
			while (col < TT_NCOLS) {
				page->attrs[line][col] = defattrs;
				page->chars[line][col] = L' ';
				col++;
			}
			line++;
			col = 0;
			continue;
		}

		if (*p == '&') {
			wc = unescape(p, &p);
			p--; /* offset the p++ later on */
		} else
			wc = *p;

		switch (state) {
		case PS_IN_TEXT:
			switch (wc) {
			case '<':
				state = PS_IN_TAG;
				if (++attrdepth < (int)LEN(attrstack))
					attrstack[attrdepth] = curattrs;
				break;
			default:
				/* ignore input beyond line length */
				if (col < TT_NCOLS) {
					page->chars[line][col] = wc;
					page->attrs[line][col] = curattrs;
					col++;
				}
				break;
			}
			break;

		case PS_IN_TAG:
			switch (wc) {
			case '/':
				/* End tag, pop attrs. Twice, because we just
				   pushed on the start of this closing tag
				   too. */
				if ((attrdepth -= 2) < 0)
					attrdepth = 0;
				else if (attrdepth < (int)LEN(attrstack))
					curattrs = attrstack[attrdepth+1];
				break;
			case '"':
				state = PS_IN_ATTRQUOTES;
				openquote = p;
				break;
			case '>':
				state = PS_IN_TEXT;
				break;
			}
			break;

		case PS_IN_ATTRQUOTES:
			switch (wc) {
			case '"':
				state = PS_IN_TAG;
				closequote = p;
				break;
			case '>':
				state = PS_IN_TEXT;
				closequote = p;
				break;
			}
			if (state != PS_IN_ATTRQUOTES) {
				/* we assume the attribute is 'class', so
				   parse the colors */
				parsecolors(openquote+1, closequote,
				    &curattrs);
			}
			break;
		}

		p++;
	}

	return TT_OK;
}

#if USE_WINHTTP
static WCHAR *
awsubstr(const WCHAR *str, size_t n)
{
	WCHAR	*mem;

	if (!(mem = malloc((n+1) * sizeof(WCHAR))))
		return NULL;
	memcpy(mem, str, n * sizeof(WCHAR));
	mem[n] = L'\0';
	return mem;
}

static enum tterr
fetchjson(const char *url, struct json_object **jroot)
{
	static const WCHAR *accept[] = {
		L"application/json",
		NULL
	};

	static HINTERNET	 session;

	enum tterr		 err		= TT_OK;
	WCHAR			 wurl[128];
	URL_COMPONENTS		 urlc;
	DWORD			 flags;
	BOOL			 ok;
	WCHAR			*host		= NULL;
	WCHAR			*path		= NULL;
	HINTERNET		 conn		= 0;
	HINTERNET		 req		= 0;
	struct json_tokener	*tokener	= NULL;
	enum json_tokener_error  jsonerr;
	char			 buf[4096];
	DWORD			 nbytes, nread;

	if (!session) {
		session = WinHttpOpen(WIDE(USERAGENT),
		    WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
		    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!session) {
			err = TT_EWIN32;
			goto cleanup;
		}
	}

	if (!MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, LEN(wurl))) {
		err = TT_EWIN32;
		goto cleanup;
	}

	memset(&urlc, 0, sizeof(urlc));
	urlc.dwStructSize = sizeof(urlc);
	urlc.dwHostNameLength = -1;
	urlc.dwUrlPathLength = -1;
	urlc.dwExtraInfoLength = -1;

	if (!WinHttpCrackUrl(wurl, 0, 0, &urlc)) {
		err = TT_EWIN32;
		goto cleanup;
	}

	host = awsubstr(urlc.lpszHostName, urlc.dwHostNameLength);
	if (!host) {
		err = TT_ELIBC;
		goto cleanup;
	}

	path = awsubstr(urlc.lpszUrlPath, urlc.dwUrlPathLength +
	    urlc.dwExtraInfoLength);
	if (!host) {
		err = TT_ELIBC;
		goto cleanup;
	}

	conn = WinHttpConnect(session, host, urlc.nPort, 0);
	if (!conn) {
		err = TT_EWIN32;
		goto cleanup;
	}

	flags = 0;
	if (urlc.nScheme == INTERNET_SCHEME_HTTPS)
		flags |= WINHTTP_FLAG_SECURE;

	req = WinHttpOpenRequest(conn, L"GET", path, NULL, WINHTTP_NO_REFERER,
	    accept, flags);
	if (!req) {
		err = TT_EWIN32;
		goto cleanup;
	}

	ok = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
	    WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if (!ok) {
		err = TT_EWIN32;
		goto cleanup;
	}

	tokener = json_tokener_new();
	*jroot = NULL;

	if (!WinHttpReceiveResponse(req, NULL)) {
		err = TT_EWIN32;
		goto cleanup;
	}

	while (WinHttpQueryDataAvailable(req, &nbytes)) {
		if (*jroot) {
			err = TT_EDATA;
			return 0;
		}

		nbytes = MIN(nbytes, sizeof(buf));
		ok = WinHttpReadData(req, buf, nbytes, &nread);
		if (!ok) {
			err = TT_EWIN32;
			goto cleanup;
		}

		*jroot = json_tokener_parse_ex(tokener, buf, nread);
		if (!*jroot) {
			jsonerr = json_tokener_get_error(tokener);
			if (jsonerr != json_tokener_continue) {
				err = TT_EDATA;
				return 0;
			}
		}

		nbytes -= nread;
	}

	if (*jroot)
		err = TT_EDATA;

cleanup:
	if (err == TT_ELIBC)
		lasterr_libc = errno;
	else if (err == TT_EWIN32)
		lasterr_win32 = GetLastError();

	if (tokener)
		json_tokener_free(tokener);
	if (req)
		WinHttpCloseHandle(req);
	if (conn)
		WinHttpCloseHandle(conn);

	free(path);
	free(host);

	return err;
}
#else /* USE_WINHTTP */
struct jsonctx {
	enum tterr		 err;
	struct json_tokener	*tokener;
	struct json_object	*object;
};

/* Callback for curl; directly forwards data to the JSON parser. */
static size_t
jsonwrite(char *ptr, size_t sz, size_t nmemb, struct jsonctx *ctx)
{
	enum json_tokener_error jsonerr;

	if (ctx->err != TT_OK)
		return 0;

	if (ctx->object) {
		ctx->err = TT_EDATA;
		return 0;
	}

	ctx->object = json_tokener_parse_ex(ctx->tokener, ptr,
	    (int)(sz * nmemb));
	if (!ctx->object) {
		jsonerr = json_tokener_get_error(ctx->tokener);
		if (jsonerr != json_tokener_continue)
			ctx->err = TT_EDATA;
	}

	return sz * nmemb;
}

static enum tterr
fetchjson(const char *url, struct json_object **jroot)
{
	enum tterr		 err	= TT_OK;
	struct jsonctx		 json;
	CURL			*curl	= NULL;
	struct curl_slist	*list	= NULL;
	long			 status;

	memset(&json, 0, sizeof(json));
	json.tokener = json_tokener_new();

	if (!(curl = curl_easy_init()))
		return TT_ECURL;

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlerrbuf);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, jsonwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);

	list = curl_slist_append(list, "Accept-Encoding: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

	if (curl_easy_perform(curl) != CURLE_OK) {
		err = TT_ECURL;
		goto cleanup;
	}

	curl_slist_free_all(list);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	if (status < 200 || status >= 300) {
		err = TT_EAPI;
		goto cleanup;
	}

	*jroot = json.object;

cleanup:
	if (curl)
		curl_easy_cleanup(curl);
	if (json.tokener)
		json_tokener_free(json.tokener);

	return err;
}
#endif /* !USE_WINHTTP */

enum tterr
tt_get(const char *id, struct ttpage *page)
{
	enum tterr		 err	= TT_OK;
	char			 url[128];
	struct json_object	*jroot;
	struct json_object	*jval;
	const char		*html;
	wchar_t			*wcp;
	int			 line, col;

	if (!id || !*id || !page)
		return TT_EARG;

	snprintf(url, LEN(url), ENDPOINT "%s", id);
	url[LEN(url)-1] = '\0';

	if ((err = fetchjson(url, &jroot)) != TT_OK)
		return err;

	if (!jroot || !json_object_is_type(jroot, json_type_object))
		return TT_EDATA;

	strncpy(page->id, id, LEN(page->id)-1);
	page->id[LEN(page->id)-1] = '\0';
	page->nextpage[0] = '\0';
	page->nextsub[0] = '\0';

	if (json_object_object_get_ex(jroot, "nextPage", &jval)) {
		strncpy(page->nextpage, json_object_get_string(jval),
		    LEN(page->nextpage)-1);
		page->nextpage[LEN(page->nextpage)-1] = '\0';
	}

	if (json_object_object_get_ex(jroot, "nextSubPage", &jval)) {
		strncpy(page->nextsub, json_object_get_string(jval),
		    LEN(page->nextsub)-1);
		page->nextsub[LEN(page->nextsub)-1] = '\0';
	}

	if (!json_object_object_get_ex(jroot, "content", &jval) ||
	    !(html = json_object_get_string(jval)))
		return TT_EDATA;

	parse(html, page);

	/* Map block drawing characters */
	for (line = 0; line < TT_NLINES; line++) {
		for (col = 0; col < TT_NCOLS; col++) {
			wcp = &page->chars[line][col];
			if (*wcp >= 0xF000)
				*wcp = SUBST_CHAR;
		}
	}

	return err;
}

const char *
tt_errstr(enum tterr err)
{
#if USE_WINHTTP
	static char *msg_win32;

	DWORD nchars;
#endif

	switch (err) {
	case TT_OK:	return "no error";
	case TT_ELIBC:	return strerror(lasterr_libc);
#if !USE_WINHTTP
	case TT_ECURL:	return curlerrbuf;
#endif
	case TT_EARG:	return "invalid argument";
	case TT_EAPI:	return "API returned an error code";
	case TT_EDATA:	return "API returned invalid or unexpected data";

#if USE_WINHTTP
	case TT_EWIN32:
		if (msg_win32) {
			LocalFree(msg_win32);
			msg_win32 = NULL;
		}
		nchars = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		    FORMAT_MESSAGE_FROM_SYSTEM, NULL, lasterr_win32, 0,
		    (char*)&msg_win32, 0, NULL);
		if (!nchars) {
			msg_win32 = NULL;
			return "Windows API error (failed to get message)";
		}
		return msg_win32;
#endif

	default:
		return "unknown error";
	}
}
