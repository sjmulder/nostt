#define TT_NLINES	24
#define TT_NCOLS	40

/* 3 color RGB */
enum ttcolor {
	TT_BLACK,
	TT_BLUE,
	TT_GREEN,
	TT_CYAN,
	TT_RED,
	TT_MAGENTA,
	TT_YELLOW,
	TT_WHITE
};

enum tterr {
	TT_OK,
	TT_EARG,
	TT_ECURL,
	TT_EAPI,
	TT_EDATA
};

enum ttmapmode {
	TT_MDEFAULT,
	TT_MASCII = TT_MDEFAULT, /* remap all chars >127 to a substitute */
	TT_MNONE,	/* 2x3 box drawing chars in 0xF000 range */
	TT_MUNICODE,	/* remap to 2x2 Unicode block drawing chars */
};

struct ttattrs {
	enum ttcolor	fg;
	enum ttcolor	bg;
};

/* Note on block drawing characters:

   Teletext supports 6-cell (2x3) block drawing characters. The NOS viewer
   and API use a custom font with these characters in the 0xF000 Unicode
   range ('private use').

   These are all mapped to SUBST_CHAR, defined in api.c */
struct ttpage {
	wchar_t		chars[TT_NLINES][TT_NCOLS];
	struct ttattrs	attrs[TT_NLINES][TT_NCOLS];
	char		id[6];
	char		nextpage[6];
	char		nextsub[6];
};

enum tterr	 tt_get(const char *id, struct ttpage *page);
const char	*tt_errstr(enum tterr err);
