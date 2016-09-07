#!/usr/bin/perl
#ident "$Id: now.pl,v 1.4 1998/04/15 03:07:24 hpa Exp $"
#
# Print the time (possibly the mtime of a file) as a hexadecimal integer
#

($file) = @ARGV;
if ( defined($file) ) {
    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,
     $ctime,$blksize,$blocks) = stat($file);
    $now = $mtime;
} else {
    $now = time;
}

printf "0x%08x\n", $now;
