#include "ngram_trie.h"

#include "test_macros.h"
#include "pocketsphinx_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
check_bowts(ngram_trie_t *lm)
{
    ngram_trie_iter_t *ng;
    int n;

    for (n = 1; n < ngram_trie_n(lm); ++n) {
        for (ng = ngram_trie_ngrams(lm, n); ng;
             ng = ngram_trie_iter_next(ng)) {
            int32 log_bowt, new_bowt;
            ngram_trie_node_t *node = ngram_trie_iter_get(ng);

            ngram_trie_node_params(lm, node, NULL, &log_bowt);
            new_bowt = ngram_trie_calc_bowt(lm, node);
	    ngram_trie_node_print(lm, node, stdout);
	    printf(": %d %d\n", log_bowt, new_bowt);
	    if (new_bowt != 0 && new_bowt != ngram_trie_zero(lm))
		    TEST_ASSERT(abs(log_bowt - new_bowt) < 1000);
        }
    }
    return 0;
}

static int
validate(ngram_trie_t *lm)
{
    ngram_trie_iter_t *ng;
    int n;

    for (n = 1; n < ngram_trie_n(lm); ++n) {
        for (ng = ngram_trie_ngrams(lm, n); ng;
             ng = ngram_trie_iter_next(ng)) {
            ngram_trie_node_t *node = ngram_trie_iter_get(ng);
            int32 logsum = ngram_trie_node_validate(lm, node);
	    ngram_trie_node_print(lm, node, stdout);
	    printf(": %d\n", logsum);
	    TEST_ASSERT(logsum > -100);
        }
    }
    return TRUE;
}


static int
test_validation(ngram_trie_t *t)
{
	check_bowts(t);
	validate(t);
	return 0;
}

int
main(int argc, char *argv[])
{
	ngram_trie_t *t;
	dict_t *dict;
	bin_mdef_t *mdef;
	logmath_t *lmath;
	cmd_ln_t *config;
	FILE *arpafh;
	int32 prob;
	int n_used;

	config = cmd_ln_init(NULL, ps_args(), TRUE,
			     "-hmm", TESTDATADIR "/hub4wsj_sc_8k",
			     "-dict", TESTDATADIR "/bn10000.homos.dic",
			     NULL);
	ps_init_defaults(config);

	lmath = logmath_init(cmd_ln_float32_r(config, "-logbase"),
			     0, FALSE);
	mdef = bin_mdef_read(config, cmd_ln_str_r(config, "-mdef"));
	dict = dict_init(config, mdef);

#if 0 /* Test memory consumption with reasonably sized LMs - still needs some work. */
	t = ngram_trie_init(NULL, lmath);
	arpafh = popen("bzip2 -dc '"TESTDATADIR "/bn.40000.arpa.bz2'", "r");
	ngram_trie_read_arpa(t, arpafh);
	fclose(arpafh);
	ngram_trie_free(t);
#endif

	t = ngram_trie_init(dict, lmath);
	arpafh = fopen(TESTDATADIR "/bn10000.3g.arpa", "r");
	ngram_trie_read_arpa(t, arpafh);
	fclose(arpafh);

	test_validation(t);

	/* Test 1, 2, 3-gram probs without backoff. */
	prob = ngram_trie_prob(t, &n_used, "THREE", "POINT", "ZERO", NULL);
	printf("P(ZERO POINT THREE) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -25776);
	prob = ngram_trie_prob(t, &n_used, "THREE", "POINT", NULL);
	printf("P(POINT THREE) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -38960);
	prob = ngram_trie_prob(t, &n_used, "THREE", NULL);
	printf("P(THREE) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -69328);

	/* Test 3-gram probs with backoff. */
	/* Backoff to 2-gram POINT FOUR + alpha(ZERO POINT) */
	prob = ngram_trie_prob(t, &n_used, "FOUR", "POINT", "ZERO", NULL);
	printf("P(ZERO POINT FOUR) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -35600);
	/* Backoff to 2-gram SIX FOUR + alpha(ZERO) */
	prob = ngram_trie_prob(t, &n_used, "FOUR", "SIX", "ZERO", NULL);
	printf("P(ZERO SIX FOUR) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -56608);
	/* Backoff to 1-gram FOUR + alpha(ZERO SEVEN) */
	prob = ngram_trie_prob(t, &n_used, "FOUR", "SEVEN", "ZERO", NULL);
	printf("P(ZERO SEVEN FOUR) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -76496);

	arpafh = fopen("tmp.bn10000.3g.arpa", "w");
	ngram_trie_write_arpa(t, arpafh);
	fclose(arpafh);
	ngram_trie_free(t);

	t = ngram_trie_init(dict, lmath);
	arpafh = fopen("tmp.bn10000.3g.arpa", "r");
	ngram_trie_read_arpa(t, arpafh);
	fclose(arpafh);

	/* Test 1, 2, 3-gram probs without backoff. */
	prob = ngram_trie_prob(t, &n_used, "THREE", "POINT", "ZERO", NULL);
	printf("P(ZERO POINT THREE) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -25776);
	prob = ngram_trie_prob(t, &n_used, "THREE", "POINT", NULL);
	printf("P(POINT THREE) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -38960);
	prob = ngram_trie_prob(t, &n_used, "THREE", NULL);
	printf("P(THREE) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -69328);

	/* Test 3-gram probs with backoff. */
	/* Backoff to 2-gram POINT FOUR + alpha(ZERO POINT) */
	prob = ngram_trie_prob(t, &n_used, "FOUR", "POINT", "ZERO", NULL);
	printf("P(ZERO POINT FOUR) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -35600);
	/* Backoff to 2-gram SIX FOUR + alpha(ZERO) */
	prob = ngram_trie_prob(t, &n_used, "FOUR", "SIX", "ZERO", NULL);
	printf("P(ZERO SIX FOUR) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -56608);
	/* Backoff to 1-gram FOUR + alpha(ZERO SEVEN) */
	prob = ngram_trie_prob(t, &n_used, "FOUR", "SEVEN", "ZERO", NULL);
	printf("P(ZERO SEVEN FOUR) = %d = %g = %f\n",
	       prob, logmath_exp(lmath, prob), logmath_log_to_log10(lmath, prob));
	TEST_EQUAL_LOG(prob, -76496);

	ngram_trie_free(t);
	dict_free(dict);
	logmath_free(lmath);
	bin_mdef_free(mdef);
	cmd_ln_free_r(config);

	return 0;
}
