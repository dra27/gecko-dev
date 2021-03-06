#!/usr/bonsaitools/bin/perl --
# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Netscape Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/NPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Tinderbox build tool.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): 

# Figure out which directory tinderbox is in by looking at argv[0]

use File::Find;

$tinderboxdir = $0;
$tinderboxdir =~ s:/[^/]*$::;      # Remove last word, and slash before it.
$tinderboxdir = '.' if $tinderboxdir eq '';
$now          = time();
$expire_time  = $now - 7 * 24 * 60 * 60;
#print "tinderbox = $tinderboxdir\n"; 

chdir $tinderboxdir or die "Couldn't chdir to $tinderboxdir"; 

sub files_to_remove {
  # Remove files older than 7 days
  unlink if /^\d+\.\d+\.gz$|^tbx.[0-9]+$|^warn\d.*\.html$/ and int(-M $_) > 7;
  # Remove files older than 1 day
  unlink if /^\d+\.\d+\.brief\.html$/ and int(-M $_) > 1;
}
&find(\&files_to_remove, $tinderboxdir);

# Remove build.dat entries older than 7 days
#
sub log_files_to_trim {
  return unless /^(?:notes.txt|build.dat|scrape.dat|warnings.dat)$/;
  warn "Cleaning $File::Find::name\n";
  my $file = $_;
  my $range_start = 0;
  my $line = 1;
  my $ed_cmds = '';
  open LOG, "$file";
  while (<LOG>) {
    $log_time = (split /\|/)[0];
    if ($log_time =~ /(\d+)\.\d+\.gz/) {
      # Log file name is first column in file. Pull time out of it.
      $log_time = $1;
    }
    if ($range_start == 0 and $log_time < $expire_time) {
      $range_start = $line;
    } elsif ($range_start != 0 and $log_time >= $expire_time) {
      if ($range_start == $line - 1) {
        $ed_cmds = "${range_start}d\n$ed_cmds";
      } else {
        $ed_cmds = "$range_start,".($line - 1)."d\n$ed_cmds";
      }
      $range_start = 0;
    }
    $line++;
  }
  close LOG;
  next if $ed_cmds eq '';
  open ED,"| ed $file" or die "died ed'ing: $!\n";
  print ED "${ed_cmds}w\nq\n";
  close ED;
}
&find(\&log_files_to_trim, $tinderboxdir);

