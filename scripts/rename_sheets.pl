#!/usr/bin/perl

# The latest official data drop on 31 Dec 2014 confuses xlsx2csv.py - unless I am doing something
# wrong - a distinct possibility.  The python script marshalls the workbook to many csv docs - and
# as far as I know right now, it still does this correctly.  But it doesn't name the files
# correctly - somehow the worksheet name to resulting file name is off.  I've md5sum'd the
# resulting files and all the md5s from the previous version exist in the directory of files for
# the new version - all but one - which is expected because only one sheet was updated.  But the
# md5s indicate that the files have different names.

# EG:
# old_good:
#    014d3e4b88b5a8824cfc2573270d1bbc  o/Reactive Advancement.csv
#    7dd803f501c79e20b48e64297dd72ebb  o/Cantrip Advancement.csv
#
# new_bad:
#    014d3e4b88b5a8824cfc2573270d1bbc  oA/Cantrip Advancement.csv
#    735974ead640193605fd81582e37af2c  oA/Reactive Advancement.csv
#    7dd803f501c79e20b48e64297dd72ebb  oA/Armor Advancement.csv

# So you can see that for the new data, the Reactive file has been incorrectly named Cantrip.  They
# both have the same md5sum - so they are exactly the same - but the file name is different/wrong.

# It drives me crazy how hard it is to use Microsoft generated stuff in a non-Microsoft environment.

# So this script will work on three args: the md5sum list from the old_good directory, the md5sum
# list from the new_bad directory, and the new_bad directory name.  I'll use the md5sums to know to
# what how the new files will be renamed.  IE, because the new file named Cantrip has the md5sum of
# the Reactive file, it will be renamed Reactive.

# make them like this:
# ( cd official_data_20141206 &&  find . -type f -print0 | xargs -0 md5sum | sort ) > good_but_old
# ( cd official_data_20141231 &&  find . -type f -print0 | xargs -0 md5sum | sort ) > bad


use strict;
use warnings;
use FileHandle;
use File::Temp qw/ tempfile tempdir /;
use File::Basename;

my $goodSums = 'good_but_old';
my $newSums = 'bad';
my $destDir = 'official_data_20141231';

my %goodMap_NameToMd5;
my %goodMap_Md5ToName;
my %leftOverFiles;

my $gsf = FileHandle->new($goodSums) or die "Can't read $goodSums; $!"; # good sum file
while (my $line = <$gsf>) {
    chomp($line);
    # "a6d5be332a1611768308cf5f4483f6b2  ./Crafting Achievements.csv"
    my ($md5, $fn) = $line =~ /^([0-9a-f]+)\s+\.\/(.*)$/;
    printf("OLD_GOOD: %38s %s\n", $fn, $md5);
    $goodMap_NameToMd5{$fn} = $md5;
    $goodMap_Md5ToName{$md5} = $fn;
}
close($gsf);

my $tmpDir = tempdir( CLEANUP => 0 );

my $bsf = FileHandle->new($newSums) or die "Can't read $newSums; $!";
while (my $line = <$bsf>) {
    chomp($line);
    # "a6d5be332a1611768308cf5f4483f6b2  ./Crafting Achievements.csv"
    my ($md5, $fn) = $line =~ /^([0-9a-f]+)\s+\.\/(.*)$/;

    if (exists $goodMap_Md5ToName{$md5}) {
	my $goldName = $goodMap_Md5ToName{$md5};
	if ($goldName ne $fn) {
	    printf("%38s should be named %38s (md5: %s)", $fn, $goldName, $md5);

	    # if the file in the destDir w/ goldName's name, move it to a temp dir
	    if (-e "${destDir}/${goldName}") {
		rename "${destDir}/${goldName}", "${tmpDir}/${goldName}" or die "$!";
		print " <";
	    }

	    # we may have moved it to the temp dir already - check here
	    if (-e "${tmpDir}/${fn}") {
		# here we must have already moved the file to the temp dir
		rename "${tmpDir}/${fn}", "${destDir}/${goldName}" or die "$!";
		print " T>";
	    } else {
		# nope - the file is still here - just rename it
		if (-e "${destDir}/${fn}") {
		    rename "${destDir}/${fn}", "${destDir}/${goldName}" or die "$!";
		    print " >";
		} else {
		    print "ERROR: we lost $fn - needed to rename it to $goldName\n";
		}
	    }
	    print "\n";
	} else {
	    print " COOL: this file got the right name; the md5s agree: $fn\n";
	}
	delete $goodMap_Md5ToName{$md5};
	delete $goodMap_NameToMd5{$goldName};
    } else {
	# here we have a new file that has an md5 that isn't in the list of old_good
	# file's md5s.
	print "WARNING: Can't find this md5 in the known md5s: $md5 -- $fn\n";
	$leftOverFiles{$fn} = 1;
    }
}
close($bsf);

if (scalar keys %goodMap_NameToMd5 == 0 && scalar keys %leftOverFiles == 0) {
    print "Awesome!  All files renamed using just the md5s\n";
    return;
}

if (scalar keys %goodMap_NameToMd5 == 1 && scalar keys %leftOverFiles == 1) {
    # perfect - there is only one file left to rename
    my $correctName = (keys %goodMap_NameToMd5)[0];
    my $existingName = (keys %leftOverFiles)[0];
    if (-e "${tmpDir}/${existingName}") {
	rename "${tmpDir}/${existingName}", "${destDir}/${correctName}" or die "$!";
    } else {
	print "ERROR: we lost this file! $existingName\n";
    }
    print "Awesome! All files renamed - including the one different file; $existingName -> $correctName\n";
} else {
    system("mv", $tmpDir, ".");
    my $dirName = basename($tmpDir);
    print "Bummer - there is more than one different file - check $dirName for leftovers\n";
}
