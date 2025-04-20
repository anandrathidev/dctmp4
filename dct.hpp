
#ifndef DCT_IMAGE_HPP
#define DCT_IMAGE_HPP

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "Matrix.hpp"

void init_dct(float **DCTMatrix, float **Matrix, int N, int M);
void init_idct(float **Matrix, float **DCTMatrix, int N, int M);


#endif

