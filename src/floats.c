// abnormal float types

#include <floats.h>
#include <math.h>
#include <stdio.h>

double float8_decode(float8_t * f) {
	double exponent = pow(2, ((double) f->exponent) - 7.0);
	double mantissa = 1.0 + ((double) f->mantissa) / 9.0;
	double sign = -((f->sign * 2.0) + -1.0);
	if (f->exponent == 0.0) {
		//exponent = 1.0;
		//mantissa = ((double) f->mantissa) / 9.0;
	} else if (f->exponent == 15.0) {
		return sign / 0.0;
	}
	return sign * mantissa * exponent;
}

double float16_decode(float16_t * f) {
	double exponent = pow(2, ((double) f->exponent) - 15.0);
	double mantissa = 1.0 + ((double) f->mantissa) / 1024.0;
	double sign = -((f->sign * 2.0) + -1.0);
	if (f->exponent == 0.0) {
		//exponent = -14.0;
		mantissa = ((double) f->mantissa) / 1024.0;
	} else if (f->exponent == 31.0) {
		return sign / 0.0;
	}
	return sign * mantissa * exponent;
}

// these arent so abnormal
double float32_decode(float * f) {
	return *f;
}

double float64_decode(double * f) {
	return *f;
}
