#!/usr/bin/perl -w

###############################################################################
#Name: patch_rules_file_tpsm.pl
#Purpose: Patch input rule files with the input options.
#Note: At present we support only 20 input strings.
###############################################################################

use Getopt::Long;

#Command line options. At present only 20 strings can be patched to rules file.
my $str1    = '';
my $str2    = '';
my $str3    = '';
my $str4    = '';
my $str5    = '';
my $str6    = '';
my $str7    = '';
my $str8    = '';
my $str9    = '';
my $str10    = '';
my $str11    = '';
my $str12    = '';
my $str13    = '';
my $str14    = '';
my $str15    = '';
my $str16    = '';
my $str17    = '';
my $str18    = '';
my $str19    = '';
my $str20    = '';
my $infile  = '';
my $outfile = '';
parseOptions(\$str1, \$str2, \$str3, \$str4, \$str5, \$str6, \$str7, \$str8, \$str9,\$str10, \$str11, \$str12, \$str13, \$str14, \$str15, \$str16, \$str17, \$str18,\$str19, \$str20, \$infile, \$outfile);

#print_options();
patch_rulesfile();

#Sub routines
sub print_options
{
	print "str1=$str1\n";
	print "str2=$str2\n";
	print "str3=$str3\n";
	print "str4=$str4\n";
	print "str5=$str5\n";
	print "str6=$str6\n";
	print "str7=$str7\n";
	print "str8=$str8\n";
	print "str9=$str9\n";
	print "str10=$str10\n";
	print "str11=$str11\n";
	print "str12=$str12\n";
	print "str13=$str13\n";
	print "str14=$str14\n";
	print "str15=$str15\n";
	print "str16=$str16\n";
	print "str17=$str17\n";
	print "str18=$str18\n";
	print "str19=$str19\n";
	print "str20=$str20\n";
	print "infile=$infile\n";
	print "outfile=$outfile\n";
}

sub usage
{
	print "Name:".$0."\n";
	print "Description: Patch rules file with the strings passed as argument.\n";
	print "Options:\n";
	print "    -help or --help: Display this message\n";
	print "    -str1 or --str1 <String>\n";
	print "    -str2 or --str2 <String>\n";
	print "    -str3 or --str3 <String>\n";
	print "    -str4 or --str4 <String>\n";
	print "    -str5 or --str5 <String>\n";
	print "    -str6 or --str6 <String>\n";
	print "    -str7 or --str7 <String>\n";
	print "    -str8 or --str8 <String>\n";
	print "    -str9 or --str9 <String>\n";
	print "    -str10 or --str10 <String>\n";
	print "    -str11 or --str11 <String>\n";
	print "    -str12 or --str12 <String>\n";
	print "    -str13 or --str13 <String>\n";
	print "    -str14 or --str14 <String>\n";
	print "    -str15 or --str15 <String>\n";
	print "    -str16 or --str16 <String>\n";
	print "    -str17 or --str17 <String>\n";
	print "    -str18 or --str18 <String>\n";
	print "    -str19 or --str19 <String>\n";
	print "    -str20 or --str20 <String>\n";
	print "    -infile or --infile <Absolute path of input rules file>\n";
	print "    -outfile or --outfile <Absolute path of output rules file>\n";
	print "NOTE1: infile and outfile are mandatory arguments\n";
	print "NOTE2: If string has space in between then it must be placed in double quotes\n";
}

