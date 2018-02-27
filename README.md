nostt
=====

Command line NOS Teletekst reader. Supports color and has an interactive mode.

**nostt** [**-iG**] [*page*]

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
     Nederlandse olympiërs gehuldigd.... 114
    %%%%%%%%%%%%%% kort nieuws%%%%%%%%%%%%%%
     Treinverkeer A'dam kort ontregeld.. 121
     Vogelgriep op bedrijf in Groningen. 121
     Meisje mishandeld om hoofddoek..... 122
     Nederlandse skiër (50) verongelukt. 122
       ticker 199              politie 147  
                          NOS Teletekst 102 

Description
-----------

Displays the requested NOS Teletekst page on the command line. Pages are
identified by a three-digit number optionally followed by a subpage, for
example 100 or 302-2.

The *page* argument is required unless using interactive mode. When a
subpage is given, nostt prints only that subpage. Otherwise, all
subpages are printed.

Options
-------

**-i**

Run in interactive mode. A prompt will appear after every page,
asking for the next page to display. Exit by sending an
interrupt signal (usually Ctrl+C).

**-G**

Enable color output. Default if standard output is a terminal
and `CLICOLOR` is set to `1`, or if `CLICOLOR_FORCE` is set to `1`.

Teletext graphics
-----------------

Teletext supports 6-cell (2x3) block drawing characters. The NOS viewer
and API use a custom font with these characters in the "private use"
0xF000 Unicode range.

Because such characters are not usually available on terminals, they are
replaced by `%` characaters.

Installation
------------

**nostt** should work on most Unix-like systems. It depends on
[libcurl](https://curl.haxx.se/libcurl/) and
[json-c](https://github.com/json-c/json-c)
Once these are installed, modify the Makefile as desired. Then:

    make

To install or uninstall, setting PREFIX as desired and switching to root if
necessary:

    PREFIX=/usr/local make install
    PREFIX=/usr/local make uninstall

Links
-----

Project page: https://github.com/sjmulder/nostt

Official NOS Teletekst web client: https://nos.nl/teletekst

Authors
-------

Sijmen J. Mulder (<ik@sjmulder.nl>).
