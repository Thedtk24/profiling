#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <omp.h>
#include "ppm.h"

#define TRSH 2.0
#define ITER 1024

#define SIZEX 1620
#define SIZEY 1080

struct ppm_pixel getcol(double val, double max)
{
	struct ppm_pixel c = { 0, 0, 0 };

	double q = val / max;
	if (q < 0.25) {
		c.r = (q * 4.0) * 255.0;
		c.g = 0;
		c.b = 255;
	} else if (q < 0.5) {
		c.r = (q - 0.25) * 4.0 * 255.0;
		c.g = 255;
		c.b = 255;
	} else if (q < 0.75) {
		c.r = 255;
		c.g = 255 - (q - 0.5) * 4.0 * 255.0;
		c.b = 255;
	} else {
		c.r = 255;
		c.g = 0;
		c.b = 255 - (q - 0.75) * 4.0 * 255.0;
	}

	return c;
}

float cx(int x)
{
	/* -2 ---> 1 */
	static const float qx = 3.0 / (float)SIZEX;
	return -2.0 + x * qx;
}

float cy(int y)
{
	/* -1 ---> 1 */
	static const float qy = 2.0 / (float)SIZEY;
	return -1.0 + y * qy;
}

int main(void)
{
	struct ppm_image im;
	ppm_image_init(&im, SIZEX, SIZEY);

	double colref = 255.0 / log(ITER);

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < SIZEX; ++i) {
		for (int j = 0; j < SIZEY; ++j) {
			float complex c = cx(i) + cy(j) * I;
			float complex z = 0;

			int iter;
			for (iter = 0; iter < ITER; ++iter) {
				float mod = cabsf(z);

				if (mod > TRSH)
					break;

				z = z*z + c;
			}

			struct ppm_pixel cc = getcol(log((double)iter), colref);
			ppm_image_setpixel(&im, i, j, cc.r, cc.g, cc.b);
		}
	}

	ppm_image_dump(&im, "m.ppm");
	ppm_image_release(&im);

	return 0;
}
