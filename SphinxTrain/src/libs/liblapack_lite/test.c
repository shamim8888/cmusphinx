#include <stdio.h>

#include <s3/mllr.h>
#include <sphinxbase/matrix.h>
#include <sphinxbase/ckd_alloc.h>

/* cc -I../../../include test.c -L../../../lib.i686-pc-linux-gnu -lutil -lmllr -llapack_lite -lcommon -lio -lm */

int
main(int argc, char *argv[])
{
	float32 **a, **ainv, d;
	int i, j;

	a = ckd_calloc_2d(3, 3, sizeof(float32));
	ainv = ckd_calloc_2d(3, 3, sizeof(float32));
	a[0][0] = a[1][1] = a[2][2] = 2.0;
	a[1][0] = 1.0;

	invert(ainv, a, 3);

	/* Should see:
	   0.500000 0.000000 0.000000 
\	   -0.250000 0.500000 0.000000 
	   0.000000 0.000000 0.500000 
	*/
	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j) {
			printf("%f ", ainv[i][j]);
		}
		printf("\n");
	}
	/* Should see 8.0 */
	printf("%f\n", determinant(a, 3));

	ckd_free_2d(a);
	ckd_free_2d(ainv);

	return 0;
}
