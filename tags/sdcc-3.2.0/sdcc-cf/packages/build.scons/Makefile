include ../packages.mk

PACKAGE=scons

VERSION=2.0.1

.PHONY: all
all: $(PACKAGE)-install.tag clean

download/$(PACKAGE)-$(VERSION).tar.gz:
	wget -P download http://sourceforge.net/projects/$(PACKAGE)/files/$(PACKAGE)/$(VERSION)/$(PACKAGE)-$(VERSION).tar.gz

$(PACKAGE)-unpack.tag: download/$(PACKAGE)-$(VERSION).tar.gz
	tar xzvf download/$(PACKAGE)-$(VERSION).tar.gz
	touch $@

$(PACKAGE)-install.tag: $(PACKAGE)-unpack.tag
	cd $(PACKAGE)-$(VERSION) && \
	python setup.py install --prefix=$(LOCAL_DIR)
	touch $@

clean:
	rm -rf $(PACKAGE)-$(VERSION) download/* *.tag
