#!/usr/bin/perl -w

use strict;
use MIME::Base64;

# Debian: apt install libfile-changenotify-watcher
use File::ChangeNotify;
use File::ChangeNotify::Watcher;

my $watcher =
File::ChangeNotify->instantiate_watcher
( directories => [ '/var/lib/samba/private/sam.ldb.d' ],
filter      => qr/^DC=/
);

while(1) {
	open(USERS,"samba-tool user list|") || die "user list:$!\n";
	while(<USERS>) {
		chomp;
		my ($username,$email,$pw)=($_,undef,undef);
		open(my $userdata,"-|","samba-tool user getpassword $_ --attributes=virtualClearTextUTF8,mail");
		while(<$userdata>) {
			chomp;
			$email=$1 if /^mail:\s+(.+)$/;
			$pw=decode_base64($1) if /^virtualClearTextUTF8::\s+([A-Za-z0-9+\/=]+)$/;
		}
		print "$email:$pw\n" if defined $pw && defined $email;
		close($userdata);
	}
	close(USERS);
	$watcher->wait_for_events();
}
