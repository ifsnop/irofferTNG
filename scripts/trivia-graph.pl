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
my ($preguntas_total);
my ($preguntas_tema, $tema);
my ($cuenta);

$databaseName = "DBI:mysql:rb"; $databaseUser = "rb_user1"; $databasePw = "rb_pass1";

$dbh = DBI->connect($databaseName, $databaseUser, $databasePw) || die "Connect failed: $DBI::errstr\n";

# get collections
$stmt = "SELECT count(id) FROM trivial_preguntas";	
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
($preguntas_total) = $sth->fetchrow_array();

#printf ("%s\n", $preguntas_total);

printf("<HTML><HEAD>\r
<META HTTP-EQUIV='refresh' content='300'>\r
<META HTTP-EQUIV='Expires' CONTENT='0'>\r
<META HTTP-EQUIV='Pragma' CONTENT='no-cache'>\r
<STYLE TYPE='text/css'>\r
    .mini {\r
	font: 11px Tahoma, Verdana, sans-serif;\r
	text-decoration:none;\r
	text-align: left;\r
	color: #FFFFFF;
}</STYLE></HEAD>\r\n");
printf("<BODY BGCOLOR=#000000><TABLE BORDER=0 cellspacing='10' cellpadding='0' width='100%'>\r\n");

$stmt = "SELECT count(trivial_preguntas.id), trivial_temas.tema FROM trivial_temas, trivial_preguntas "
	."WHERE trivial_temas.id = trivial_preguntas.tema_id GROUP BY trivial_temas.id ORDER by trivial_temas.tema";
$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
$sth->execute || die "execute: $$stmt: $DBI::errstr";
$cuenta=0;
printf("<TR>");
while ( ($preguntas_tema, $tema) = $sth->fetchrow_array() ) {

    printf("<TD class='mini'>$tema</SPAN></TD><TD class='mini'>%3.1f %%</TD><TD>",$preguntas_tema*100.0/$preguntas_total);
    printf("<img src='/themes/ExtraLite/images/leftbar_w.gif' height='15' width='7'>");
    printf("<img src='/themes/ExtraLite/images/mainbar_w.gif' height='15' width='%3.0f'>", $preguntas_tema*1000.0/$preguntas_total);
    printf("<img src='/themes/ExtraLite/images/rightbar_w.gif' height='15' width='7'>");
    printf("</TD>\r\n");
    $cuenta++;
    if ($cuenta eq 2) { 
	$cuenta=0;
	printf("</TR><TR>\r\n");
    }
}
printf("</TR></TABLE><br><span class='mini'>total de preguntas contempladas: $preguntas_total</span></BODY></HTML>\r\n");

exit;

#$stmt = "SELECT count(trivial_preguntas.id), trivial_temas.tema FROM trivial_temas, trivial_preguntas"
#	."WHERE trivial_temas.id = trivial_preguntas.tema_id GROUP BY trivial_temas.id ORDER by trivial_temas.tema";

#$sth = $dbh->prepare($stmt) || die "prepare: $$stmt: $DBI::errstr";
#$sth->execute || die "execute: $$stmt: $DBI::errstr";
#while ( ($id, $name, $basepath, $csvname, $cuenta) = $sth->fetchrow_array() ) {
#    $stmt2 = "SELECT COUNT(id) FROM csvs WHERE id = '" . $id . "' AND owned ='1'";
#    $sth2 = $dbh->prepare($stmt2) || die "prepare: $$stmt2: $DBI::errstr";
#    $sth2->execute ||die "execute: $$stmt2: $DBI::errstr";
#    ($actual) = $sth2->fetchrow_array();
##    printf ("%d\t"$id\t$name\t$cuenta\t$actual\t$csvname\t$basepath\n";
#    printf ("%02d  %-10s\t%5d  %5d  %-35s  %-20s\n", $id, $name, $cuenta, $actual, $csvname, $basepath);
##\t$cuenta\t$actual\t$csvname\t$basepath\n";
#};
#exit;

