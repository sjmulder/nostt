## TBD

 - Option -i (interactive mode) was removed. Interactive mode is now used if
   no page argument is given.
 - Option -G (color output) was removed. Color output is the default now for
   interactive terminals, but can be disabled with environment CLICOLOR=0 or
   forced with CLICOLOR_FORCE=1.
 - Tweaked default CFLAGS and LDFLAGS, in particular adding /usr/local search
   paths.
 - make [un]install now respects DESTDIR. prefix, bindir, and man1dir have
   been replaced with PREFIX and MANPREFIX.
 - Several portability fixes.

## 1.0 (2018-03-04)

Initial release.
