#+##############################################################################
#                                                                              #
# File: Makefile                                                               #
#                                                                              #
# Description: toplevel Makefile                                               #
#                                                                              #
#-##############################################################################

# Author: Lionel Cons (http://cern.ch/lionel.cons)
# Copyright (C) CERN 2012-2016

VERSION=$(shell cat VERSION)
PKGDIR=libdirq-$(VERSION)
TARBALL=$(PKGDIR).tar.gz
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
