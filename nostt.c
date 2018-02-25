#define DEBUG_CHARS	0
#define DEBUG_COLORS	0

#define ENABLE_COLOR	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <err.h>
#include "api.h"

#if ENABLE_COLOR
static int
mapfg(enum ttcolor c)
{
	switch (c) {
	case TT_BLACK:		return 30;
	case TT_BLUE:		return 34;
	case TT_GREEN:		return 32;
	case TT_CYAN:		return 36;
	case TT_RED:		return 31;
	case TT_MAGENTA:	return 35;
	case TT_YELLOW:		return 33;
	default:		return 37;
	}
}

static int
mapbg(enum ttcolor c)
{
	switch (c) {
	case TT_BLACK:		return 40;
	case TT_BLUE:		return 44;
	case TT_GREEN:		return 42;
	case TT_CYAN:		return 46;
	case TT_RED:		return 41;
	case TT_MAGENTA:	return 45;
	case TT_YELLOW:		return 43;
	default:		return 47;
	}
}
#endif /* ENABLE_COLOR */

static void
putcell(struct ttpage *page, int line, int col)
{
	wchar_t		 wc;
#if ENABLE_COLOR
	struct ttattrs	*attrs;
	struct ttattrs	*prevattrs;
	int		 colorflag;

	attrs = &page->attrs[line][col];
	if (!col) {
		colorflag = 1;
	} else {
		prevattrs = &page->attrs[line][col-1];
		colorflag = memcmp(attrs, prevattrs, sizeof(*attrs));
	}

	if (colorflag)
		printf("\e[%d;%dm", mapfg(attrs->fg), mapbg(attrs->bg));
#endif /* ENABLE_COLOR */

	wc = page->chars[line][col];
#if DEBUG_CHARS
	printf("%04x %lc ", wc);
#elif DEBUG_COLORS
	printf("%x%x %lc ", attrs->fg, attrs->bg, wc);
#else
	printf("%lc", wc);
#endif
}

int
main(int argc, char **argv)
{
	const char	*id;
	struct ttpage 	 page;
	enum tterr	 ret;
	int		 line;
	int		 col;

	setlocale(LC_ALL, "");

	if (argc != 2 || !*argv[1])
		errx(1, "usage: nostt <page>");
	
	id = argv[1];
	while (*id) {
		ret = tt_get(id, &page, NULL);
		if (ret != TT_OK)
			errx(1, "%s", tt_errstr(ret));
		
		for (line = 0; line < TT_NLINES; line++) {
			for (col = 0; col < TT_NCOLS; col++)
				putcell(&page, line, col);

#if ENABLE_COLOR
			puts("\e[0m");
#else
			putchar('\n');
#endif
		}

		/* If a subpage was requested (e.g. 101-2), don't print any
		   further pages. */
		if (id == argv[1] && strchr(id, '-'))
			break;

		putchar('\n');
		id = page.nextsub;
	}

	return 0;
}
