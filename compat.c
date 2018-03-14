#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "compat.h"

char *argv0;

#ifdef COMPAT_ERR
void
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

void
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
ssize_t
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
