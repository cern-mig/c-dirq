#+##############################################################################
#                                                                              #
# File: Makefile                                                               #
#                                                                              #
# Description: Makefile for the documentation                                  #
#                                                                              #
#-##############################################################################

# Author: Lionel Cons (http://cern.ch/lionel.cons)
# Copyright (C) CERN 2012-2016

DIRQH=../src/dirq.h
FILES=dirq.pod dirq.html dirq.3
VERSION=$(shell cat ../VERSION)

.PHONY: all clean distclean

all: $(FILES)

dirq.pod: dirq.p0d $(DIRQH)
	@rm -f $@
	./pp dirq.p0d $(DIRQH) > $@

dirq.html: dirq.pod
	pod2html --noindex --infile=$^ --outfile=$@

dirq.3: dirq.pod
	pod2man -c '' -n dirq -s 3 -r 'dirq $(VERSION)' $^ $@

clean:
	rm -f pod2htm*.tmp *~

distclean: clean
	rm -f $(FILES)
