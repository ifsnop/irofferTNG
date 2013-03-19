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
my ($stmt, $sth, @res);
my ($st, $fname, $size, $crc, $path, $comment, $id, $cname, $csvname, $basepath, $tmp, $command, $csvfile, $csvbasepath);
my ($count, %csv, %csvname);

sub process_csv {

    $st = stat($File::Find::name);
    if ( (! ($st->mode & 0040000)) && (substr("$_", -4, 4) eq ".csv")  ) {
        print "scan " . $File::Find::name . "\n";
	($cname, $tmp) = split(/_/);
	($cname, $tmp) = split(/\(/, $cname);
	($tmp, $count) = split(/_/, substr("$_", 0, length("$_") -4));
	if (!defined($csv{$cname})) {
	    $csv{$cname} = $count;
	    $csvname{$cname} = "$_";
	} elsif ($csv{$cname} < $count) {
	    print "deleting " . $basepath . $_ . "\n";
	    unlink $basepath . $_;
	    $csv{$cname} = $count;
	    $csvname{$cname} = "$_";
	} else {
	    print "deleting " . $File::Find::name . "\n";
	    unlink $File::Find::name;
	}
    }
}

$databaseName = "DBI:mysql:rb";
$databaseUser = "rb_user1";
$databasePw = "rb_pass1";

if ($#ARGV != 0) {
    die "Usage: $0 base_path\n";
}

print "Starting...\n";
$basepath = $ARGV[0];
if (substr("$basepath", -1, 1) ne '/') {
    $basepath = "$basepath" . "/";
}

find(\&process_csv, $basepath);

print "Connecting to database [" . $databaseName . "]\n";
$dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

my @j = keys %csv;
for ($i=0; $i<=$#j; $i++) {
    #lets update the database
#    printf "%s - %d - %s\n", $csvname{$j[$i]}, $csv{$j[$i]}, $j[$i];
    $stmt = "SELECT collections.id, collections.name, collections.csvname, collections.basepath FROM collections WHERE csvname LIKE '" . $j[$i]  . "%'";;
#    print $stmt . "\n";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";
    ($id, $name, $csvfile, $csvbasepath) = $sth->fetchrow_array();
    if (!defined($id)) {
	#coleccion nueva
	print "new > " . $basepath . $csvname{$j[$i]} . "\n";
    } else {
	#print "scan2 >" . $basepath . $csvname{$j[$i]} . "\n";
    	($cname, $tmp) = split(/_/, $csvfile);
	($cname, $tmp) = split(/\(/, $cname);
	($tmp, $count) = split(/_/, substr("$csvfile", 0, length("$csvfile") -4));
	if ($csv{$j[$i]} >= $count) {
	    $stmt =  "./csv2db.pl '" . $basepath . $csvname{$j[$i]} . "' " . $name . " " . $csvbasepath . " " . "APPEND\n";;
	    system($stmt);
	    print "deleting after update >" . $basepath . $csvname{$j[$i]} . "\n";
	    unlink $basepath . $csvname{$j[$i]};
	} else {
	    print "deleting old >" . $basepath . $csvname{$j[$i]} . "\n";
	    unlink $basepath . $csvname{$j[$i]};
	}
    }
}

print "Bye...\n";
exit;

