#!/usr/bin/perl
# @(#)46        1.1.4.1  src/htx/usr/lpp/htx/etc/scripts_stx/eMerge.pl, htxconf, htxubuntu 9/15/10 01:02:57
#
###############################################################################
#
# COMPONENT_NAME: htxmisc_pl
#
# ORIGINS: 27
#
# OWNER: Jack E. Moore
#        jem@us.ibm.com
#
# DESCRIPTION: This script merges the output of the AIX "errpt -a" command
#              with the error output from HTX (htxerr)
#
###############################################################################
  $seperator = "---------------------------------------------------------------------------";


###############################################################################
# Read in aixerr's into array
  @aixerr = "";
  @aixerr = `/bin/errpt`;
  $aixerr = @aixerr;

# Read in aixerr_a's into array
  @aixerr_a = "";
  @aixerr_a = `/bin/errpt -a`;
  $aixerr_a = @aixerr_a;

# Read in htxerr's into array
  @htxerr = "";
  if ( -f "htxerr" ) {
       @htxerr = `/bin/cat htxerr`;
  } elsif ( -f "/tmp/trial/htxerr" ) {
       @htxerr = `/bin/cat /tmp/trial/htxerr`;
  } elsif ( -f "/tmp/htxerr" ) {
       @htxerr = `/bin/cat /tmp/htxerr`;
  } else {
       die ("Can not locate the htxerr file (looked in ., /tmp/trial and /tmp)\n");
  }
  $htxerr = @htxerr;

  #print "Total lines in aixerr:   $aixerr\n";
  #print "Total lines in aixerr_a: $aixerr_a\n";
  #print "Total lines in htxerr:   $htxerr\n";


###############################################################################
# Work on aixerr (errpt) entries - this is used to capture the timestamps only
  $count = "0";
  @timestamp = "";
  foreach $line (@aixerr) {
     if ($line !~ /IDENTIFIER/ ) {
         @Words = split(" ", $line);
         $Word  = @Words[1];
         @Word  = split(//, $Word);
         $Month = $Word[0].$Word[1];
         $Day   = $Word[2].$Word[3];
         $Hour  = $Word[4].$Word[5];
         $Min   = $Word[6].$Word[7];
         $Year  = $Word[8].$Word[9];
         $Year = convYear($Year);
         $Word = $Year.$Month.$Day.$Hour.$Min;
         @timestamp[$count++] = "$Word\n";
     }
  }

  $timestamp = @timestamp;
  #print "Total elements in timestamp: $timestamp\n";


###############################################################################
# Work on aixerr_a (errpt_a) entries - parse out each error stanza into an
# array element
  $count = "0";
  $lcount = "0";
  @aixerr_items = "";
  foreach $line (@aixerr_a) {
     if ($line =~ /$seperator/ && $lcount > "0") {
         @aixerr_items[$count++] = "$fline";
         $fline="";
     }
     $lcount++;
     $fline = "$fline"."$line";
  }
  @aixerr_items[$count] = "$fline"."$line";
  $aixerr_items = @aixerr_items;
  #print "Total elements in aixerr_items: $aixerr_items\n";

  if ($aixerr_items != $timestamp ) {
      print "Mismatch in \"errpt\" and \"errpt -a\" items - please rerun\n";
      exit;
  }


###############################################################################
# Work on htxerr entries - parse out each error stanza into an array element
  $count = "0";
  $lcount = "0";
  $fline="";
  @htxerr_items = "";
  foreach $line (@htxerr) {
     if ($line =~ /.*err=.*sev=.*/ && $lcount > "0") {
         @htxerr_items[$count++] = "$fline";
         $fline="";
     }
     $lcount++;
     $fline = "$fline"."$line";
  }
  @htxerr_items[$count] = "$fline"."$line";
  $htxerr_items = @htxerr_items;
  #print "Total elements in htxerr_items: $htxerr_items\n";


###############################################################################
###############################################################################
# Move the aixerr_a elements to a hash array
  $count = "0";
  $Seq = "0";
  @err_hash = "";
  foreach $element (@aixerr_items) {
     @hline = split("\n", $element);
     ($junk, $junk, $Month, $Day, $Time, $junk) = split(" ", @hline[4]);
     ($Hour, $Min, $Sec) = split(":", $Time);
     $Month = convMonth($Month);

   # This pulls the actual year from the aixerr reported data.  Year data is
   # not provided in the aixerr_a (errpt -a) data
     $timestamp = @timestamp[$count];
     $count++;
     @Word  = split(//, $timestamp);
     $Year  = $Word[0].$Word[1].$Word[2].$Word[3];

   # If there is more than one timestamp used, this will assign a sequence 
   # number so that the data can be reconstructed in the correct order
     $ftimestamp = $Year.$Month.$Day.$Hour.$Min.$Sec;
     if ( "$ftimestamp" =~ "$ftimestamp_save" ) {
          $ftimestamp_save = $ftimestamp;
          $Seq++;
        } else {
          $Seq = "0";
          $ftimestamp_save = $ftimestamp;
     }
     $ftimestamp = $Year.$Month.$Day.$Hour.$Min.$Sec.$Seq."a";
     #print "Formatted Timestamp:   $ftimestamp\n";
     $err_hash{$ftimestamp} = "$element";
  }


###############################################################################
# Move the htxerr elements to a hash array
  $count = "";
  #$ftimestamp_save = "0";
  foreach $element (@htxerr_items) {
     @hline = split("\n", $element);
     #print "@hline[0]\n";
     ($junk, $Month, $Day, $Time, $Year, $junk) = split(" ", @hline[0]);
     ($Hour, $Min, $Sec) = split(":", $Time);
     $Month = convMonth($Month);

     $ftimestamp = $Year.$Month.$Day.$Hour.$Min.$Sec;
     if ( "$ftimestamp" =~ "$ftimestamp_save" ) {
          $ftimestamp_save = $ftimestamp;
          $Seq++;
        } else {
          $Seq = "0";
          $ftimestamp_save = $ftimestamp;
     }
     $ftimestamp = $Year.$Month.$Day.$Hour.$Min.$Sec.$Seq."h";
     #print "Formatted Timestamp:   $ftimestamp\n";
     $err_hash{$ftimestamp} = "$seperator\n"."$element";
  }

  foreach $element (sort { $b <=> $a } keys %err_hash) {
           print "$err_hash{$element}\n";
  }


###############################################################################
###############################################################################
# Subroutines
###############################################################################
###############################################################################
sub convMonth {
  if ($Month =~ "Jan") { $Month="01"; }
  if ($Month =~ "Feb") { $Month="02"; }
  if ($Month =~ "Mar") { $Month="03"; }
  if ($Month =~ "Apr") { $Month="04"; }
  if ($Month =~ "May") { $Month="05"; }
  if ($Month =~ "Jun") { $Month="06"; }
  if ($Month =~ "Jul") { $Month="07"; }
  if ($Month =~ "Aug") { $Month="08"; }
  if ($Month =~ "Sep") { $Month="09"; }
  if ($Month =~ "Oct") { $Month="10"; }
  if ($Month =~ "Nov") { $Month="11"; }
  if ($Month =~ "Dec") { $Month="12"; }
  return ($Month);
}

###############################################################################
sub convYear {
  if ($Year =~ "01") { $Year="2001"; }
  if ($Year =~ "02") { $Year="2002"; }
  if ($Year =~ "03") { $Year="2003"; }
  if ($Year =~ "04") { $Year="2004"; }
  if ($Year =~ "05") { $Year="2005"; }
  return ($Year);
}

