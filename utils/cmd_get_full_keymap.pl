#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
GetOptions(
    "serial=s" => \$serial,
);

my $fh = Dalsik::open_serial($serial);

print "Sending:\n\tGET_FULL_KEYMAP\n";
my $cmd = Dalsik::get_cmd('GET_FULL_KEYMAP');
unless (print $fh $cmd) {
    die "Could not send GET_FULL_KEYMAP: $!";
}

print "Received:\n";
while (my $line = <$fh>) {
    print "\t$line";

    if ($line =~ /^CMD/) {
        last;
    }
}

close $fh;
