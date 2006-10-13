# -*- cperl -*-
## ====================================================================
##
## Copyright (c) 2006 Carnegie Mellon University.  All rights
## reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
##
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in
##    the documentation and/or other materials provided with the
##    distribution.
##
## This work was supported in part by funding from the Defense Advanced
## Research Projects Agency and the National Science Foundation of the
## United States of America, and the CMU Sphinx Speech Consortium.
##
## THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
## ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
## THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
## PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
## NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## ====================================================================
##
## Author: David Huggins-Daines
##
use strict;
package SphinxTrain::Util;
require Exporter;
use vars qw(@ISA @EXPORT);
@ISA=qw(Exporter);
@EXPORT=qw(DateStr CreateHeader Log HTML_Print FormatURL
           ImgSrc LogWarning LogError Converged RunTool
           RunScript LaunchScript WaitForScript GetLists
	   WaitForConvergence TiedWaitForConvergence);

use Sys::Hostname;
use File::Basename;
use File::Spec;
use Queue;

die "Configuration (e.g. etc/sphinx_train.cfg) not defined\n" unless defined ($ST::CFG_DONE);

$ST::CFG_OKAY_COLOR = '00D000';
$ST::CFG_WARNING_COLOR = '555500';
$ST::CFG_ERROR_COLOR = 'DD0000';

sub DateStr
{
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
}

sub CreateHeader {
    my $logfile = shift;

    my $hostname = hostname();
    open HTML_LOG,">$logfile";
    print HTML_LOG "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n";
    print HTML_LOG "<html>\n";
    print HTML_LOG "<head>\n";
    print HTML_LOG "<META HTTP-EQUIV=REFRESH CONTENT=60>\n"; # Force reloading every 60 seconds
    print HTML_LOG "<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\">\n";
    print HTML_LOG "<title>$ST::CFG_EXPTNAME</title>\n";
    print HTML_LOG "</head>\n";
    print HTML_LOG "<body BGCOLOR = \"#F0F0F5\">\n";
    print HTML_LOG "<pre>\n";
    print HTML_LOG "<CENTER><h1>${ST::CFG_BASE_DIR}/${ST::CFG_EXPTNAME}</h1></CENTER>\n";
    print HTML_LOG "<CENTER> <h3>$hostname</h3></CENTER>\n";
    print HTML_LOG "<hr>\n";
    close HTML_LOG;
}

sub Log
{
    my $message = shift;
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
    my $logfile = "$ST::CFG_BASE_DIR/$ST::CFG_EXPTNAME.html";

    print "$message" if $ST::CFG_VERBOSE;
    if (!-s $logfile) {
        CreateHeader($logfile);
    }
    open HTML_LOG,">>$logfile";
    if ($message =~ m/^(MODULE:).*/) {
        print HTML_LOG "<hr>\n";
        chomp ($message);
	print HTML_LOG "<b>$message\t($day_str)</b>\n";	# Put Date information on Module headers
    } else {
        print HTML_LOG "$message";
    }
    close HTML_LOG;
}

sub HTML_Print 
{
    my $message = shift;
    my $logfile = "$ST::CFG_BASE_DIR/$ST::CFG_EXPTNAME.html";

    if (! -s $logfile) {
        CreateHeader($logfile);
    }
    open HTML_LOG,">>$logfile";
    print HTML_LOG "$message";
    close HTML_LOG;

}

sub FormatURL
{
    my $path = shift;
    my $label = shift;
    my $url;

    if ($path =~ m/^\/.*/) {
        $url = "file://";
    } else {
        $url = "file:///";
    }
    $url = "<a href=\"" . $url . "$path\">$label</a>";
    return $url;
}

sub ImgSrc
{
    my $path = shift;
    my $url;

    if ($path =~ m/^\/.*/) {
        $url = "file://";
    } else {
        $url = "file:///";
    }
    $url = "<img src=\"" . $url . "$path\">";
    return $url;
}

