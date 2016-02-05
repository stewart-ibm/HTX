#!/usr/bin/perl

$para_start = 0;
$do_once = 1;

#print "$ARGV[0]";
#print "opening file\n";
open ( FILEP, "/tmp/htxconf.op");

while( $line = <FILEP>) {
   #print "$line";
   #print "end line";
   if ( $do_once ) {
      if ( $line =~ s/://) {
           #printf "in :\n ******* $line ****\n";
           chomp($line);
           #printf "***$line***\n";
        if ($line =~ $ARGV[0] ) {
            $para_start = 1;
            print "$line:\n";
        } else  {
            if ( $para_start ) {
               $do_once = 0;
               #print "para end";
            }
            $para_start = 0;
        }
      } else {
         if ( $para_start ) {
            print $line;
            #print "inside loop ";
         }
      }
    }

}

