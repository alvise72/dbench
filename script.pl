#!/usr/bin/perl

use Data::Dumper qw(Dumper);

print $ENV{"HOSTNAME"};

#@bsize = ("16k", "32k", "64k", "128k", "256k", "512k", "1M", "2M", "4M", "8M", "16M");
@bsize = ("1k", "2k", "4k", "8k", "16k");
@fsize= ("10M", "20M", "40M", "80M", "160M", "320M", "640M","1280M","2560M", "5120M", "10G");

$unit = 1;
my %grades;
foreach $blocksize (@bsize) {
    print "Doing test for blocksize $blocksize...\n";
    foreach $filesize (@fsize) {
	print "Doing test for filesize $filesize... ";
	$cmd = "/usr/local/bin/bench_writer -i 1 -b $blocksize -D $filesize -F -f ./testfile-";
	@output = `$cmd | grep "Speed"|awk '{print \$3" "\$4}'`;
	($thisSpeed,$thisUnit) = split /\s+/, $output[0];
	print "speed=$thisSpeed - unit=$thisUnit\n";
	if($thisUnit =~ m/^G.*/) {
	    $unit = 1073741824;
	}
	if($thisUnit =~ m/^M.*/) {
	    $unit = 1048576;
	}
	if($thisUnit =~ m/^k.*/) {
	    $unit = 1024;
	}
	if($thisUnit =~ m/^T.*/) {
	    $unit = 1099511627776;
	}

	$thisSpeed *= $unit;
	$grade{$blocksize}{$filesize} = $thisSpeed;
    }
}

foreach $BS (@bsize) {
    foreach $FS (@fsize) {
	print "$BS,$FS,$grade{$BS}{$FS}\n";
    }
    

}
