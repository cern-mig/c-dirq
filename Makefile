#+##############################################################################
#                                                                              #
# File: Makefile                                                               #
#                                                                              #
# Description: toplevel Makefile                                               #
#                                                                              #
#-##############################################################################

# Author: Lionel Cons (http://cern.ch/lionel.cons)
# Copyright (C) CERN 2012-2017

VERSION=$(shell cat VERSION)
PKGTAG=v${VERSION}
PKGDIR=libdirq-$(VERSION)
TARBALL=$(PKGDIR).tar.gz
PKGFILES=CHANGES DESIGN LICENSE README.md VERSION \
 Makefile configure doc src libdirq.spec

.PHONY: all test install sources tag clean distclean

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
	( cd $$tempdir; tar cvfz $(TARBALL) $(PKGDIR) ); \
	mv -f $$tempdir/$(TARBALL) $(TARBALL); \
	rm -fr $$tempdir

tag:
	@seen=`git tag -l | grep -Fx ${PKGTAG}`; \
	if [ "x$$seen" = "x" ]; then \
	    set -x; \
	    git tag ${PKGTAG}; \
	    git push --tags; \
	else \
	    echo "already tagged with ${PKGTAG}"; \
	fi

clean:
	@make -C doc clean
	@[ -e src/Makefile ] && make -C src clean || true
	rm -f *~

distclean:
	@make -C doc distclean
	@[ -e src/Makefile ] && make -C src distclean || true
	rm -f *~ $(TARBALL)
