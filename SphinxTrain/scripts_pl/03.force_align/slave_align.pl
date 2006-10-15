#!/usr/bin/perl
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
use File::Copy;
use File::Basename;
use File::Spec::Functions;
use File::Path;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config;
use SphinxTrain::Util;

my ($iter, $n_parts) = @ARGV;
$iter = 1 unless defined $iter;
$n_parts = (defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1) unless defined $n_parts;

$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/03.force_align";
my $outdir = "$ST::CFG_BASE_DIR/falignout";

Log("MODULE: 03 Force-aligning transcripts\n");
unless (-x catdir($ST::CFG_BIN_DIR, "sphinx3_align")
	or -x catdir($ST::CFG_BIN_DIR, "sphinx3_align.exe")) {
    Log("    Skipped: No sphinx3_align(.exe) found in $ST::CFG_BIN_DIR\n");
    Log("    If you wish to do force-alignment, please copy or link the\n");
    Log("    sphinx3_align binary from Sphinx 3 to $ST::CFG_BIN_DIR\n");
    Log("    and either define \$CFG_MODEL_DIR in sphinx_train.cfg or\n");
    Log("    run context-independent training first.\n");
    exit 0;
}

unless (defined($ST::CFG_FORCE_ALIGN_MODELDIR)
	or -f "$ST::CFG_MODEL_DIR/$ST::CFG_EXPTNAME.ci_$ST::CFG_DIRLABEL/means") {
    Log("    Skipped: No acoustic models available for force alignment\n");
    Log("    If you wish to do force-alignment, please define \$CFG_MODEL_DIR\n");
    Log("    in sphinx_train.cfg or run context-independent training first.\n");
    exit 0;
}

Log("    Cleaning up directories: logs...");
rmtree($logdir, 0, 1);
mkdir($logdir,0777);
Log("output...");
rmtree($outdir, 0, 1);
mkdir($outdir,0777);
Log("qmanager...");
rmtree($ST::CFG_QMGR_DIR, 0, 1);
mkdir($ST::CFG_QMGR_DIR,0777);
Log("s2stseg...");
rmtree($ST::CFG_STSEG_DIR, 0, 1);
mkdir($ST::CFG_STSEG_DIR,0777);
Log("\n");

# Build state segmentation directories
Log("    Building state segmentation directories...");
open INPUT,"${ST::CFG_LISTOFFILES}" or die "Failed to open $ST::CFG_LISTOFFILES: $!";
my %dirs;
while (<INPUT>) {
    chomp;
    my @fields = split;
    my $uttid = pop @fields;
    my $basedir = dirname($uttid);
    next if $basedir eq ".";
    unless ($dirs{$basedir}) {
	$dirs{$basedir}++;
	mkpath(catdir($ST::CFG_STSEG_DIR, $basedir), 0, 0777);
    }
}
Log("\n");
close INPUT;

# Preprocess the transcript to remove extraneous <s> and </s> markers (argh)
my $transcriptfile = "$outdir/$ST::CFG_EXPTNAME.aligninput";
open INPUT,"<$ST::CFG_TRANSCRIPTFILE" or die "Failed to open $ST::CFG_TRANSCRIPTFILE: $!";
open OUTPUT,">$transcriptfile" or die "Failed to open $transcriptfile: $!";
while (<INPUT>) {
    chomp;
    s,</?s>,,g;
    # Also normalize whitespaces
    s,^\s+,,;
    s,\s+$,,;
    s,\s+, ,g;
    print OUTPUT $_, "\n";
}
close INPUT;
close OUTPUT;

# Run n_parts of force alignment
Log("Running force alignment in  $n_parts parts\n");
my @jobs;
for (my $i = 1; $i <= $n_parts; ++$i) {
    push @jobs, LaunchScript("falign.$i", ['force_align.pl', $i, $n_parts]);
}

# Wait for them all to finish, and concatenate their output
my $listoffiles = catfile($outdir, "${ST::CFG_EXPTNAME}.alignedfiles");
my $transcriptfile = catfile($outdir, "${ST::CFG_EXPTNAME}.alignedtranscripts");
open CTL, ">$listoffiles" or die "Failed to open $listoffiles: $!";
open LSN, ">$transcriptfile" or die "Failed to open $transcriptfile: $!";

my $i = 1;
my $return_value = 0;
foreach my $job (@jobs) {
    WaitForScript($job);
    Log("$i ");
    my $logfile = catfile($logdir, "${ST::CFG_EXPTNAME}.$i.falign.log");
    open LOG, "<$logfile" or die "Failed to open $logfile: $!";
    my $success = 0;
    while (<LOG>) {
	if (/TOTAL FRAMES/) {
	    $success = 1;
	    last;
	}
    }
    unless ($success) {
	Log("Failed in part $i");
	$return_value = 1;
	next;
    }
    close LOG;

    my $part = catfile($outdir, "${ST::CFG_EXPTNAME}.alignedtranscripts.$i");
    open INPUT, "<$part" or die "Failed to open $part: $!";
    open INCTL,"${ST::CFG_LISTOFFILES}" or die "Failed to open $ST::CFG_LISTOFFILES: $!";
    while (<INPUT>) {
	my ($uttid) = (/\(([^\(\)]+)\)$/);
	print LSN;
	# Now scan forward in the control past any utterances that were missed
	while (defined(my $ctlline = <INCTL>)) {
	    # For whatever reason, the full path isn't in the align output
	    chomp(my $ctl_uttid = (split " ", $ctlline)[-1]);
	    if ($ctl_uttid =~ /$uttid$/) {
		print CTL $ctlline;
		last;
	    }
	}
    }
    ++$i;
}
Log("\n");

exit $return_value;
