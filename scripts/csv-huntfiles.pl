#!/usr/bin/perl -w
#
# script utilities
# Copyright (C) 2001 - now() ifsnop/rapid
#
# By using this file, you agree to the terms and conditions set
# forth in the GNU General Public License.  More information is
# available in the README file.
#
# I don't know how to code in perl, so please report any bug
# you find.

# Use the DBI module
use DBI qw(:sql_types);
use File::Find;
use File::stat;
use File::Copy;

# Declare local variables
my ($databaseName, $databaseUser, $databasePw, $dbh);
my ($stmt, $sth, $sthup, $i);
my ($fname, $size, $crc, $path, $comment, $id, $cname, $csvname, $basepath, $owned, $tmp, $lpath);
my ($huntdir, $st, $stdir, $dirtry);
my ($n_huntedfiles, $n_totalfiles, $n_ignoredfiles);

sub process_dirs {
    $st = stat($File::Find::name);
    if (($st->mode & 0040000) && !($File::Find::name eq ".")) {
	if (rmdir($File::Find::name)) {
	    $dirtry++;
	    print("deleting empty dir > $File::Find::name\n");
	}
    }

    return;
}
sub process_zip {
    $st = stat($File::Find::name);
    $fname = $File::Find::name;
    $fname = $fname . "_d";
    
    if (! ($st->mode & 0040000)) {
	if (/\.zip$/s) {
	    system("mkdir '$fname'");
	    system("/usr/bin/unzip -d '$fname' '$File::Find::name'");
	    unlink($File::Find::name);
	}
    }
    return;
}

sub process_file {

    $st = stat($File::Find::name);
    
    if (! ($st->mode & 0040000)) {
	#es un fichero y es accesible
	$n_totalfiles++;
        $newcrc = `/usr/local/bin/crc32 "$File::Find::name"`;
        $stmt = "SELECT csvs.id, csvs.name, collections.name, path, size, crc, owned, basepath FROM csvs, collections WHERE csvs.id=collections.id AND size='" . $st->size . "' AND crc='$newcrc'";
	$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
	$sth->execute || die "execute: $$stmt: $DBI::errstr";
	for ($i = $sth->rows; $i > 0; $i--) {
	    ($id, $fname, $cname, $path, $size, $crc, $owned, $basepath) = $sth->fetchrow_array();
            if ($owned == 1) {
	        print "$File::Find::name (dupe: $cname:$path$fname)\n";
    	    } else {
		$n_huntedfiles++;
	        print "$File::Find::name -> $basepath$path$fname\n";
		$stdir = stat("$basepath$path") || system("mkdir -p '$basepath$path'");
	        if ($i == 1) {
	    	    move("$File::Find::name", "$basepath$path$fname");
		} else {
		    copy("$File::Find::name", "$basepath$path$fname");
		}
  	 	$stmt = "UPDATE csvs SET owned = 1 WHERE id=$id AND size='" . $st->size . "' AND crc='$newcrc' AND name='$fname' AND path='$path'";
		$sthup = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
		$sthup->execute || die "execute: $$stmt: $DBI::errstr";
	    }
	}
	if (($sth->rows() > 0) && ($owned == 1)) {
	    $n_dupefiles++;
    	    unlink $File::Find::name;
	}
	if ($sth->rows == 0) {
	    $n_ignoredfiles++;
	}
    }
    return;
}

$databaseName = "DBI:mysql:rb";
$databaseUser = "rb_user1";
$databasePw = "rb_pass1";

if ($#ARGV != 0) {
    die "Usage: $0 hunt_directory\n";
}

print "Starting...\n";
$huntdir = $ARGV[0];
$n_huntedfiles = 0;
$n_ignoredfiles = 0;
$n_totalfiles = 0;
$n_dupefiles = 0;

find(\&process_zip, $huntdir);
print "Connecting to database [" . $databaseName . "]\n";
$dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

# find collection id
#$stmt = "SELECT id, basepath, csvname from collections where name = '" . $cname . "'";
#$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
#$sth->execute || die "execute: $$stmt: $DBI::errstr";
#($id, $basepath, $csvname) = $sth->fetchrow_array();

#if ( !defined $id  ) { die "Collection not found\n";}

# find files on csvs
find(\&process_file, $huntdir);

#delete empty directories
do {
    $dirtry = 0;
    finddepth(\&process_dirs, $huntdir);
} until ($dirtry==0);

printf("total files: %d hunted: %d dupes: %d ignored: %d\n", $n_totalfiles, $n_huntedfiles, $n_dupefiles, $n_ignoredfiles);

exit;

