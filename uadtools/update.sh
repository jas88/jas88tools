#!/bin/sh

# Auto-update script - to be run from Cron on hacklab1/Hannah
# Pulls latest boot image and management scripts from JS

# Dependencies:
# HTTP daemon on :80 serving /lab/boot as :80/boot, including PHP
# tftp sha256sum curl

set -e
mkdir /tmp/$$
cd /lab/boot
curl -s https://us.deadnode.org/tc.xz.sha256 > /tmp/$$/sums
if ! sha256sum --status -c /tmp/$$/sums
then
	cd /tmp/$$
	curl -s https://us.deadnode.org/tc.xz > tc.xz
	curl -s https://us.deadnode.org/boot.txt > boot.txt
	sha256sum --status -c sums
	xzcat tc.xz > /dev/null
	php -l boot.txt > /dev/null
	mv tc.xz /lab/boot/
	mv boot.txt /lab/boot/boot.php
fi
rm /tmp/$$/sums
rmdir /tmp/$$
