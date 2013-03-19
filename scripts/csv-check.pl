#!/usr/bin/perl 
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

# Declare local variables
my ($databaseName, $databaseUser, $databasePw, $dbh);
my ($stmt, $sth);
my ($fname, $size, $crc, $path, $comment, $id, $cname, $csvname, $basepath, $owned, $tmp, $lpath);
my ($reportdir, $csvnamefiltered);
my ($filescountok, $filescountmissing, $totalbytescount, $totalbytesowned);

sub process_file {

    $st = stat($File::Find::name);
    
    if (! ($st->mode & 0040000)) {
	#es un fichero y es accesible
	$lpath = substr($File::Find::dir, length($basepath), length($File::Find::dir) - length($basepath)) . "/";
        $stmt = "SELECT name, path, size, crc, owned, comment FROM csvs WHERE name ='$_' AND size='" . $st->size . "' AND id='$id' AND path=\"$lpath\"";
	$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
	$sth->execute || die "execute: $$stmt: $DBI::errstr";
	($fname, $path, $size, $crc, $owned, $comment) = $sth->fetchrow_array();
	if (defined $fname) {
	    #existe en la bbdd
	    if ($owned == 1) {
		print OKFILES "$fname,$size,$crc,$path,$comment\r\n";
	    } else {
		#NO existe en la bbdd
	        $newcrc = `/usr/local/bin/crc32 "$File::Find::name"`;
		if ($crc eq $newcrc) {
		    # el fichero cumple
	            $stmt = "UPDATE csvs SET owned = 1 WHERE id='$id' AND path=\"$path\" AND name='$_' AND size='" . $st->size . "' AND crc='$crc'";
		    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
		    $sth->execute || die "execute: $$stmt: $DBI::errstr";
		    print OKFILES "$_," . $st->size . ",$newcrc,$lpath,\r\n";
		} else {
		    # el fichero NO cumple
		    print BADFILES "$_," . $st->size . ",$newcrc,$lpath,\r\n";
		}
	    }
	} else {
            $newcrc = `/usr/local/bin/crc32 "$File::Find::name"`;
	    print NEWFILES "$_," . $st->size . ",$newcrc,$lpath,\r\n";
	}
    } else {
	print "directory : $File::Find::name\n";
    }
    return;
}

$databaseName = "DBI:mysql:rb";
$databaseUser = "rb_user1";
$databasePw = "rb_pass1";