sub LogWarning
{
    my $message = shift;
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
    my $logfile = "$ST::CFG_BASE_DIR/$ST::CFG_EXPTNAME.html";

    print "WARNING: $message" if $ST::CFG_VERBOSE;
    if (! -s $logfile) {
        CreateHeader($logfile);
    }
    open HTML_LOG,">>$logfile";
    print HTML_LOG "<p>WARNING: $message</p>";
    close HTML_LOG;
}

sub LogError
{
    my $message = shift;
    my (@time) = localtime(time());
    my ($day_str) = sprintf ("%4d-%02d-%02d %02d:%02d",(1900 + $time[5]), ($time[4]+1),$time[3],$time[2],$time[1]);
    my $logfile = "$ST::CFG_BASE_DIR/$ST::CFG_EXPTNAME.html";

    chomp ($message);		# remove \n if it exists

    print "$message" if $ST::CFG_VERBOSE;
    if (! -s $logfile) {
        CreateHeader($logfile);
    }
    open HTML_LOG,">>$logfile";
    print HTML_LOG "<p>$message</p>";
    close HTML_LOG;
}

sub Converged
{
  my $iter = shift;
  my $logdir = shift;
  my ($l,$p,$ratio);

  # See what happened at this iteration
  $l = `grep \"overall>\" $logdir/${ST::CFG_EXPTNAME}.${iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;

  if ($iter > 1) {
      my $tmp_iter = $iter - 1;
      # See what happened last iteration
      $p = `grep \"overall>\" $logdir/${ST::CFG_EXPTNAME}.${tmp_iter}-*.bw.log | awk '{X += \$3;Y += \$6} END {print Y/X}'`;
      # Compute it's ratio
      $ratio = ($l-$p)/abs($p);
      Log ("\t\tRatio: $ratio\n");
  }

  # Don't even bother checking convergence until we've reached a minimum number of loops
  return 0 if ($iter < $ST::CFG_MIN_ITERATIONS);
  return 1 if ($ratio < $ST::CFG_CONVERGENCE_RATIO);
  return 2 if ($iter > $ST::CFG_MAX_ITERATIONS);
}

sub RunTool {
  my ($cmd, $logfile, $ctl_counter, @args) = @_;

  unless ($cmd =~ / / or File::Spec->file_name_is_absolute($cmd)) {
      # TODO: Handle architecture-specific directories here
      $cmd = File::Spec->catfile($ST::CFG_BIN_DIR, $cmd);

      # Windows needs the .exe suffix, but all OSes need to verify if
      # the file exists. The right test here would be "-x", but this
      # doesn't work in Windows. We use the next best thing, "-e"
      foreach ("", ".exe") {
	  if (-e "$cmd$_") {
	      $cmd .= $_;
	      last;
	  }
      }
      die "Could not find executable for $cmd" unless -e $cmd;
  }
  local (*LOG, $_);
  open LOG,">$logfile";

  local $| = 1;	# Turn on autoflushing

  my $returnvalue = 0;
  my $error_count = 0;
  my $warning_count = 0;
  my $processed_counter = 0;
  my $processed_counter = 0;
  my $printed = 0;

  my ($pid, $pipe);
  if ($^O eq 'MSWin32') {
      # Win32 can't do -|, so quote all the arguments and do a simple
      # pipe open.
      foreach (@args) {
	  $_ = qq{"$_"};
      }
      $pid = open $pipe, "\"$cmd\" @args 2>&1 |";
  }
  else {
      $pid = open $pipe, "-|";
      die "Failed to open pipe: $!" unless defined($pid);
  }

  if ($pid == 0) {
      open STDERR, ">&STDOUT";
      exec $cmd, @args;
  }
  else {
    while (<$pipe>) {
      print LOG "$_";
      if (/(FATAL).*/) {
	LogError ($_ . "\n");
        $returnvalue = 1;
        last;
      }
      $error_count++ if m/(ERROR).*/;
      $warning_count++ if m/(WARNING).*/;
      if ($ctl_counter) {
	# Keep track of progress being made.
	$processed_counter++  if (/.*(utt\>).*/);
	my $percentage = int (($processed_counter / $ctl_counter) * 100);
	if (!($percentage % 10)) {
	  Log ("${percentage}% ") unless $printed;
	  $printed = 1;
	} else {
	  $printed = 0;
	}
      }
    }
    close $pipe;
    if (($error_count > 0) or ($warning_count > 0)) {
      LogError ("\n\t\tThis step had $error_count ERROR messages and " .
		    "$warning_count WARNING messages.\n" .
		    "\t\tPlease check the log file for details.\n");
    }
  }
  my $date = localtime;
  print LOG "$date\n";
  close LOG;
  if ($returnvalue) {
    HTML_Print ("\t\t<font color=\"$ST::CFG_ERROR_COLOR\"> FAILED </font>\n");
  } else {
    HTML_Print ("\t\t<font color=\"$ST::CFG_OKAY_COLOR\"> completed </font>\n");
  }

  return ($returnvalue);
}

