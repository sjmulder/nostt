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

/* Note on block drawing characters:

   Teletext supports 6-cell (2x3) block drawing characters. The NOS viewer
   and API use a custom font with these characters in the 0xF000 Unicode
   range ('private use').

   Unicode does not have 6-cell box drawing characters, but does include
   4-cell characters. An attempt can be made to map from 6-cell to 4-cell
   characters like so:

    .x      .x
    .x  ->  xx
    xx

   This is the TT_MUNICODE mode below. The other options are to leave the
   characters as-is (requiring a custom font) and replacement with a
   placeholder character (TT_MASCII, '.'). */
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

struct ttpage {
	wchar_t		chars[TT_NLINES][TT_NCOLS];
	struct ttattrs	attrs[TT_NLINES][TT_NCOLS];
	char		id[6];
	char		nextpage[6];
	char		nextsub[6];
};

struct ttgetopts {
	const char	*useragent;	/* NULL for default */
	const char	*endpoint;	/* NULL for default */
	enum ttmapmode	 mapmode;	/* 0 = TT_MDEFAULT, see ttmapmode */
};

enum tterr	 tt_get(const char *id, struct ttpage *page,
		    const struct ttgetopts *opts /* optional */);
const char	*tt_errstr(enum tterr err);
