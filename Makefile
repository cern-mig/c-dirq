#+##############################################################################
#                                                                              #
# File: Makefile                                                               #
#                                                                              #
# Description: toplevel Makefile                                               #
#                                                                              #
#-##############################################################################

# $Id: Makefile,v 1.6 2016/01/13 09:37:10 c0ns Exp $

VERSION=$(shell cat VERSION)
PKGDIR=libdirq-$(VERSION)
TARBALL=$(PKGDIR).tgz
PKGFILES=CHANGES DESIGN LICENSE README VERSION \
 Makefile configure doc src libdirq.spec

.PHONY: all test install clean distclean sources

all test install: src/Makefile
	make -C src $@

src/Makefile: src/Makefile.in
	./configure

sources: $(TARBALL)

$(TARBALL):
	@make -C doc all clean
	@[ -e src/Makefile ] && make -C src distclean || true
	@tempdir=`mktemp -d -t c-dirq-XXXXX`; \
	mkdir $$tempdir/$(PKGDIR); \
	cp -a $(PKGFILES) $$tempdir/$(PKGDIR); \
	( cd $$tempdir; tar cvfz $(TARBALL) --exclude "RCS" $(PKGDIR) ); \
	mv -f $$tempdir/$(TARBALL) $(TARBALL); \
	rm -fr $$tempdir

clean:
	@make -C doc clean
	@[ -e src/Makefile ] && make -C src clean || true
	rm -f *~

distclean:
	@make -C doc distclean
	@[ -e src/Makefile ] && make -C src distclean || true
	rm -f *~ $(TARBALL)
