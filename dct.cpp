
#include "dct.hpp"

void init_dct(DCTMatrixT& dct_matrix, DCTMatrixT& matrix, int N, int M)
{
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

void idct(DCTMatrixT& dct_matrix, DCTMatrixT& matrix, int N, int M){
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
 
