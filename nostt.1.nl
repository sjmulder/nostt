.Dd 5 augustus 2023
.Dt NOSTT 1
.Os
.Sh NAAM
.Nm nostt
.Nd lees NOS Teletekst op de commandline
.Sh GEBRUIK
.Nm
.Op Ar pagina
.Sh BESCHRIJVING
Toont de gevraagde NOS Teletekst-pagina op de commandline.
Pagina's worden benoemd met een driecijferig nummer
en optioneel een subpagina, bijvoorbeeld 100 of 302.
Als er geen subpagina is opgegeven, worden alle pagina's weergeven.
.Pp
Zonder
.Ar pagina Ns
-argument
werkt
.Nm
interactief.
.Sh OMGEVINGSVARIABELEN
.Bl -tag -width LC_MONETARY
.It Ev NO_COLOR
Als de waarde
.Dq 0
is wordt er geen kleur gebruikt.
.It Ev CLICOLOR
Als de waarde
.Dq 1
is wordt er geen kleur gebruikt.
.It Ev CLICOLOR_FORCE
Als de waarde
.Dq 1
is wordt er kleur gebruikt
zelfs wanner de uitvoer een pipe of bestand is.
Merk op dat
.Ev NO_COLOR=1 en
.Ev CLICOLOR=0
prioriteit hebben.
.El
.Sh ZIE OOK
Projectpagina:
.Lk https://github.com/sjmulder/nostt
.Pp
Officiële NOS Teletekst-webapplicatie:
.Lk https://nos.nl/teletekst
.Sh AUTEURS
.An Sijmen J. Mulder
.Aq Mt ik@sjmulder.nl .
.Sh BUGS
Teletekst 6-cel (2x3) bloktekens worden weergeven als %-tekens
omdat deze, zonder gebruik van speciale lettertypen,
niet accuraat in een terminal weergeven kunnen worden.
(De webapplicatie en API van de NOS gebruiken zo'n speciaal lettertype
met deze lettertypen in een Unicode-bereik
gereserveerd voor privegebruik.)
.Pp
In MinTTY in Windows wordt er standaard geen kleur gebruikt
omdat de uitvoer wordt omgeleid.
Gebruik
.Ev CLICOLOR_FORCE=1
om kleur te forceren.
