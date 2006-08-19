#!/usr/bin/perl
## ====================================================================
##
## Copyright (c) 1996-2000 Carnegie Mellon University.  All rights
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
## Author: Ricky Houghton
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

#***************************************************************************
# This script launches all the ci - continuous training jobs in the proper
# order. First it cleans up the directories, then launches the
# flat initialization, and the baum-welch and norm jobs for the required
# number of iterations. Within each iteration it launches as many baumwelch
# jobs as the number of parts we wish to split the training into.
#***************************************************************************

my ($iter, $n_parts) = @ARGV;
$iter = 1 unless defined $iter;
$n_parts = (defined($ST::CFG_NPART) ? $ST::CFG_NPART : 1) unless defined $n_parts;

my $modeldir  = "$ST::CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777) unless -d $modeldir;

$| = 1; # Turn on autoflushing
my $logdir = "${ST::CFG_LOG_DIR}/04.cd_schmm_untied";
mkdir ("$logdir",0777) unless -d $logdir;

#Read npart_untied from variables.def
my $return_value = 0;
if ($iter == 1) {
   # Clean up junk from earlier runs
   Log ("MODULE: 04 Training Context Dependent models\n");
   Log ("    Cleaning up directories: accumulator...");

    rmtree ($ST::CFG_BWACCUM_DIR, 0, 1);
    mkdir ($ST::CFG_BWACCUM_DIR,0777);

    Log ("logs...");
    rmtree ($logdir, 0, 1);
    mkdir ($logdir,0777);

    Log ("qmanager...");
    rmtree ($ST::CFG_QMGR_DIR, 0, 1);
    mkdir ($ST::CFG_QMGR_DIR,0777);

    Log ("\n");
    # For the first iteration Flat initialize models.
    $return_value = Initialize();
    exit ($return_value) if ($return_value);
}

# Call baum_welch with iter part and n_parts,
# once done call norm_and_lauchbw.pl
my @deps;
for (my $i=1; $i<=$n_parts; $i++)
{
    push @deps, LaunchScript("bw.$iter.$i", ['baum_welch.pl', $iter, $i, $n_parts])
}
LaunchScript("norm.$iter", ['norm_and_launchbw.pl', $iter, $n_parts], \@deps);
# For the first iteration (i.e. the one that was called from the
# command line or a parent script), wait until completion or error
if ($iter == 1) {
    $return_value = WaitForConvergence($logdir);
}
exit $return_value;

exit 0;

sub Initialize () {
  my $cihmmdir = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.ci_${ST::CFG_DIRLABEL}";
  my $cdhmmdir = "${ST::CFG_BASE_DIR}/model_parameters/${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_untied";
  mkdir ($cdhmmdir,0777) unless -d $cdhmmdir;

  my $logdir  =  "${ST::CFG_LOG_DIR}/04.cd_schmm_untied";
  mkdir ($logdir,0777) unless -d $logdir;
  my $logfile = "$logdir/${ST::CFG_EXPTNAME}.copycitocd.log";

  Log ("    Initialization Copy CI to CD ");
  HTML_Print (FormatURL("$logfile", "Log File") . " ");

  my $COPY_CI_TO_CD = "${ST::CFG_BIN_DIR}/init_mixw";

  # if there is an LDA transformation, use it
  my @feat;
  if (defined($ST::CFG_LDA_TRANSFORM) and -r $ST::CFG_LDA_TRANSFORM) {
      @feat = (-feat => '1s_c', -ceplen => $ST::CFG_LDA_DIMENSION);
  } else {
      @feat = (-feat => $ST::CFG_FEATURE, -ceplen => $ST::CFG_VECTOR_LENGTH);
  }
  return RunTool
      ('init_mixw', $logfile, 0,
       -src_moddeffn => "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.ci.mdef",
       -src_ts2cbfn => $ST::CFG_HMM_TYPE,
       -src_mixwfn => "$cihmmdir/mixture_weights",
       -src_meanfn => "$cihmmdir/means",
       -src_varfn => "$cihmmdir/variances",
       -src_tmatfn => "$cihmmdir/transition_matrices",
       -dest_moddeffn => "${ST::CFG_BASE_DIR}/model_architecture/${ST::CFG_EXPTNAME}.untied.mdef",
       -dest_ts2cbfn => $ST::CFG_HMM_TYPE,
       -dest_mixwfn => "$cdhmmdir/mixture_weights",
       -dest_meanfn => "$cdhmmdir/means",
       -dest_varfn => "$cdhmmdir/variances",
       -dest_tmatfn => "$cdhmmdir/transition_matrices",
       @feat);
}


