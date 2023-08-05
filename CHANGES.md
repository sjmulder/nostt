1.3 (2023-08-03)
----------------
 - Fixed: typo and style in man page (thanks Omar Polo).
 - New: Dutch translation of the man page.
 - New: unveil() support on OpenBSD (thanks Tim van der Molen).
 - New: support for NO_COLOR=1 (http://no-color.org).
 - New: non-canonical CMake build file.
 - Change: migrated builds from sr.ht to GitHub Actions.

1.2.2 (2022-06-15)
------------------
 - Fixed: API error caused by use of old HTTP endpoint. Switched to
   same endpoint but HTTPS.

1.2.1 (2020-07-25)
------------------
 - New: Dutch translation of README.md.
 - Fixed: interactive prompt not printed with musl libc.
 - Fixed: typos in README.md.
 - Change: migrated builds from Travis to sr.ht.

1.2 (2020-01-02)
----------------
 - Fixed: build warning from mismatched callback signature.
 - Fixed: only first column printed with colors off.
 - New: type 'q' at the prompt to quit.
 - New: next page suggested after last subpage (e.g. [101] after [101-4]).
 - Change: drop /usr/local from default include and lib search path. System
   search paths aren't for us to decide.

1.1.1 (2018-12-06)
------------------
 - Fixed: wrong implicit permissions in `make install`.

1.1 (2018-11-12)
----------------
 - New: 'make check' target.
 - Removed: -i option (interactive mode). Interactive mode is now used if no
   page argument is given.
 - Removed: -G option (color ouptut). Color output is the default now for
   interactive terminals, but can be disabled with environment CLICOLOR=0 or
   forced with CLICOLOR_FORCE=1.
 - Changed: default CFLAGS and LDFLAGS, in particular adding /usr/local
   search paths.
 - Changed: make [un]install now respects DESTDIR. prefix, bindir, and
   man1dir have been replaced with PREFIX and MANPREFIX.
 - Fixed: several portability issues.

1.0 (2018-03-04)
----------------
Initial release.
