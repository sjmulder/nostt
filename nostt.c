/* nostt.c - Copyright (c) 2018, Sijmen J. Mulder (see LICENSE.md) */

#define USAGE	"usage: nostt [page]"

#define _WITH_GETLINE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "api.h"
#include "compat.h"

#ifdef _WIN32
# include <windows.h>
#endif

#ifndef COMPAT_ERR
# include <err.h>
#endif

static void
enablecolor()
{
#ifdef _WIN32
	HANDLE	hout;
	DWORD	mode	= 0;

	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(hout, &mode);
	/* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
	SetConsoleMode(hout, mode|4);
#endif
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
		printf("\033[%d;%dm", 30 + attrs->fg, 40 + attrs->bg);

	putwchar(page->chars[line][col]);
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
		fflush(stdout);

		if ((len = getline(&input, &cap, stdin)) == -1)
			return NULL;
		if (len && input[len-1] == '\n')
			input[--len] = '\0';

		if (!strcmp("q", input))
			return NULL;
		else if (len)
			return input;
		else
			return suggestion;
	}
}

int
main(int argc, char **argv)
{
	const char	*id;
	struct ttpage 	 page;
	enum tterr	 ret;
	int		 withcolor	= 0;
	int		 interactive	= 0;
	int		 line, col;

#ifdef __OpenBSD__
	if (pledge("stdio rpath inet dns", NULL) == -1)
		err(1, "pledge");
#endif

	(void) argc;

	argv0 = *argv;
	setlocale(LC_ALL, "");

	if (argv[1]) {
		if (argv[2] || *argv[1] == '-')
			errx(1, USAGE);
		id = argv[1];
	} else {
		id = "100";
		interactive = 1;
	}

	withcolor =
	    enveq("CLICOLOR_FORCE", "1") ||
	    (isatty(STDOUT_FILENO) && !enveq("CLICOLOR", "0"));

	if (withcolor)
		enablecolor();

	while (1) {
		ret = tt_get(id, &page);
		if (ret != TT_OK) {
			if (!interactive)
				errx(1, "%s", tt_errstr(ret));

			warnx("%s", tt_errstr(ret));
			puts("(q or ^C to exit)\n");
			if (!(id = prompt(NULL)))
				return 0;
			continue;
		}

		if (withcolor) {
			for (line = 0; line < TT_NLINES; line++) {
				for (col = 0; col < TT_NCOLS; col++)
					putcell_color(&page, line, col);
				puts("\033[0m");
			}
		} else {
			for (line = 0; line < TT_NLINES; line++)
				printf("%.*ls\n", TT_NCOLS,
				    page.chars[line]);
		}

		if (interactive) {
			putchar('\n');
			id = prompt(
			    *page.nextsub ? page.nextsub :
			    *page.nextpage ? page.nextpage : NULL);
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
}
