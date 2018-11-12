TBD
---
 - Removed: -i option (interactive mode). Interactive mode is now used if no
   page argument is given.
 - Removed: -G option (color ouptut). Color output is the default now for
   interactive terminals, but can be disabled with environment CLICOLOR=0 or
   forced with CLICOLOR_FORCE=1.
 - Changed: default CFLAGS and LDFLAGS, in particular adding /usr/local search
   paths.
 - Changed: make [un]install now respects DESTDIR. prefix, bindir, and man1dir
   have been replaced with PREFIX and MANPREFIX.
 - Fixed: several portability issues.

1.0 (2018-03-04)
----------------
Initial release.
