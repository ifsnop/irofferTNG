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
use File::Basename;

# Declare local variables

my ($databaseName, $databaseUser, $databasePw, $dbh);
my ($stmt, $sth, @res);
my ($fname, $size, $crc, $path, $comment, $id, $cname, $csvname, $basepath, $tmp, $command);
my ($count);

$databaseName = "DBI:mysql:rb";
$databaseUser = "rb_user1";
$databasePw = "rb_pass1";

if ($#ARGV != 3) {
    die "Usage: $0 input_csv_file collection_trigger base_path COMMAND={ADD, APPEND, DELETE}\n";
}

print "Starting...\n";
$cname = $ARGV[1];
$csvname = $ARGV[0];
$basepath = $ARGV[2];
$command = $ARGV[3];
# quitamos el / del final del basepath, que NO queremos que este en la bbdd
$basepath=~s/\/$//;

if ($command eq "ADD") {
    print "Opening csv file [" . $csvname . "]\n";
    open(CSVFILE, $ARGV[0]) || die "Can't open [". $ARGV[0] ."]\n";

    print "Connecting to database [" . $databaseName . "]\n";
    $dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

    # crear un nuevo id para la coleccion
    $stmt = "SELECT max(id) from collections";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";
    @res = $sth->fetchrow_array();
    $id = $res[0];
    if (!defined $id) { $id = 0; } else { $id += 1; }
    
    $stmt = "SELECT id from collections where name = '" . $cname . "'";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";
    @res = $sth->fetchrow_array();
    $tmp = $res[0];
    if (defined $tmp) { die "csv exists on database"; }

    print "Inserting collection\n";
    $stmt = "INSERT INTO collections (id, name, csvname, basepath, complete) VALUES (" . $id . ", '". $cname . "', '". basename($csvname) . "', '" . &stdstr($basepath) . "', 0)";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";

    print "Inserting to database\n";
    while (<CSVFILE>) {
        ($fname, $size, $crc, $path, $comment) = split(',', $_);
	chomp($comment); 
        $comment =~ s/\r//g;
	$path =~ s/\\/\//g;
#	if ($comment ne '') {
#	    $path = $path . ',' . $comment;
#	    $path =~ s/\\/\//g;
#	    $comment='';
#	}
        $stmt = "INSERT INTO csvs (id, name, size, crc, path, comment, owned) VALUES (" . $id . ", '". $fname . "', ". $size . ", '" . $crc . "', \"" . &stdstr($path) . "\", \"". &stdstr($comment) . "\", 0)";
#	print $stmt . "\n";
        $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
        $sth->execute || die "execute: $$stmt: $DBI::errstr";
    }

} elsif ($command eq "DELETE") {
    print "Connecting to database [" . $databaseName . "]\n";
    $dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

    # buscar el id de la coleccion
    $stmt = "SELECT id from collections WHERE name='" . $cname . "'";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";
    @res = $sth->fetchrow_array();
    $id = $res[0];
    if ( !defined $id ) { die "Collection doesn't exist"; }
    
    #borrar la coleccion
    $stmt = "DELETE from collections WHERE id='" . $id . "'";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";
    print "Deleting collection\n";

    #borrar los ficheros del csv
    $stmt = "DELETE from csvs WHERE id='" . $id . "'";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";
    print "Deleting csvs\n";

} elsif ($command eq "APPEND") {
    print "Opening csv file [" . $csvname . "]\n";
    open(CSVFILE, $ARGV[0]) || die "Can't open [". $ARGV[0] ."]\n";

    print "Connecting to database [" . $databaseName . "]\n";
    $dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

    # buscar el id de la coleccion
    $stmt = "SELECT id, csvname from collections WHERE name='" . $cname . "'";
    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    $sth->execute || die "execute: $$stmt: $DBI::errstr";
    @res = $sth->fetchrow_array();
    $id = $res[0];
    if ( $id eq "" ) { die "Collection doesn't exist"; }
    
    if ($res[1] ne $csvname) {
	print "Updating csv name...\n";
	$stmt = "UPDATE collections SET csvname ='" . basename($csvname) . "' WHERE id ='" . $id . "'";
	$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
	$sth->execute || die "execute: $$stmt: $DBI::errstr";
    }
    
    $count = 0;
    print "Adding to database\n";
    while (<CSVFILE>) {
        ($fname, $size, $crc, $path, $comment) = split(',', $_);
	chomp($comment); 
        $comment =~ s/\r//g;
	$path =~ s/\\/\//g;

        $stmt = "SELECT name FROM csvs WHERE name = '" . $fname . "' AND crc ='" . $crc ."' AND path =\"" . stdstr($path) . "\" AND size ='" . $size . "' AND id='" . $id . "'";
        $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
	$sth->execute || die "execute: $$stmt: $DBI::errstr";
        $tmp = $sth->fetchrow_array();
        if (!defined $tmp) { 
    	    $stmt = "INSERT INTO csvs (id, name, size, crc, path, comment, owned) VALUES (" . $id . ", '". $fname . "', ". $size . ", '" . $crc . "', \"" . &stdstr($path) . "\", \"". $comment . "\", 0)";
    	    $sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
    	    $sth->execute || die "execute: $$stmt: $DBI::errstr";
	    $count++;
	}
    }
    print "Total added: " . $count . "\n";
}

print "Finishing...\n";

close(CSVFILE);
exit;

    # Prepare and execute the SQL query
#    $sth = $$dbh->prepare($$stmt) || die "prepare: $$stmt: $DBI::errstr";
#    $sth->execute || die "execute: $$stmt: $DBI::errstr";

# Clean up the record set
# $sth->finish();


# $dbh->disconnect();



sub stdstr {
    local $or = $_[0];
#    $or =~ s /\'/\\\'/g;
    $or =~ s /\"/\\\"/g;
    $or =~ s /%/\\%/g;
    $or =~ s /\\/\\\\/g;
    return $or;
}
		