sub RunScript {
    my ($script, @args) = @_;
    my $scriptdir = File::Spec->catdir($ST::CFG_SCRIPT_DIR,
				       basename(dirname($0)));
    unless (File::Spec->file_name_is_absolute($script)) {
	$script = File::Spec->catfile($scriptdir, $script);
    }
    unshift(@args, -cfg => $ST::CFG_FILE);

    return system($script, @args);
}

sub LaunchScript {
    my ($name, $cmd, $deps) = @_;
    my $scriptdir = File::Spec->catdir($ST::CFG_SCRIPT_DIR,
				       basename(dirname($0)));

    unless (defined($ST::Q)) {
	$ST::CFG_QUEUE_TYPE = "Queue" unless defined $ST::CFG_QUEUE_TYPE;
	eval "require $ST::CFG_QUEUE_TYPE"
	    or die "Failed to load queue $ST::CFG_QUEUE_TYPE: $@";
	$ST::Q = $ST::CFG_QUEUE_TYPE->new();
    }

    my ($script, @args);
    if (ref($cmd)) {
	($script, @args) = @$cmd;
    }
    else {
	$script = $cmd;
    }
    unless (File::Spec->file_name_is_absolute($script)) {
	$script = File::Spec->catfile($scriptdir, $script);
    }
    unshift(@args, -cfg => File::Spec->rel2abs($ST::CFG_FILE));

    # Capture output somewhere for background queues
    my @qmanager;
    if ($ST::CFG_QUEUE_TYPE ne 'Queue') {
	my $bn = basename($script);
	mkdir $ST::CFG_QMGR_DIR, 0777 unless -d $ST::CFG_QMGR_DIR;
	push @qmanager, errfile => File::Spec->catfile($ST::CFG_QMGR_DIR, "$name.err");
	push @qmanager, outfile => File::Spec->catfile($ST::CFG_QMGR_DIR, "$name.out");
    }
    return $ST::Q->submit_job({deps => $deps,
			       name => $name,
			       @qmanager,
			       command => [$script, @args]});
}

sub WaitForScript {
    my $id = shift;
    return $ST::Q->waitfor_job($id);
}

sub WaitForConvergence {
    my $logdir = shift;
    my $interval = shift;

    # For some reason (probably due to stupidity of system()), we
    # can't do this globally or in Queue::POSIX, but we need it here
    # (hopefully it does nothing on Windows)
    local $SIG{CHLD} = sub { wait; };

    $interval = 5 unless defined($interval);
    # Wait for training to complete (FIXME: This needs to be more robust)
    my $maxiter = 0;
    while (1) {
	my $iter;
	my $likeli;
	for ($iter = 1; $iter <= $ST::CFG_MAX_ITERATIONS; ++$iter) {
	    my $norm_log = File::Spec->catfile($logdir, "$ST::CFG_EXPTNAME.$iter.norm.log");
	    if (! -f $norm_log) { # Special case for CI training
		$norm_log = File::Spec->catfile($logdir, "$ST::CFG_EXPTNAME.1.$iter.norm.log");
	    }
	    open LOG, "<$norm_log" or last;
	    while (<LOG>) {
		if (/failed/ or /Aborting/) {
		    Log("Training failed in iteration $iter\n");
		    return -1;
		}
		elsif (/COMPLETE/) {
		    Log("Training completed after $iter iterations\n");
		    return 0;
		}
		elsif (/Likelihood Per Frame = (\S+)/) {
		    $likeli = $1;
		}
	    }
	    close LOG;
	}
	--$iter;
	if ($iter > $maxiter) {
	    Log("    Baum-Welch iteration $iter Average log-likelihood $likeli\n");
	    $maxiter = $iter;
	}
	sleep $interval;
    }
    return 0;
}

