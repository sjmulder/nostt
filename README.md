nostt
=====
(Ook in het [Nederlands](README.nl.md))

Command line NOS Teletekst reader for Unix-likes and Windows. Supports
color and has an interactive mode.

**nostt** [*page*]

    $ nostt 102
                          NOS Teletekst 102 
           N O S   B I N N E N L A N D      
                                            
     KNSB:woensdag marathon op natuurijs 104
     Discriminatiecursus uitzendbranche. 105
                                            
     Trouwen in gemeenschap goedkoper... 106
     Media geweerd bij oorlogsherdenking 107
                                            
     Baby Hannah terecht en ongedeerd... 108
     Lichaam vermiste Orlando gevonden.. 109
                                            
     Politiemol Mark M. meldt zich...... 110
     16-jarige vast voor liquidaties.... 111
                                            
     Mies Bouwman (88) overleden........ 112
     "Koningin van de tv overleden"..... 113
     Nederlandse olympiers gehuldigd.... 114
    %%%%%%%%%%%%%% kort nieuws%%%%%%%%%%%%%%
     Treinverkeer A'dam kort ontregeld.. 121
     Vogelgriep op bedrijf in Groningen. 121
     Meisje mishandeld om hoofddoek..... 122
     Nederlandse skier (50) verongelukt. 122
       ticker 199              politie 147  
                          NOS Teletekst 102 

Description
-----------
Displays the requested [NOS Teletekst](https://nos.nl/teletekst) page on
the command line. Pages are identified by a three-digit number
optionally followed by a subpage, for example 100 or 302-2. All subpages
are printed if not specified.

If no *page* argument is given, nostt runs interactively.

See the manual page for more details: `man nostt`.

Known issues
------------
Teletext's 6-cell (2x3) block characters are displayed as % characters
as they cannot be accurately reproduced in a terinal without special
fonts.  (The NOS viewer and API uses a custom font with these characters
in the "private use" 0xF000 Unicode range.)

Color output is disabled by default in MinTTY on Windows. This is
because MinTTY redirects standard output. Set `CLICOLOR_FORCE` to 1 to
force color output: `env CLICOLOR_FORCE=1 nostt`

Installation
------------
Should work on Windows and any Unix, including Linux and macOS.

**macOS** with [Homebrew](https://brew.sh) from
[my tap](https://github.com/sjmulder/homebrew-tap):

    brew install sjmulder/tap/nostt

**NetBSD** (and more, using [pkgsrc](https://pkgsrc.org)):

    pkg_add nostt

**FreeBSD**:

    pkg install nostt

From **source**:

 1. Download and extract the source code of the
    [latest release](https://github.com/sjmulder/nostt/releases).

 2. Install the dependencies, e.g. on Debian or Ubuntu:

        sudo apt install build-essential libcurl4-openssl-dev libjson-c-dev

 3. In the source directory:

        make
        sudo make install

    See the Makefile for more options. Uninstall with
    `sudo make uninstall`.

Building on Windows is supported through
[MinGW](http://mingw-w64.org/doku.php). A CMake build file is also
included for convenience.

Authors
-------
Sijmen J. Mulder (<ik@sjmulder.nl>)
