#define DEBUG_IO	0
#define DEBUG_HTML	0
#define DEBUG_PARSER	0

#define SUBST_CHAR	'%'

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <curl/curl.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
# include <json-c/json.h>
#pragma clang diagnostic pop

#include "api.h"

#define LEN(a) (sizeof(a)/sizeof(*(a)))

#if DEBUG_IO
# define debug_io	printf
#else
# define debug_io(...)	(void)0
#endif

#if DEBUG_PARSER
# define debug_parser	printf
#else
# define debug_parser(...) (void)0
#endif

const char *tt_useragent	= "nostt";
const char *tt_endpoint		= "http://teletekst-data.nos.nl/json/";

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

static const struct ttgetopts defopts = {
	/* useragent */	"nostt",
	/* endpoint */	"http://teletekst-data.nos.nl/json/",
	/* mapmode */	TT_MDEFAULT
};

/* Callback for curl; directly forwards data to the JSON parser. */
static size_t
jsonwrite(char *ptr, size_t sz, size_t nmemb, struct jsonctx *ctx)
{
	enum json_tokener_error jsonerr;

	debug_io("jsonwrite: sz=%zu nmemb=%zu\n", sz, nmemb);

	if (ctx->err != TT_OK)
		return 0;

	if (ctx->object) {
		debug_io("extraneous JSON data\n");
		ctx->err = TT_EDATA;
		return 0;
	}

	ctx->object = json_tokener_parse_ex(ctx->tokener, ptr, sz * nmemb);
	if (!ctx->object) {
		jsonerr = json_tokener_get_error(ctx->tokener);
		if (jsonerr != json_tokener_continue) {
			debug_io("JSON write error\n");
			ctx->err = TT_EDATA;
		}
	}

	return sz * nmemb;
}

