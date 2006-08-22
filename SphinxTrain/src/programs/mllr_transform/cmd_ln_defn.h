/*********************************************************************
 *
 * $Header$
 *
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1998 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: cmd_ln_defn.h
 * 
 * Description: 
 * 	Command line argument definition
 *
 * Author: 
 *      Eric Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/cmd_ln.h>
#include <s3/err.h>

#ifndef CMD_LN_DEFN_H
    const char helpstr[] =  
"Description: \n\
\n\
Given a set of MLLR transform, mllr_transform can transform \n\
the mean according to formulat y=Ax+b.\n \
\n \
The output and input files are specified by -outmeanfn and \n\
 -inmeanfn respectively. You may also transform the context-\n\
dependent model using the option -cdonly.  In that case you \n\
need to specify a model definition using -moddeffn.";

    const char examplestr[] = 
"Example: \n\
  The simplest case: \n\
  mllr_transform -inmeanfn inMeans -outmeanfn outMeans  -mllrmat matrix \n\
  \n\
  Adapt only on CD phones: \n\
  mllr_transform  -inmeanfn inMeans -outmeanfn outMeans  -mllrmat matrix -cdonly yes -moddeffn mdef \n\
  \n\
  Help and example: \n\
  nmllr_transform -help yes -example yes ";

    static arg_def_t defn[] = {
	{ "-help",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows the usage of the tool"},

	{ "-example",
	  CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
	  "no",
	  "Shows example of how to use the tool"},

	{ "-inmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Input Gaussian mean file name"},

	{ "-outmeanfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output Gaussian mean file name"},

	{ "-ingaucntfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Input Gaussian accumulation count file name"},

	{ "-outgaucntfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Output Gaussian accumulation count file name"},

	{ "-mllrmat",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "The MLLR matrix file"},

	{ "-cb2mllrfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  ".1cls.",
	  "The codebook-to-MLLR class file. Override option -cdonly"},

        { "-cdonly",
          CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
          "no",
          "Use CD senones only. -moddeffn must be given."},

        { "-inverse",
          CMD_LN_BOOLEAN,
	  CMD_LN_NO_VALIDATION,
          "no",
          "Apply the inverse transform (for speaker adaptive training)."},

        { "-moddeffn",
          CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
          CMD_LN_NO_DEFAULT,
          "Model Definition file. "},

	{ "-varfn",
	  CMD_LN_STRING,
	  CMD_LN_NO_VALIDATION,
	  CMD_LN_NO_DEFAULT,
	  "Variance (baseline-var, or error-var) file (NOT USED!!!)"},

        { "-varfloor",
          CMD_LN_FLOAT32,
	  CMD_LN_NO_VALIDATION,
          "1e-3",
          "Variance floor value (NOT USED!!!)"},

        { NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION,    CMD_LN_NO_DEFAULT, NULL }
    };
#define CMD_LN_DEFN_H

#endif /* CMD_LN_DEFN_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.5  2004/08/07  20:41:00  arthchan2003
 * texFormat.pl file to handle < and >
 * 
=======
 * Revision 1.4  2004/08/07 19:51:52  arthchan2003
 * Make compiler happy about the help messages
 *
 * Revision 1.3  2004/08/03 07:31:17  arthchan2003
 * make the changes more latex friendly\n
 *
 * Revision 1.2  2004/08/03 07:23:12  arthchan2003
 * Check in the code for usage and example of mllr_transform
 *
 * Revision 1.1  2004/07/26 05:04:20  arthchan2003
 * mllr_transform committed, it is an adaptation of Sam Joo's mllr_adapt
 *
 *
 */
