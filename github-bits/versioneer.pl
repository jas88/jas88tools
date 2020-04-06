#!/usr/bin/perl -w

use strict;

while(<>) {
  chomp;
  warn $_ unless /"([0-9a-z.]+)".+"([0-9a-z.]+)"/i;
  next unless //;
  print "Update $1 to $2\n";
}