sub parseOptions
{
	#All arguments are passed by reference
	my($str1, $str2, $str3, $str4, $str5, $str6, $str7, $str8, $str9, $str10, $str11, $str12, $str13, $str14, $str15, $str16, $str17, $str18, $str19, $str20, $infile, $outfile) = @_;

	my $help = 0;

	if( @ARGV == 0 )
	{
		usage();
		exit 0;
	}

	GetOptions('help|?' => \$help, 'str1=s' => $str1, 'str2=s' => $str2, 
			'str3=s' => $str3, 'str4=s' => $str4, 'str5=s' => $str5, 'str6=s' => $str6,
			'str7=s' => $str7, 'str8=s' => $str8, 'str9=s' => $str9, 'str10=s' => $str10,
			'str11=s' => $str11, 'str12=s' => $str12, 'str13=s' => $str13, 'str14=s' => $str14,
			'str15=s' => $str15, 'str16=s' => $str16, 'str17=s' => $str17, 'str18=s' => $str18,
			'str19=s' => $str19, 'str20=s' => $str20, 'infile=s' => $infile, 'outfile=s' => $outfile); 

	if( $help || ($$infile eq "") || ($$outfile eq "") )
	{
		usage();
		exit 0;
	}
}

sub patch_rulesfile
{
	my $line;
	my $i = 0;
	my $found1 = 0;
	my $found2 = 0;
	my @out_arr;
	my $patch_start = 1;
	open(INFILE, "$infile") ||
         die("ERROR: Unable to open input file \"$infile\". Exiting.\n");
   if($patch_start == 0)
	{
	while( defined($line = <INFILE>) )
	{
		if($line =~ m/RULE_ID/)
		{
			if($found1 == 0)
			{
				 $found1 = 1;
				 $out_arr[$i++] = $line;
				 next;
			}
			else
			{
				 if($str1 ne "")
				 {
					  $out_arr[$i++] = $str1."\n";
				 }
				 if($str2 ne "")
				 {
					  $out_arr[$i++] = $str2."\n";
				 }
				 if($str3 ne "")
				 {
					  $out_arr[$i++] = $str3."\n";
				 }
				 if($str4 ne "")
				 {
					  $out_arr[$i++] = $str4."\n";
				 }
				 if($str5 ne "")
				 {
					  $out_arr[$i++] = $str5."\n";
				 }
				 if($str6 ne "")
				 {
					  $out_arr[$i++] = $str6."\n";
				 }
				 if($str7 ne "")
				 {
					  $out_arr[$i++] = $str7."\n";
				 }
				 if($str8 ne "")
				 {
					  $out_arr[$i++] = $str8."\n";
				 }
				 if($str9 ne "")
				 {
					  $out_arr[$i++] = $str9."\n";
				 }
				 if($str10 ne "")
				 {
					  $out_arr[$i++] = $str10."\n";
				 }
				 if($str11 ne "")
				 {
					  $out_arr[$i++] = $str11."\n";
				 }
				 if($str12 ne "")
				 {
					  $out_arr[$i++] = $str12."\n";
				 }
				 if($str13 ne "")
				 {
					  $out_arr[$i++] = $str13."\n";
				 }
				 if($str14 ne "")
				 {
					  $out_arr[$i++] = $str14."\n";
				 }
				 if($str15 ne "")
				 {
					  $out_arr[$i++] = $str15."\n";
				 }
				 if($str16 ne "")
				 {
					  $out_arr[$i++] = $str16."\n";
				 }
				 if($str17 ne "")
				 {
					  $out_arr[$i++] = $str17."\n";
				 }
				 if($str18 ne "")
				 {
					  $out_arr[$i++] = $str18."\n";
				 }
				 if($str19 ne "")
				 {
					  $out_arr[$i++] = $str19."\n";
				 }
				 if($str20 ne "")
				 {
					  $out_arr[$i++] = $str20."\n";
				 }
				 $out_arr[$i++] = $line;
			}
		}
		else
		{
			$out_arr[$i++] = $line;
		}
	}
	if($str1 ne "")
	{
		 $out_arr[$i++] = $str1."\n";
	}
	if($str2 ne "")
	{
		 $out_arr[$i++] = $str2."\n";
	}
	if($str3 ne "")
	{
		 $out_arr[$i++] = $str3."\n";
	}
	if($str4 ne "")
	{
		 $out_arr[$i++] = $str4."\n";
	}
	if($str5 ne "")
	{
		 $out_arr[$i++] = $str5."\n";
	}
	if($str6 ne "")
	{
		 $out_arr[$i++] = $str6."\n";
	}
	if($str7 ne "")
	{
		 $out_arr[$i++] = $str7."\n";
	}
	if($str8 ne "")
	{
		 $out_arr[$i++] = $str8."\n";
	}
	if($str9 ne "")
	{
		 $out_arr[$i++] = $str9."\n";
	}
	if($str10 ne "")
	{
		 $out_arr[$i++] = $str10."\n";
	}
	if($str11 ne "")
	{
		 $out_arr[$i++] = $str11."\n";
	}
	if($str12 ne "")
	{
		 $out_arr[$i++] = $str12."\n";
	}
	if($str13 ne "")
	{
		 $out_arr[$i++] = $str13."\n";
	}
	if($str14 ne "")
	{
		 $out_arr[$i++] = $str14."\n";
	}
	if($str15 ne "")
	{
		 $out_arr[$i++] = $str15."\n";
	}
	if($str16 ne "")
	{
		 $out_arr[$i++] = $str16."\n";
	}
	if($str17 ne "")
	{
		 $out_arr[$i++] = $str17."\n";
	}
	if($str18 ne "")
	{
		 $out_arr[$i++] = $str18."\n";
	}
	if($str19 ne "")
	{
		 $out_arr[$i++] = $str19."\n";
	}
	if($str20 ne "")
	{
		 $out_arr[$i++] = $str20."\n";
	}
	}
	else
	{
	while( defined($line = <INFILE>) )
	{
		if($line =~ m/RULE_ID/)
		{
				 $out_arr[$i++] = "\n";
				 $out_arr[$i++] = $line;
				 if($str1 ne "")
				 {
					  $out_arr[$i++] = $str1."\n";
				 }
				 if($str2 ne "")
				 {
					  $out_arr[$i++] = $str2."\n";
				 }
				 if($str3 ne "")
				 {
					  $out_arr[$i++] = $str3."\n";
				 }
				 if($str4 ne "")
				 {
					  $out_arr[$i++] = $str4."\n";
				 }
				 if($str5 ne "")
				 {
					  $out_arr[$i++] = $str5."\n";
				 }
				 if($str6 ne "")
				 {
					  $out_arr[$i++] = $str6."\n";
				 }
				 if($str7 ne "")
				 {
					  $out_arr[$i++] = $str7."\n";
				 }
				 if($str8 ne "")
				 {
					  $out_arr[$i++] = $str8."\n";
				 }
				 if($str9 ne "")
				 {
					  $out_arr[$i++] = $str9."\n";
				 }
				 if($str10 ne "")
				 {
					  $out_arr[$i++] = $str10."\n";
				 }
				 if($str11 ne "")
				 {
					  $out_arr[$i++] = $str11."\n";
				 }
				 if($str12 ne "")
				 {
					  $out_arr[$i++] = $str12."\n";
				 }
				 if($str13 ne "")
				 {
					  $out_arr[$i++] = $str13."\n";
				 }
				 if($str14 ne "")
				 {
					  $out_arr[$i++] = $str14."\n";
				 }
				 if($str15 ne "")
				 {
					  $out_arr[$i++] = $str15."\n";
				 }
				 if($str16 ne "")
				 {
					  $out_arr[$i++] = $str16."\n";
				 }
				 if($str17 ne "")
				 {
					  $out_arr[$i++] = $str17."\n";
				 }
				 if($str18 ne "")
				 {
					  $out_arr[$i++] = $str18."\n";
				 }
				 if($str19 ne "")
				 {
					  $out_arr[$i++] = $str19."\n";
				 }
				 if($str20 ne "")
				 {
					  $out_arr[$i++] = $str20."\n";
				 }
				 $out_arr[$i++] = "\n";
		}
		else
		{
			$out_arr[$i++] = $line;
		}
	}
	}
	open(OUTFILE, ">$outfile") ||
         die("Unable to open output file \"$outfile\" for writing. Exiting program.");
	print OUTFILE @out_arr;
	
	close(INFILE);
	close(OUTFILE);
}