sub TiedWaitForConvergence {
    my $logdir = shift;
    my $interval = shift;

    # For some reason (probably due to stupidity of system()), we
    # can't do this globally or in Queue::POSIX, but we need it here
    # (hopefully it does nothing on Windows)
    local $SIG{CHLD} = sub { wait; };

    $interval = 5 unless defined($interval);
    # Wait for training to complete (FIXME: This needs to be more robust)
    my ($maxngau, $maxiter, $lastngau, $lastiter) = (0,0,0,0);
    while (1) {
	my ($ngau, $iter, $likeli);

	$ngau = 1;
    NGAU:
	while ($ngau <= $ST::CFG_FINAL_NUM_DENSITIES) {
	ITER:
	    for ($iter = 1; $iter <= $ST::CFG_MAX_ITERATIONS; ++$iter) {
		my $norm_log = File::Spec->catfile($logdir,
						   "$ST::CFG_EXPTNAME.$ngau.$iter.norm.log");
		open LOG, "<$norm_log" or last ITER;
		while (<LOG>) {
		    if (/failed/ or /Aborting/) {
			Log("Training failed in iteration $iter\n");
			return -1;
		    } elsif (/COMPLETE/) {
			Log("Training for $ngau Gaussian(s) completed after $iter iterations\n");
			return 0;
		    }
		    elsif (/Likelihood Per Frame = (\S+)/) {
			$likeli = $1;
		    }
		}
		close LOG;
	    }
	    --$iter;
	    if ($iter != 0 and $ngau > $maxngau) {
		$maxngau = $ngau;
	    }
	    if ($iter > 0) {
		$maxiter = $iter;
	    }

	    if ($iter == 0 or $ngau == $ST::CFG_FINAL_NUM_DENSITIES) {
		last NGAU;
	    }
	    else {
		$ngau *= 2;
		if ($ngau > $ST::CFG_FINAL_NUM_DENSITIES) {
		    $ngau = $ST::CFG_FINAL_NUM_DENSITIES;
		}
	    }
	}
	if ($maxngau > $lastngau or $maxiter > $lastiter) {
	    Log("Baum-Welch gaussians $maxngau iteration $maxiter Average log-likelihood $likeli\n");
	    $lastngau = $maxngau;
	    $lastiter = $maxiter;
	}
	sleep $interval;
    }
    return 0;
}

sub GetLists {
    my ($listoffiles, $transcriptfile);
    if ( $ST::CFG_FORCEDALIGN eq "no" ) {
	$listoffiles = $ST::CFG_LISTOFFILES;
	$transcriptfile = $ST::CFG_TRANSCRIPTFILE;
    } else {
	$listoffiles   = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedfiles";
	$transcriptfile  = "$ST::CFG_BASE_DIR/falignout/${ST::CFG_EXPTNAME}.alignedtranscripts";
    }
    return ($listoffiles, $transcriptfile);
}

1;
__END__

=head1 NAME

SphinxTrain::Util - Utility functions for Sphinx training

=head1 SYNOPSIS

  use SphinxTrain::Util;

=head1 DESCRIPTION

This module exports various utility functions used by the Sphinx trainer.

=head1 EXPORTS

=over 4

=item DateStr

=item CreateHeader

=item Log

=item HTML_Print

=item FormatURL

=item ImgSrc

=item LogWarning

=item LogError

=item Converged

=item RunTool

=item RunScript

=item LaunchScript

=item WaitForScript

=item WaitForConvergence

=item TiedWaitForConvergence

=back

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
