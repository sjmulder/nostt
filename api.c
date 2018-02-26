#define USERAGENT	"nostt"
#define ENDPOINT	"http://teletekst-data.nos.nl/json/"
#define SUBST_CHAR	'%'

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "api.h"

#define LEN(a) (sizeof(a)/sizeof(*(a)))

enum parsest {
	PS_IN_TEXT,
	PS_IN_TAG,
	PS_IN_ATTRQUOTES,
};

/* see jsonwrite() below */
struct jsonctx {
	enum tterr		 err;
	struct json_tokener	*tokener;
	struct json_object	*object;
};

static char curlerrbuf[CURL_ERROR_SIZE];

/* indexed by ttcolor */
static const char *colornames[] = {
	"black",
	"blue",
	"green",
	"cyan",
	"red",
	"magenta",
	"yellow",
	"white"
};

static const struct ttattrs defattrs = {
	/* fg */	TT_WHITE,
	/* bg */	TT_BLACK
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

	ctx->object = json_tokener_parse_ex(ctx->tokener, ptr, sz * nmemb);
	if (!ctx->object) {
		jsonerr = json_tokener_get_error(ctx->tokener);
		if (jsonerr != json_tokener_continue)
			ctx->err = TT_EDATA;
	}

	return sz * nmemb;
}

/* does nothing if no match */
static void
parsecolor(const char *str, const char *end, enum ttcolor *color)
{
	int	i;
	size_t	len;

	for (i = 0; i < LEN(colornames); i++) {
		len = strlen(colornames[i]);
		if (len >= end - str && !strncmp(str, colornames[i], len)) {
			*color = (enum ttcolor)i;
			return;
		}
	}
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

		/* unescape hex escapes, used for block drawing characters */
		if (!strncmp("&#x", p, 3)) {
			wc = (wchar_t)strtol(p+3, (char **)&p, 16);
			if (!*p)
				p++; /* skip the ';' */
		} else
			wc = *p;

		switch (state) {
		case PS_IN_TEXT:
			switch (wc) {
			case '<':
				state = PS_IN_TAG;
				if (++attrdepth < LEN(attrstack))
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
				else if (attrdepth < LEN(attrstack))
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

enum tterr
tt_get(const char *id, struct ttpage *page)
{
	enum tterr		 err	= TT_OK;
	char			 url[128];
	CURL			*curl	= NULL;
	int			 status;
	struct jsonctx		 json;
	struct json_object	*jval;
	const char		*html;
	wchar_t			*wcp;
	int			 line, col;

	if (!id || !*id || !page)
		return TT_EARG;

	memset(&json, 0, sizeof(json));

	snprintf(url, LEN(url), ENDPOINT "%s", id);
	url[LEN(url)-1] = '\0';

	json.tokener = json_tokener_new();

	if (!(curl = curl_easy_init())) {
		err = TT_ECURL;
		goto cleanup;
	}

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlerrbuf);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "application/json");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, jsonwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);

	if (curl_easy_perform(curl) != CURLE_OK) {
		err = TT_ECURL;
		goto cleanup;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	if (status < 200 || status >= 300) {
		err = TT_EAPI;
		goto cleanup;
	}

	if (!json.object ||
	    !json_object_is_type(json.object, json_type_object)) {
		err = TT_EDATA;
		goto cleanup;
	}

	strncpy(page->id, id, LEN(page->id)-1);
	page->id[LEN(page->id)-1] = '\0';
	page->nextpage[0] = '\0';
	page->nextsub[0] = '\0';
 
	if (json_object_object_get_ex(json.object, "nextPage", &jval)) {
		strncpy(page->nextpage, json_object_get_string(jval),
		    LEN(page->nextpage)-1);
		page->nextpage[LEN(page->nextpage)-1] = '\0';
	}

	if (json_object_object_get_ex(json.object, "nextSubPage", &jval)) {
		strncpy(page->nextsub, json_object_get_string(jval),
		    LEN(page->nextsub)-1);
		page->nextsub[LEN(page->nextsub)-1] = '\0';
	}

	if (!json_object_object_get_ex(json.object, "content", &jval) ||
	    !(html = json_object_get_string(jval))) {
		err = TT_EDATA;
		goto cleanup;
	}

	parse(html, page);

	/* Map block drawing characters */
	for (line = 0; line < TT_NLINES; line++) {
		for (col = 0; col < TT_NCOLS; col++) {
			wcp = &page->chars[line][col];
			if (*wcp >= 0xF000)
				*wcp = SUBST_CHAR;	
		}
	}

cleanup:
	if (curl)
		curl_easy_cleanup(curl);
	if (json.tokener)
		json_tokener_free(json.tokener);

	return err;
}

const char *
tt_errstr(enum tterr err)
{
	switch (err) {
	case TT_OK:	return "no error";
	case TT_EARG:	return "invalid argument";
	case TT_ECURL:	return curlerrbuf;
	case TT_EAPI:	return "API returned an error code";
	case TT_EDATA:	return "API returned invalid or unexpected data";
	default:	return "unknown error";
	}
}
