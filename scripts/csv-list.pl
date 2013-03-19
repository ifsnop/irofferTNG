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
my ($reportdir);

$databaseName = "DBI:mysql:rb";
$databaseUser = "rb_user1";
$databasePw = "rb_pass1";

if (($#ARGV < 0) || ($#ARGV > 1)) {
    die "Usage: $0 -col\n";
}

    
#print "Starting...\n";
if ($ARGV[0] ne "-col") {
    exit;
}

$dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

# get collections
$stmt = "SELECT c.id, c.name, c.basepath, c.csvname, COUNT(csvs.name) AS cuenta FROM collections AS c, csvs WHERE c.id = csvs.id GROUP BY csvs.id ORDER BY c.name";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
while ( ($id, $name, $basepath, $csvname, $cuenta) = $sth->fetchrow_array() ) {
    $stmt2 = "SELECT COUNT(id) FROM csvs WHERE id = '" . $id . "' AND owned ='1'";
    $sth2 = $dbh->prepare($stmt2) || die "prepare: $$stmt2: $DBI::errstr";
    $sth2->execute ||die "execute: $$stmt2: $DBI::errstr";
    ($actual) = $sth2->fetchrow_array();
#    printf ("%d\t"$id\t$name\t$cuenta\t$actual\t$csvname\t$basepath\n";
    printf ("%02d  %-10s\t%5d  %5d  %-35s  %-20s\n", $id, $name, $cuenta, $actual, $csvname, $basepath);
#\t$cuenta\t$actual\t$csvname\t$basepath\n";
};
exit;

