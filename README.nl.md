nostt
=====
(Also in [English](README.md))

Lees NOS Teletekst op de commandline. Met ondersteuning voor kleur en
een interacdtieve modus.

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

Overzicht
---------
Schrijft de gegeven [teletekstpagina](https://nos.nl/teletekst) naar de 
terminal. De pagina moet worden opgegeven als driecijferig nummer,
optioneel gevolgd door een streepje en schermnummer. Bijvoorbeeld: 100
of 302-2. Als er geen schermnummer wordt opgegeven worden alle schermen
weergeven.

Als er geen paginanummer wordt opgegeven werkt nostt in interactieve
modus.

Zie de manpagina voor meer details: `man nostt`.

Bekende problemen
-----------------
De speciale teletekst-lettertekens voor het maken van lijnen en blokken
worden niet door reguliere lettertypen ondersteund en daarom weergeven
als procentekens: `%`. (De NOS
Teletekst-[website](https://nos.nl/teletekst) gebruikt een speciaal
lettertype met deze tekens in de 0xF000 *Special Use*-categorie van
Unicode.)

In MinTTY op Windows staan kleuren standaard uit. Dit is omdat MinTTY
de standaarduitvoer afvangt. Zet omgevingsvariabele `CLICOLOR_FORCE` op
1 om kleur in te schakelen: `env CLICOLOR_FORCE=1 nostt`.

Installatie
-----------
Zou moeten werken op Windows en Unix, inclusief Linux en macOS.

**macOS** met [Homebrew](https://brew.sh) vanaf
[mijn tap](https://github.com/sjmulder/homebrew-tap):

    brew install sjmulder/tap/nostt

**NetBSD** (e.a. via [pkgsrc](https://pkgsrc.org)):

    pkg_add nostt

**FreeBSD**:

    pkg install nostt

Vanaf **broncode**: 

 1. Download de broncode van de
    [laatste release](https://github.com/sjmulder/nostt/releases) en pak
    deze uit.
 
 2. Installeer de afhankelijkheden, bijv. op Debian of Ubuntu:

        sudo apt install build-essential libcurl4-openssl-dev libjson-c-dev

 3. In de directory met de broncode:

        make
        sudo make install

    Zie de Makefile voor meer opties. Deinstallatie kan met
    `sudo make uninstall`.

Op **Windows** is compilatie mogelijk met
[MinGW](http://mingw-w64.org/doku.php). Voor het gemak is er ook een
CMake-projectbestand aanwezig.

Auteur
------
Sijmen J. Mulder (<ik@sjmulder.nl>)
