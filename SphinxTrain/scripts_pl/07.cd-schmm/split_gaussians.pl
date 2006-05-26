#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-2004 Carnegie Mellon University.  All rights 
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
## Author: Evandro Gouvea
##

use File::Path;
use File::Copy;

my $index = 0;
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    $index = 2;
} else {
    $cfg_file = "etc/sphinx_train.cfg";
}

if (! -s "$cfg_file") {
    print "Unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_train.cfg for default\n";
    exit -3;
}
require $cfg_file;
require "$CFG_SCRIPT_DIR/util/utils.pl";

#***************************************************************************
# This script splits the current set of gaussian models. The increase
# in number of Gaussian components is the argument used to call the
# script.
# ***************************************************************************

# If the number of parts is given as command line argument, overwrite
# the number coming from the config file
if (($#ARGV < ($index + 1))) {
  print "Arguments missing. \n" .
    "Usage: $0 [-cfg <config file>] <current n_gaussians> <increase>\n";
  exit -3;
}
my $n_current = $ARGV[$index];
my $n_inc = $ARGV[$index + 1];
if ($n_current + $n_inc > $CFG_FINAL_NUM_DENSITIES) {
  &ST_Log("Increase in number of Gaussians beyond the desired total\n" .
	  "Current: $n_current, increase by: $n_inc, desired total: $CFG_FINAL_NUM_DENSITIES\n");
  exit -3;
}

$| = 1; # Turn on autoflushing
my $scriptdir = "$CFG_SCRIPT_DIR/07.cd-schmm";
my $logdir = "$CFG_LOG_DIR/07.cd-schmm";
mkdir ("$logdir",0777) unless -d $logdir;

my $modeldir  = "$CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

&ST_Log ("    Split Gaussians, increase by $n_inc\n");
    
#**************************************************************************
# this script copies the mixw/mean/var/tmat from a cd (continuous)
# HMM to another cd (continuous) HMM, increasing the number of
# Gaussian components. our final goal is to train increasing the
# number of components in small steps. This should minimize
# convergence problems..
# *************************************************************************

my $src_hmmdir = "$CFG_BASE_DIR/model_parameters/${CFG_EXPTNAME}.cd_${CFG_DIRLABEL}_${CFG_N_TIED_STATES}";
mkdir ($src_hmmdir,0777) unless -d $src_hmmdir;

my $src_moddeffn = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.$CFG_N_TIED_STATES.mdef";
my $src_mixwfn = "$src_hmmdir/mixture_weights";
my $src_meanfn = "$src_hmmdir/means";
my $src_varfn = "$src_hmmdir/variances";
my $src_tmatfn = "$src_hmmdir/transition_matrices";

my $dest_hmmdir = "$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.cd_${CFG_DIRLABEL}_initial";
mkdir ($dest_hmmdir,0777) unless -d $dest_hmmdir;

my $dest_moddeffn = "$CFG_BASE_DIR/model_architecture/$CFG_EXPTNAME.$CFG_N_TIED_STATES.mdef";
my $dest_mixwfn = "$dest_hmmdir/mixture_weights";
my $dest_meanfn = "$dest_hmmdir/means";
my $dest_varfn = "$dest_hmmdir/variances";
my $dest_tmatfn = "$dest_hmmdir/transition_matrices";

my $backup_hmmdir =  "$CFG_BASE_DIR/model_parameters/$CFG_EXPTNAME.cd_${CFG_DIRLABEL}_${CFG_N_TIED_STATES}_${n_current}";
mkdir ($backup_hmmdir,0777) unless -d $backup_hmmdir;

my $backup_mixwfn = "$backup_hmmdir/mixture_weights";
my $backup_meanfn = "$backup_hmmdir/means";
my $backup_varfn = "$backup_hmmdir/variances";
my $backup_tmatfn = "$backup_hmmdir/transition_matrices";

# Copy the current models to a safe location, to preserve them
copy "$src_mixwfn", "$backup_mixwfn";
copy "$src_meanfn", "$backup_meanfn";
copy "$src_varfn", "$backup_varfn";
copy "$src_tmatfn", "$backup_tmatfn";

# The transition file won't change, so just copy it
copy "$src_tmatfn", "$dest_tmatfn";

# With semi continuous models, we already start with the right number
# of components
if ($CFG_HMM_TYPE eq ".semi.") {
  &ST_Log ("Split Gaussian not performed for semi continuous\n");
  exit 0;
}

# If $n_inc isn't at least 1, there's nothing else to do
if ($n_inc <= 0) {
  exit 0;
}

my $logdir = "$CFG_BASE_DIR/logdir/07.cd-schmm";
mkdir ($logdir,0777) unless -d $logdir;
my $logfile = "$logdir/$CFG_EXPTNAME.copy.ci.2.cd.log";

my $SPLIT = "$CFG_BIN_DIR/inc_comp";

my $split_cmd = "\"$SPLIT\" " .
  "-ninc $n_inc " .
  "-dcountfn \"$src_mixwfn\" " .
  "-inmixwfn  \"$src_mixwfn\" " .
  "-outmixwfn \"$dest_mixwfn\" " .
  "-inmeanfn  \"$src_meanfn\" " .
  "-outmeanfn \"$dest_meanfn\" " .
  "-invarfn   \"$src_varfn\" " .
  "-outvarfn  \"$dest_varfn\" " .
  "-feat      $CFG_FEATURE " .
  "-ceplen    $CFG_VECTOR_LENGTH ";

exit (RunTool($split_cmd, $logfile, 0));
