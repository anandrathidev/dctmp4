
#include "dct.hpp"
#include <array>
#include <functional>


using  ImageMat8x8 = FixedMatrix<Channels, 8, 8>;
using  DCTMatrix8x8 = FixedMatrix<int16_t, 8, 8>;

// convert from RGB to YCbCr, constants are similar to ITU-R, see https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
float rgb2y (float r, float g, float b) { return +0.299f   * r +0.587f   * g +0.114f   * b; }
float rgb2cb(float r, float g, float b) { return -0.16874f * r -0.33126f * g +0.5f     * b; }
float rgb2cr(float r, float g, float b) { return +0.5f     * r -0.41869f * g -0.08131f * b; }

// forward DCT computation "in one dimension" (fast AAN algorithm by Arai, Agui and Nakajima: "A fast DCT-SQ scheme for images")
template<typename T>
void _DCTImpl(T&& block0 ,
             T&&  block1 ,
             T&&  block2 ,
             T&&  block3 ,
             T&&  block4 ,
             T&&  block5 ,
             T&&  block6 ,
             T&&  block7  ) 
{ 
  const auto SqrtHalfSqrt = 1.306562965f; //    sqrt((2 + sqrt(2)) / 2) = cos(pi * 1 / 8) * sqrt(2)
  const auto InvSqrt      = 0.707106781f; // 1 / sqrt(2)                = cos(pi * 2 / 8)
  const auto HalfSqrtSqrt = 0.382683432f; //     sqrt(2 - sqrt(2)) / 2  = cos(pi * 3 / 8)
  const auto InvSqrtSqrt  = 0.541196100f; // 1 / sqrt(2 - sqrt(2))      = cos(pi * 3 / 8) * sqrt(2)

  // based on https://dev.w3.org/Amaya/libjpeg/jfdctflt.c , the original variable names can be found in my comments
  auto add07 = block0 + block7; auto sub07 = block0 - block7; // tmp0, tmp7
  auto add16 = block1 + block6; auto sub16 = block1 - block6; // tmp1, tmp6
  auto add25 = block2 + block5; auto sub25 = block2 - block5; // tmp2, tmp5
  auto add34 = block3 + block4; auto sub34 = block3 - block4; // tmp3, tmp4

  auto add0347 = add07 + add34; auto sub07_34 = add07 - add34; // tmp10, tmp13 ("even part" / "phase 2")
  auto add1256 = add16 + add25; auto sub16_25 = add16 - add25; // tmp11, tmp12

  block0 = add0347 + add1256; block4 = add0347 - add1256; // "phase 3"

  auto z1 = (sub16_25 + sub07_34) * InvSqrt; // all temporary z-variables kept their original names
  block2 = sub07_34 + z1; block6 = sub07_34 - z1; // "phase 5"

  auto sub23_45 = sub25 + sub34; // tmp10 ("odd part" / "phase 2")
  auto sub12_56 = sub16 + sub25; // tmp11
  auto sub01_67 = sub16 + sub07; // tmp12

  auto z5 = (sub23_45 - sub01_67) * HalfSqrtSqrt;
  auto z2 = sub23_45 * InvSqrtSqrt  + z5;
  auto z3 = sub12_56 * InvSqrt;
  auto z4 = sub01_67 * SqrtHalfSqrt + z5;
  auto z6 = sub07 + z3; // z11 ("phase 5")
  auto z7 = sub07 - z3; // z13
  block1 = z6 + z4; block7 = z6 - z4; // "phase 6"
  block5 = z7 + z2; block3 = z7 - z2;
}


template<typename DataType>
using  DctFunc = std::function<void (ImageMat8x8 , DCTMatrix8x8 )>;

template<typename DataType, bool isRGB>
void run_dct_RGB(const DataType *data, long width, long height, DctFunc<DataType> dct_func )
{
    long width_strides = width / 8;
    long height_strides = height / 8;
    int i, j, k, l;
    for (i = 0; i < height_strides; ++i) 
    {
        for (j = 0; j < width_strides; ++j) 
        {
            ImageMatY8x8  image_region_Y;
            ImageMatCb8x8 image_region_Cb;
            ImageMatCr8x8 image_region_Cr;
            //DCTMatrix8x8  dct_output_region;
            // copy image into tmp
            for (k = 0; k < 8; ++k) {
                for (l = 0; l < 8; ++l) {
                    int index = ((i * 8) + k) * width + ((j * 8) + l);
                    if constexpr(isRGB)
                    {
                        image_region_Y[k][l]  = rgb2y (data[index].red, data[index].green, data[index].blue);
                        image_region_Cb[k][l] = rgb2cb(data[index].red, data[index].green, data[index].blue);
                        image_region_Cr[k][l] = rgb2cr(data[index].red, data[index].green, data[index].blue);                    
                    }
                    else
                    {
                        image_region_Y[k][l]  = data[index].Y;
                        image_region_Cb[k][l] = data[index].Cb;
                        image_region_Cr[k][l] = data[index].Cr;
                    }
                    //image_region[k][l] = data[index];
                }
            }
            // Call dct function
            // DCT: rows
            for (auto row = 0; row < 8; row++)
                _DCTImpl(image_region_Y(row, 0), 
                         image_region_Y(row, 1),
                         image_region_Y(row, 2),
                         image_region_Y(row, 3),
                         image_region_Y(row, 4),
                         image_region_Y(row, 5),
                         image_region_Y(row, 6),
                         image_region_Y(row, 7) );
            for (auto col = 0; col < 8; col++)
                _DCTImpl(image_region_Y(0, col), 
                         image_region_Y(1, col),
                         image_region_Y(2, col),
                         image_region_Y(3, col),
                         image_region_Y(4, col),
                         image_region_Y(5, col),
                         image_region_Y(6, col),
                         image_region_Y(7, col) );

        }
    }
}


void init_dct8x8(ImageMat8x8 matrix, DCTMatrix8x8 dct_matrix)
{
    const int N=8;
    const int M=8;
    int i, j, u, v;
    for (u = 0; u < N; ++u) {
        for (v = 0; v < M; ++v) {
            dct_matrix(u,v) = 0;
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    dct_matrix(u,v) += matrix(i, j) * cos(M_PI/((float)N)*(i+1./2.)*u)*cos(M_PI/((float)M)*(j+1./2.)*v);
                }               
            }
        }
    }  
}

void idct(ImageMat8x8 matrix, DCTMatrix8x8 dct_matrix){
    const int N=8;
    const int M=8;
    int i, j, u, v;

    for (u = 0; u < N; ++u) {
        for (v = 0; v < M; ++v) {
          matrix(u , v) = 1/4.*dct_matrix(0, 0);
          for(i = 1; i < N; i++){
            matrix(u, v) += 1/2.*dct_matrix(i, 0);
           }
           for(j = 1; j < M; j++){
            matrix(u , v) += 1/2.*dct_matrix(0, j);
           }

           for (i = 1; i < N; i++) {
                for (j = 1; j < M; j++) {
                    matrix(u , v) += dct_matrix( i, j) * cos(M_PI/((float)N)*(u+1./2.)*i)*cos(M_PI/((float)M)*(v+1./2.)*j);
                }               
            }
            matrix(u , v) *= 2./((float)N)*2./((float)M);
        }
    }  
 }

 void write_mat(FILE *fp, float **m, int N, int M){

    int i, j;
    for(i =0; i< N; i++){
     fprintf(fp, "%f", m[i][0]);
     for(j = 1; j < M; j++){
        fprintf(fp, "\t%f", m[i][j]);
         }   
     fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
 }
 