/* See the note above ttmapmode in api.h for an explanation */
static wchar_t
mapboxchar(wchar_t wc, enum ttmapmode mode)
{
	switch (mode) {
	case TT_MASCII:
		return wc > 0x7F ? SUBST_CHAR : wc;
	case TT_MUNICODE:
		break; /* continue below */
	case TT_MNONE:
	default:
		return wc;
	}

	/* For the basic conversion case, we simply strip out the middle of
	   the three rows . For a few special cases (marked '!') we deviate
	   and use a different mapping because the middle row is very
	   important (e.g. for '.. xx ..'). */

	switch (wc) {
	case 0xF020: return L' ';
	case 0xF021: return 0x2598; /* x. .. .. -> x. .. */
	case 0xF022: return 0x259D; /* .x .. .. -> .x .. */
	case 0xF023: return 0x2580; /* xx .. .. -> xx .. */
	case 0xF024: return L' ';   /* .. x. .. -> .. .. */
	case 0xF025: return 0x2598; /* x. x. .. -> x. .. */
	case 0xF026: return 0x259D; /* .x x. .. -> .x .. */
	case 0xF027: return 0x2580; /* xx x. .. -> xx .. */
	case 0xF028: return L' ';   /* .. .x .. -> .. .. */
	case 0xF029: return 0x2598; /* x. .x .. -> x. .. */
	case 0xF02A: return 0x259D; /* .x .x .. -> .x .. */
	case 0xF02B: return 0x2580; /* xx .x .. -> xx .. */
	case 0xF02C: return 0x2584; /* .. xx .. -> .. xx   ! */
	case 0xF02D: return 0x2599; /* x. xx .. -> x. xx   ! */
	case 0xF02E: return 0x259F; /* .x xx .. -> .x xx   ! */
	case 0xF02F: return 0x2588; /* xx xx .. -> xx xx   ! */
	case 0xF030: return L' ';   /* .. .. x. -> .. .. */
	case 0xF031: return 0x2598; /* x. .. x. -> x. .. */
	case 0xF032: return 0x259D; /* .x .. x. -> .x .. */
	case 0xF033: return 0x2580; /* xx .. x. -> xx .. */
	case 0xF034: return 0x2596; /* .. x. x. -> .. x. */
	case 0xF035: return 0x258C; /* x. x. x. -> x. x. */
	case 0xF036: return 0x259E; /* .x x. x. -> .x x. */
	case 0xF037: return 0x259B; /* xx x. x. -> xx x. */
	case 0xF038: return 0x2596; /* .. .x x. -> .. x. */
	case 0xF039: return 0x258C; /* x. .x x. -> x. x. */
	case 0xF03A: return 0x259E; /* .x .x x. -> .x x. */
	case 0xF03B: return 0x259B; /* xx .x x. -> xx x. */
	case 0xF03C: return 0x2596; /* .. xx x. -> .. x. */
	case 0xF03D: return 0x258C; /* x. xx x. -> x. x. */
	case 0xF03E: return 0x259E; /* .x xx x. -> .x x. */
	case 0xF03F: return 0x259B; /* xx xx x. -> xx x. */
	/* 0x20 sized gap */
	case 0xF060: return 0x2597; /* .. .. .x -> .. .x */
	case 0xF061: return 0x259A; /* x. .. .x -> x. .x */
	case 0xF062: return 0x2590; /* .x .. .x -> .x .x */
	case 0xF063: return 0x259C; /* xx .. .x -> xx .x */
	case 0xF064: return 0x2597; /* .. x. .x -> .. .x */
	case 0xF065: return 0x259A; /* x. x. .x -> x. .x */
	case 0xF066: return 0x2590; /* .x x. .x -> .x .x */
	case 0xF067: return 0x259C; /* xx x. .x -> xx .x */
	case 0xF068: return 0x2597; /* .. .x .x -> .. .x */
	case 0xF069: return 0x259A; /* x. .x .x -> x. .x */
	case 0xF06A: return 0x2590; /* .x .x .x -> .x .x */
	case 0xF06B: return 0x259C; /* xx .x .x -> xx .x */
	case 0xF06C: return 0x2597; /* .. xx .x -> .. .x */
	case 0xF06D: return 0x259A; /* x. xx .x -> x. .x */
	case 0xF06E: return 0x2590; /* .x xx .x -> .x .x */
	case 0xF06F: return 0x259C; /* xx xx .x -> xx .x */
	case 0xF070: return 0x2584; /* .. .. xx -> .. xx */
	case 0xF071: return 0x2599; /* x. .. xx -> x. xx */
	case 0xF072: return 0x259F; /* .x .. xx -> .x xx */
	case 0xF073: return 0x2588; /* xx .. xx -> xx xx */
	case 0xF074: return 0x2584; /* .. x. xx -> .. xx */
	case 0xF075: return 0x2599; /* x. x. xx -> x. xx */
	case 0xF076: return 0x259F; /* .x x. xx -> .x xx */
	case 0xF077: return 0x2588; /* xx x. xx -> xx xx */
	case 0xF078: return 0x2584; /* .. .x xx -> .. xx */
	case 0xF079: return 0x2599; /* x. .x xx -> x. xx */
	case 0xF07A: return 0x259F; /* .x .x xx -> .x xx */
	case 0xF07B: return 0x2584; /* xx .x xx -> xx xx */
	case 0xF07C: return 0x2584; /* .. xx xx -> .. xx */
	case 0xF07D: return 0x2599; /* x. xx xx -> x. xx */
	case 0xF07E: return 0x259F; /* .x xx xx -> .x xx */
	case 0xF07F: return 0x2588; /* xx xx xx -> xx xx */
	}

	return wc;
}