if (($#ARGV < 0) || ($#ARGV > 1)) {
    die "Usage: $0 collection_name [FORCED]\n";
}

    
print "Starting...\n";
$cname = $ARGV[0];


if (defined $ENV{'RB_REPORTDIR'}) {
    $reportdir = $ENV{'RB_REPORTDIR'};
} else {
    $reportdir = "$ENV{'HOME'}/.rb";
}

print "Connecting to database [" . $databaseName . "]\n";
$dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

# find collection id
$stmt = "SELECT id, basepath, csvname from collections where name = '" . $cname . "'";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
($id, $basepath, $csvname) = $sth->fetchrow_array();


if ( !defined $id  ) { die "Collection not found\n";}

if (defined $ARGV[1]) {
    if ($ARGV[1] eq "FORCED") {
	print "Cleaning owned...\n";
        $stmt = "UPDATE csvs SET owned = 0 WHERE id='$id'";
        $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
        $sth->execute || die "execute: $$stmt: $DBI::errstr";
    }
}

($csvnamefiltered,$tmp)=split('_',$csvname);
($csvnamefiltered,$tmp)=split(/\(/,$csvnamefiltered);

print "Opening files...\n";
open (OKFILES, "> $reportdir/$csvnamefiltered.ok") or die("Can't open OK report file\n"); #ficheros que cumplen el crc
open (NEWFILES, "> $reportdir/$csvnamefiltered.new") or die("Can't open NEW report file\n"); #ficheros que no están en el csv
open (BADFILES, "> $reportdir/$csvnamefiltered.bad") or die("Can't open BAD report file\n"); #ficheros que están y no cumplen el crc
open (MISFILES, "> $reportdir/$csvnamefiltered.mis") or die("Can't open MIS report file\n"); #ficheros que faltan
open (REPORT, "> $reportdir/$csvnamefiltered.txt") or die ("Can't open RPT report file\n"); #ficheros que faltan para el mirc/pserve
open (HUNTERREPORT, "> $reportdir/$csvnamefiltered.rpt") or die ("Can't open HNTRPT report file\n"); #report ala hunter para mirc/pserve
open (SEPARATOR, "> $reportdir/$csvnamefiltered.---") or die("Can't open MIS report file\n"); #separador
print "Scanning...\n";




print HUNTERREPORT "Hunter Collection Manager - Version 2.0\r\n";
print HUNTERREPORT "Sat Nov 28 13:15:20 2003\r\n";
print HUNTERREPORT "\r\n";
print HUNTERREPORT "Collection: $csvnamefiltered - Location: $basepath\r\n";
print HUNTERREPORT "CSV file:   /mnt/raid/desktop/csv/$csvname\r\n";
print HUNTERREPORT "\r\n";
print HUNTERREPORT "Status        File name     Size     CRC 32     SubFolder\r\n";
print HUNTERREPORT "------        ---------     ----     ------     ---------\r\n";


# find files on csvs
find(\&process_file, $basepath);

$stmt = "SELECT name, path, size, crc FROM csvs WHERE id='$id' AND owned=0 ORDER BY path,name";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
while ( ($fname, $path, $size, $crc, $comment) = ($sth->fetchrow_array()) ) {
    if (!defined $comment) {
	print MISFILES "$fname,$size,$crc,$path,\r\n";
    } else {
    	print MISFILES "$fname,$size,$crc,$path,$comment\r\n";
    }
    $path=~s/\//\\/g;
    print REPORT "Missing $fname $size $crc $path Unknown\r\n";
    print HUNTERREPORT "Missing       $fname $size $crc $path Unknown\r\n";
#		 Missing       Hayley_Coppin_23.jpg            303021   784a942f   \Hayley_Coppin\  Unknown

}

$filescount=0;
$stmt = "SELECT COUNT(csvs.id) FROM csvs WHERE csvs.id ='$id'";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
($filescount) = $sth->fetchrow_array();

$filescountmissing=0;
$stmt = "SELECT COUNT(csvs.id) FROM csvs WHERE csvs.id ='$id' AND owned = 0";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
($filescountmissing) = $sth->fetchrow_array();

$filescountok=0;
$stmt = "SELECT COUNT(csvs.id) FROM csvs WHERE csvs.id ='$id' AND owned = 1";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
($filescountok) = $sth->fetchrow_array();

$totalbytescount=0;
$stmt = "SELECT SUM(size) FROM csvs WHERE csvs.id ='$id'";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
($totalbytescount) = $sth->fetchrow_array();

$totalbytesowned=0;
$stmt = "SELECT SUM(size) FROM csvs WHERE csvs.id ='$id' AND owned = 1";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
($totalbytesowned) = $sth->fetchrow_array();

print HUNTERREPORT "\r\n";
print HUNTERREPORT "\r\n";
print HUNTERREPORT "\r\n";
print HUNTERREPORT "Results for collection:\r\n";
print HUNTERREPORT "-----------------------\r\n";
print HUNTERREPORT "Files in collection:   $filescount\r\n";
print HUNTERREPORT "\r\n";
print HUNTERREPORT "Verified:         $filescountok\r\n";
print HUNTERREPORT "Ok:               0\r\n";
print HUNTERREPORT "Bad Crc:          0\r\n";
print HUNTERREPORT "Bad filesize:     0\r\n";
print HUNTERREPORT "Missing:          $filescountmissing\r\n";
print HUNTERREPORT "Extras:           0\r\n";
print HUNTERREPORT "Unneeded:         0\r\n";
print HUNTERREPORT "\r\n";
print HUNTERREPORT "Total bytes in collection:   $totalbytescount\r\n";
print HUNTERREPORT "Collection now has bytes:    $totalbytesowned\r\n";
print HUNTERREPORT "\r\n";



close OKFILES;
close NEWFILES;
close BADFILES;
close MISFILES;
close REPORT;
close HUNTERREPORT;
close SEPARATOR;
exit;

