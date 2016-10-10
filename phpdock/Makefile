PHPVER	:= 7.0.4
OUTDIR	:= $(CURDIR)/out


.PHONY:	fetch

fetch:
	curl -L http://php.net/get/php-$(PHPVER).tar.xz/from/this/mirror | tar xJf -

config:
	mkdir -p $(OUTDIR)
	cd php-$(PHPVER) && ./configure --prefix=$(OUTDIR) --enable-fpm --with-fpm-acl --enable-exif --enable-ftp --enable-intl --enable-embedded-mysqli --enable-zip --with-pic --enable-static --disable-shared

build:	config
	$(MAKE) -C php-$(PHPVER) fpm install
