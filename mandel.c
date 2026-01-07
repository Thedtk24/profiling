#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include "ppm.h"

#define TRSH 2.0
#define ITER 1024

#define WIDTH  3.0
#define HEIGHT 2.0

#define ZOOM     400
#define CENTERX -0.2190024822784844 // -0.5
#define CENTERY -0.6964558799506655

static const int SIZEY = 1080;
static const int SIZEX = (int)(SIZEY * WIDTH / HEIGHT); // Preserve aspect ratio

struct ppm_pixel getcol(double val, double max)
{
	struct ppm_pixel c = { 0, 0, 0 };

	double q = val / 4.7;
	q -= floor(q);
	if (q < 1.0/6) {
		c.r = q * 6.0 * 255.0;
		c.g = 0;
		c.b = 255;
	} else if (q < 2.0/6) {
		c.r = 255;
		c.g = (q - 1.0/6) * 6.0 * 255.0;
		c.b = 255;
	} else if (q < 3.0/6) {
		c.r = 255;
		c.g = 255;
		c.b = 255 - (q - 2.0/6) * 6.0 * 255.0;
	} else if (q < 4.0/6) {
		c.r = 255;
		c.g = 255 - (q - 3.0/6) * 6.0 * 255.0;
		c.b = 0;
	} else if (q < 5.0/6) {
		c.r = 255 - (q - 5.0/6) * 6.0 * 255.0;
		c.g = 0;
		c.b = 0;
	} else {
		c.r = 0;
		c.g = 0;
		c.b = (q - 4.0/6) * 6.0 * 255.0;
	}

	return c;
}

double cx(int x)
{
	/* -2 ---> 1 */
	static const double qx = WIDTH / ZOOM / (double)SIZEX;
	static const double left = CENTERX - WIDTH / ZOOM /2;
	return left + x * qx;
}

double cy(int y)
{
	/* -1 ---> 1 */
	static const double qy = HEIGHT / ZOOM / (double)SIZEY;
	static const double top = CENTERY - HEIGHT / ZOOM /2;
	return top + y * qy;
}

int main(void)
{
	struct ppm_image im;
	ppm_image_init(&im, SIZEX, SIZEY);

	double colref = log(ITER);

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < SIZEX; ++i) {
		for (int j = 0; j < SIZEY; ++j) {
			double complex c = cx(i) + cy(j) * I;
			double complex z = 0;

			int iter;
			for (iter = 0; iter < ITER; ++iter) {
				double mod = cabs(z);

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