/* does nothing if no match */
static void
parsecolor(const char *str, const char *end, enum ttcolor *color)
{
	int	i;
	size_t	len;

#if DEBUG_PARSER
	printf("parsecolor: '");
	fwrite(str, end-str, 1, stdout);
	printf("'\n");
#endif

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

#if DEBUG_PARSER
	printf("parsecolors: '");
	fwrite(str, end-str, 1, stdout);
	printf("'\n");
#endif

	while (end - str > 3) {
		if (memcmp("bg-", str, 3) == 0) {
			color = &attrs->bg;
			str += 3;
		} else {
			color = &attrs->fg;
		}

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

#if DEBUG_HTML
	puts(html);
#endif

	p = html;
	while (line < TT_NLINES) {
#if DEBUG_PARSER
		if (isprint(*p))
			debug_parser("got '%c'\n", *p);
		else
			debug_parser("got %02x\n", *p);
#endif

		/* clear rest of line if EOL or EOF */
		if (*p == '\0' || *p == '\n') {
			debug_parser("filling up line\n");
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
			debug_parser("skipping hex character\n");
			wc = (wchar_t)strtol(p+3, (char **)&p, 16);
			if (!*p)
				p++; /* skip the ';' */
#if DEBUG_PARSER
			if (isprint(*p))
				debug_parser("now got '%c'\n", wc);
			else
				debug_parser("now got %02x\n", wc);
#endif
		} else {
			wc = *p;
		}

		switch (state) {
		case PS_IN_TEXT:
			switch (wc) {
			case '<':
				debug_parser("-> PS_IN_TAG\n");
				state = PS_IN_TAG;
				debug_parser("pushing curattrs\n");
				if (++attrdepth < LEN(attrstack)) {
					attrstack[attrdepth] = curattrs;
					debug_parser("curattrs[%i] = %x %x\n",
					    attrdepth, curattrs.fg,
					    curattrs.bg);
				}
				break;
			default:
				/* ignore input beyond line length */
				if (col >= TT_NCOLS) {
					debug_parser("skipping, past EOL\n");
				} else {
					debug_parser("assigning %i,%i\n",
					    line, col);
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
				debug_parser("popping curattrs (2x)\n");
				if ((attrdepth -= 2) < 0)
					attrdepth = 0;
				else if (attrdepth < LEN(attrstack)) {
					curattrs = attrstack[attrdepth+1];
					debug_parser("curattrs[%i] = %x %x\n",
					    attrdepth+1, curattrs.fg,
					    curattrs.bg);
				}
				break;
			case '"':
				debug_parser("-> PS_IN_ATTRQUOTES\n");
				state = PS_IN_ATTRQUOTES;
				openquote = p;
				break;
			case '>':
				debug_parser("-> PS_IN_TEXT\n");
				state = PS_IN_TEXT;
				break;
			}
			break;

		case PS_IN_ATTRQUOTES:
			switch (wc) {
			case '"':
				debug_parser("-> PS_IN_TAG\n");
				state = PS_IN_TAG;
				closequote = p;
				break;
			case '>':
				debug_parser("-> PS_IN_TEXT\n");
				state = PS_IN_TEXT;
				closequote = p;
				break;
			}
			if (state != PS_IN_ATTRQUOTES) {
				debug_parser("parsing color\n");
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
tt_get(const char *id, struct ttpage *page, const struct ttgetopts *opts)
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

	if (!opts)
		opts = &defopts;

	snprintf(url, LEN(url), "%s%s",
	    opts->endpoint ? opts->endpoint : defopts.endpoint,
	    id);
	url[LEN(url)-1] = '\0';

	json.tokener = json_tokener_new();

	if (!(curl = curl_easy_init())) {
		err = TT_ECURL;
		goto cleanup;
	}

	debug_io("fetching %s\n", url);
	
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlerrbuf);
	curl_easy_setopt(curl, CURLOPT_USERAGENT,
	    opts->useragent ? opts->useragent : defopts.useragent);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "application/json");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, jsonwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);
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

	debug_io("parsed, id=%s nextsub=%s nextnum=%s\n", page->id,
	    page->nextsub, page->nextpage);

	for (line = 0; line < TT_NLINES; line++) {
		for (col = 0; col < TT_NCOLS; col++) {
			wcp = &page->chars[line][col];
			*wcp = mapboxchar(*wcp, opts->mapmode);
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
