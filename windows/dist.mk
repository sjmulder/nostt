# use from the project root, e.g.:
#   nostt$ mingw32-make -f windows/dist.mk dist

include Makefile

VERSION?=	dev
DISTDIR?=	nostt-$(VERSION)
ARCHIVE?=	$(DISTDIR).windows.zip
DLLDIR?=	$(MINGW_PREFIX)/bin

DLLS=		libcurl-4.dll libeay32.dll libffi-6.dll libgmp-10.dll \
		libgnutls-30.dll libhogweed-4.dll libiconv-2.dll \
		libidn-11.dll libidn2-0.dll libintl-8.dll libjson-c-2.dll \
		libnettle-6.dll libnghttp2-14.dll libp11-kit-0.dll \
		librtmp-1.dll libssh2-1.dll libtasn1-6.dll \
		libunistring-2.dll ssleay32.dll zlib1.dll

clean: clean-dist

clean-dist:
	rm -rf $(DISTDIR) $(ARCHIVE)

dist: nostt
	mkdir -p $(DISTDIR)
	cp $(addprefix $(DLLDIR)/,$(DLLS)) $(DISTDIR)
	cp nostt.exe $(DISTDIR)
	cp windows/header.html $(DISTDIR)/README.html
	markdown README.md >> $(DISTDIR)/README.html
	cp windows/header.html $(DISTDIR)/LICENSE.html
	markdown LICENSE.md >> $(DISTDIR)/LICENSE.html
	rm -f $(ARCHIVE)
	zip -r $(ARCHIVE) $(DISTDIR)

.PHONY: clean-dist dist
