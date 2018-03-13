#ifdef _WIN32
# define COMPAT_ERR
# define COMPAT_GETLINE
#endif

#ifdef COMPAT_ERR
void	errx(int eval, const char *fmt, ...);
void	warnx(const char *fmt, ...);
#endif

#ifdef COMPAT_GETLINE
ssize_t	getline(char **linep, size_t *linecapp, FILE *stream);
#endif

extern char *argv0;
