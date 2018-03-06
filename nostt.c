/* nostt.c - Copyright (c) 2018, Sijmen J. Mulder (see LICENSE.md) */

#define USAGE	"usage: nostt [-G] [page]"

#ifdef __MINGW32__
# define COMPAT_ERR
# define COMPAT_GETLINE
#endif

#define _WITH_GETLINE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "api.h"

#ifdef _WIN32
# include <windows.h>
#endif

#ifndef COMPAT_ERR
# include <err.h>
#endif

static char *argv0;

#ifdef COMPAT_ERR
static void
errx(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "%s ", argv0);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);

	exit(eval);
}

static void
warnx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "%s ", argv0);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);
}
#endif

#ifdef COMPAT_GETLINE
static ssize_t
getline(char **linep, size_t *linecapp, FILE *stream)
{
	size_t	len = 0;
	int	ch;

	fflush(stdout);

	if (!*linep) {
		*linecapp = 128;
		if (!(*linep = malloc(*linecapp)))
			return -1;
	}

	while (1) {
		if (len+2 >= *linecapp) {
			while (len+2 >= (*linecapp *= 2))
				;
			if (!(*linep = realloc(*linep, *linecapp)))
				return -1;
		}

		ch = fgetc(stream);
		if (ch != -1)
			(*linep)[len++] = (int)ch;

		if (ch == -1 || ch == '\n') {
			(*linep)[len] = '\0';
			return len;
		}
	}
}
#endif

static void
enablecolor()
{
#ifdef _WIN32
	HANDLE	hout;
	DWORD	mode	= 0;

	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(hout, &mode);
	SetConsoleMode(hout, mode|4); /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
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
		printf("\e[%d;%dm", 30 + attrs->fg, 40 + attrs->bg);

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
	int		 colorflag	= 0;
	int		 interactive	= 0;
	int		 line, col;

	argv0 = *argv;
	setlocale(LC_ALL, "");

	while ((c = getopt(argc, argv, "G")) != -1) {
		switch (c) {
		case 'G':
			colorflag = 1;
			break;
		default:
			errx(1, USAGE, c);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 1)
		errx(1, USAGE);

	if (!(id = *argv)) {
		id = "100";
		interactive = 1;
	}

	if (!colorflag)
		colorflag =
		    enveq("CLICOLOR_FORCE", "1") ||
		    (isatty(STDOUT_FILENO) && enveq("CLICOLOR", "1"));
	if (colorflag)
		enablecolor();

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
}
