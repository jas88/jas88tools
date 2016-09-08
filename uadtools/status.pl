#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;
my $dirname = dirname(__FILE__);

my $lib=do {
    local $/ = undef;
    open my $fh, "<", $dirname.'/status.js'
        or die "could not open ".$dirname."/status.js: $!";
    <$fh>;
};
open(LOG,"access.log") || open(LOG,"/var/log/nginx/access.log") || die "Can't access log:$!\n";
print <<EOH;
<!DOCTYPE HTML>
<html lang="en-US">
<head>
	<meta charset="UTF-8">
	<title>Netlab Machine Status</title>
</head>
<body>
<table>
<thead><tr><th>Machine</th><th>Time</th>Status</th></tr></thead>
<tbody>
EOH

my %status;
while(<LOG>) {
	next unless /Syslinux/;
	next if /libcom32\.c32/;
	my ($ip,$j1,$j2,$time,$tz,$verb,$url,$httpver,$ok,$size,$ref)=split(/ /);
	$status{$ip}=$time.' '.$url;
}

foreach (sort keys %status) {
	next unless /^\d+\.\d+\.\d+\.(\d+)$/;
	my $mname='N'.$1;
	my $mode=$status{$_};
	if    ($mode =~/chain.c32$/    ) { $mode=~s/chain\.c32/Windows\/Linux/; }
	elsif ($mode =~/tc.xz$/        ) { $mode=~s/tc\.xz/TinyCore/; }
	elsif ($mode =~/pxelinux.cfg\//) { $mode=~s/pxelinux\..+$/Boot menu/; }
	$mode=~s/^\[([^ ]+) \/boot\//$1<\/td><td>/;
	
	print '<tr><th>',$mname,'</th><td>',$mode,"</td></tr>\n";
}

print <<EOT;
</tbody>
	<script type="text/javascript">
$lib
	</script>
</body>
</html>
EOT
