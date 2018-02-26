/* nostt.c - Copyright (c) 2018, Sijmen J. Mulder (see LICENSE.md) */

#define USAGE	"usage: nostt [-iG] [page]"

#define _WITH_GETLINE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <err.h>
#include "api.h"

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

static int
enveq(const char *name, const char *val)
{
	char	*actual;

	return (actual = getenv(name)) && strcmp(actual, val) == 0;
}

static void
putcell_color(struct ttpage *page, int line, int col)
{
	wchar_t		 wc;
	struct ttattrs	*attrs;
	struct ttattrs	*prevattrs;
	int		 attrsflag;

	attrs = &page->attrs[line][col];
	if (!col)
		attrsflag = 1;
	else {
		prevattrs = &page->attrs[line][col-1];
		attrsflag = memcmp(attrs, prevattrs, sizeof(*attrs));
	}

	if (attrsflag)
		printf("\e[%d;%dm", mapfg(attrs->fg), mapbg(attrs->bg));

	wc = page->chars[line][col];
	printf("%lc", wc);
}

static const char *
prompt(const char *suggestion)
{
	static char	*input = NULL;
	static ssize_t	 len;
	static size_t	 cap;

	while (1) {
		if (suggestion)
			printf("page [%s]? ", suggestion);
		else
			printf("page? ");

		if ((len = getline(&input, &cap, stdin)) == -1)
			return NULL;
		else if (!len) {
			if (suggestion)
				return suggestion;
		} else if (input[len-1] == '\n') {
			input[--len] = '\0';
			if (len)
				return input;
			else if (suggestion)
				return suggestion;
		}
	}
}

int
main(int argc, char **argv)
{
	const char	*id;
	int		 c;
	struct ttpage 	 page;
	enum tterr	 ret;
	int		 colorflag;
	int		 interactive	= 0;
	int		 line, col;

	setlocale(LC_ALL, "");

	colorflag =
	    enveq("CLICOLOR_FORCE", "1") ||
	    (isatty(STDOUT_FILENO) && enveq("CLICOLOR", "1"));

	while ((c = getopt(argc, argv, "iG")) != -1) {
		switch (c) {
		case 'i':
			interactive = 1;
			break;
		case 'G':
			colorflag = 1;
			break;
		default:
			errx(1, USAGE);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 1)
		id = argv[0];
	else if (argc || !interactive)
		errx(1, USAGE);
	else
		id = "100";

	while (1) {
		ret = tt_get(id, &page);
		if (ret != TT_OK) {
			if (!interactive)
				errx(1, "%s", tt_errstr(ret));

			warnx("%s", tt_errstr(ret));
			puts("(press ^C to exit)\n");
			if (!(id = prompt(NULL)))
				return 0;
			continue;
		}

		if (colorflag) {
			for (line = 0; line < TT_NLINES; line++) {
				for (col = 0; col < TT_NCOLS; col++)
					putcell_color(&page, line, col);
				puts("\e[0m");
			}
		} else {
			for (line = 0; line < TT_NLINES; line++)
				printf("%.*ls\n", TT_NCOLS, page.chars[line]);
		}

		if (interactive) {
			putchar('\n');
			id = prompt(*page.nextsub ? page.nextsub : NULL);
			if (!id)
				return 0;
		} else if (id == argv[0] && strchr(id, '-')) {
			/* Only print requested subpage */
			return 0;
		} else if (*page.nextsub) {
			putchar('\n');
			id = page.nextsub;
		} else
			return 0;
	}

	return 0;
}
