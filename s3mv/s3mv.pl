#!/usr/bin/env perl -w

use strict;
use Digest::MD5 qw(md5_hex);
use File::Path qw(make_path);
use File::Spec;
use Net::Amazon::S3;

$|=1;

my ($aws_access_key_id,$aws_secret_access_key,$bucketname,$prefix)=@ARGV;
my $s3 = Net::Amazon::S3->new({
	aws_access_key_id     => $aws_access_key_id,
	aws_secret_access_key => $aws_secret_access_key,
	retry                 => 1
}
) || die "$!\n";

my $bucket=$s3->bucket($bucketname) || die "$bucketname:$!\n";

$prefix ||= '';

my $data=$bucket->list({max_keys => 1000, prefix => $prefix}) || die "list $bucketname:$!\n";

foreach ($data->{'keys'}) {
	foreach (@$_) {
		my %file=%$_;
		my (undef,$dir,$filename)=File::Spec->splitpath($file{'key'});
		make_path($dir);
		$bucket->get_key_filename($file{'key'},'GET',$file{'key'}) || die "$!\n";
		open(FILE, $file{'key'}) or die "Can't open '".$file{'key'}."': $!";
		binmode(FILE);
		die "TREASON! ".$file{'key'}."\n" unless $file{'etag'} eq Digest::MD5->new->addfile(*FILE)->hexdigest;
		close(FILE);
		$bucket->delete_key($file{'key'}) || die $!.":".$file{'key'}."\n";
		print ".";
	}
}

print "\n";