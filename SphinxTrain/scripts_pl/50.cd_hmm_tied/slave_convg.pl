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

# For the first iteration, we initialize the number of gaussian
# components to 1. That's how we start with continuous models, and
# it is irrelevant for semi continuous
my $n_gau = 1;
if (@ARGV) {
    $n_gau = shift;
}
my $iter = 1;
if (@ARGV) {
    $iter = shift;
}

my $n_parts = ($ST::CFG_NPART) ? $ST::CFG_NPART : 1;
# If the number of parts is given as command line argument, overwrite
# the number coming from the config file
if (@ARGV) {
   $n_parts = shift;
}
# as the next stage (deleted interpolation) requires at least 2 parts
# we set the default number of parts to be 2, but only if we're using
# semi continuous models
if (($n_parts < 2) and ($ST::CFG_HMM_TYPE eq ".semi.")) {
    $n_parts = 2;
}

$| = 1; # Turn on autoflushing
my $logdir = "$ST::CFG_LOG_DIR/50.cd_hmm_tied";

my $modeldir  = "$ST::CFG_BASE_DIR/model_parameters";
mkdir ($modeldir,0777);

# We have to clean up and run flat initialize if it is the first iteration
if (($iter == 1) && (($n_gau == 1) || ($ST::CFG_HMM_TYPE eq ".semi."))) {
    Log ("MODULE: 50 Training Context dependent models\n");
    Log ("    Cleaning up directories: accumulator...");
    rmtree ($ST::CFG_BWACCUM_DIR, 0, 1);
    mkdir ($ST::CFG_BWACCUM_DIR,0777);
    Log ("logs...\n");
    rmtree ($logdir, 0, 1);
    mkdir ($logdir,0777);
    Log ("qmanager...");
    rmtree ($ST::CFG_QMGR_DIR, 0, 1);
    mkdir ($ST::CFG_QMGR_DIR,0777);
    copyci2cd2initialize();
}

# Call baum_welch with iter part and n_parts,
# once done call norm_and_lauchbw.pl
my @deps;
for (my $i=1; $i<=$n_parts; $i++)
{
    push @deps, LaunchScript("bw.$n_gau.$iter.$i", ['baum_welch.pl', $n_gau, $iter, $i, $n_parts]);
}
LaunchScript("norm.$iter", ['norm_and_launchbw.pl', $n_gau, $iter, $n_parts], \@deps);
# For the first iteration (i.e. the one that was called from the
# command line or a parent script), wait until completion or error
my $return_value = 0;
if ($iter == 1 and $n_gau == 1) {
    $return_value = TiedWaitForConvergence($logdir);
}
exit $return_value;

sub copyci2cd2initialize ()
{
    Log ("    Copy CI to CD initialize\n");

    #**************************************************************************
    # this script copies the mixw/mean/var/tmat from a ci (continuous) HMM
    # to a cd (continuous) HMM. We need two mdef files, one without triphones
    # for the ci hmm definition and another one with triphones for the cd
    # triphone definition.
    # our final goal is to train in stages, first the ci models and then
    # with cd models. This should minimize initialization problems..
     #*************************************************************************

    my $ci_hmmdir = "$ST::CFG_BASE_DIR/model_parameters/${ST::CFG_EXPTNAME}.ci_${ST::CFG_DIRLABEL}";
    my $src_moddeffn = "$ST::CFG_BASE_DIR/model_architecture/${ST::CFG_EXPTNAME}.ci.mdef";
    my $src_mixwfn = "$ci_hmmdir/mixture_weights";
    my $src_meanfn = "$ci_hmmdir/means";
    my $src_varfn = "$ci_hmmdir/variances";
    my $src_tmatfn = "$ci_hmmdir/transition_matrices";

    my $cd_hmmdir = "$ST::CFG_BASE_DIR/model_parameters/${ST::CFG_EXPTNAME}.cd_${ST::CFG_DIRLABEL}_initial";
    mkdir ($cd_hmmdir,0777);

    my $dest_moddeffn = "$ST::CFG_BASE_DIR/model_architecture/${ST::CFG_EXPTNAME}.$ST::CFG_N_TIED_STATES.mdef";
    my $dest_mixwfn = "$cd_hmmdir/mixture_weights";
    my $dest_meanfn = "$cd_hmmdir/means";
    my $dest_varfn = "$cd_hmmdir/variances";
    my $dest_tmatfn = "$cd_hmmdir/transition_matrices";

    my $logdir = "$ST::CFG_BASE_DIR/logdir/50.cd_hmm_tied";
    mkdir ($logdir,0777);
    my $logfile = "$logdir/$ST::CFG_EXPTNAME.copy.ci.2.cd.log";

    HTML_Print (FormatURL("$logfile", "Log File") . " ");

    # if there is an LDA transformation, use it
    my @feat;
    if (defined($ST::CFG_LDA_TRANSFORM) and -r $ST::CFG_LDA_TRANSFORM) {
	@feat = (-feat => '1s_c', -ceplen => $ST::CFG_LDA_DIMENSION);
    } else {
	@feat = (-feat => $ST::CFG_FEATURE, -ceplen => $ST::CFG_VECTOR_LENGTH);
    }
    return RunTool('init_mixw', $logfile, 0,
		   -src_moddeffn => $src_moddeffn,
		   -src_ts2cbfn => $ST::CFG_HMM_TYPE,
		   -src_mixwfn => $src_mixwfn,
		   -src_meanfn => $src_meanfn,
		   -src_varfn => $src_varfn,
		   -src_tmatfn => $src_tmatfn,
		   -dest_moddeffn => $dest_moddeffn,
		   -dest_ts2cbfn => $ST::CFG_HMM_TYPE,
		   -dest_mixwfn => $dest_mixwfn,
		   -dest_meanfn => $dest_meanfn,
		   -dest_varfn => $dest_varfn,
		   -dest_tmatfn => $dest_tmatfn,
		   @feat);
}

