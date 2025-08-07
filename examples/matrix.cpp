#include <iostream>
#include <linalg.h>
#include "alglibmisc.h"
#include "common.h"

using namespace std;
using namespace alglib;

real_2d_array computeNormalizedLaplacian(const real_2d_array &W) {
    int n = W.rows(); // تعداد گره‌ها
    real_2d_array L;  // ماتریس لاپلاسین
    
    real_1d_array degrees, d_inv_sqrt;
    degrees.setlength(n);
    for (int i = 0; i < n; i++) {
        degrees[i] = 0;
        for (int j = 0; j < n; j++) {
            degrees[i] += W(i,j);
        }
    }

    d_inv_sqrt.setlength(n);
    for (int i = 0; i < n; i++) {
        d_inv_sqrt[i] = (degrees[i] != 0) ? 1.0 / sqrt(degrees[i]) : 0.0; 
    }

    L.setlength(n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            L(i,j) = (i==j) ? degrees[i] - W(i,j) : -W(i,j);
        }
    }
    
    real_2d_array L_N;
    L_N.setlength(n, n);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            L_N[i][j] += d_inv_sqrt[i] * L[i][j] * d_inv_sqrt[j];
        }
    }    

    return L_N;
}




sparsematrix computeNormalizedLaplacianSparse(const sparsematrix &W) {
    ae_int_t n = sparsegetnrows(W);
    real_1d_array degrees, d_inv_sqrt;
    degrees.setlength(n);
    d_inv_sqrt.setlength(n);

    integer_1d_array colidx; colidx.setlength(n);
    real_1d_array vals; vals.setlength(n);
    ae_int_t nz;
    // محاسبه درجات با استفاده از sparsegetcompressedrow
    for (int i = 0; i < n; ++i) {
        degrees[i] = 0.0;
        sparsegetcompressedrow(W, i, colidx, vals, nz);
        for (int k = 0; k < nz; ++k) {
            degrees[i] += vals[k];
        }
    }

    for (int i = 0; i < n; ++i) {
        d_inv_sqrt[i] = degrees[i] > 0 ? 1.0 / sqrt(degrees[i]) : 0.0;
    }

    // ساخت L = D - W
    sparsematrix L;
    sparsecreate(n, n, L);
    for (int i = 0; i < n; ++i) {
        sparseadd(L, i, i, degrees[i]);
        integer_1d_array colidx;
        real_1d_array vals;
        ae_int_t nz;
        sparsegetcompressedrow(W, i, colidx, vals, nz);
        for (int k = 0; k < nz; ++k) {
            int j = colidx[k];
            if (j != i) {
                sparseadd(L, i, j, -vals[k]);
            }
        }
    }
    sparseconverttocrs(L);

    // ساخت L_N = D^{-1/2} * L * D^{-1/2}
    sparsematrix L_N;
    sparsecreate(n, n, L_N);
    for (int i = 0; i < n; ++i) {
        integer_1d_array colidx;
        real_1d_array vals;
        ae_int_t nz;
        sparsegetcompressedrow(L, i, colidx, vals, nz);
        for (int k = 0; k < nz; ++k) {
            int j = colidx[k];
            double val = vals[k];
            double scaled = d_inv_sqrt[i] * val * d_inv_sqrt[j];
            sparseadd(L_N, i, j, scaled);
        }
    }
    sparseconverttocrs(L_N);

    // تبدیل به dense برای نمایش

    return L_N;
}








int main() {    
    // ae_int_t n = 10;
    // real_2d_array Wdense;
    // sparsematrix W;
    // sparsecreate(n, n, W);
    // Wdense.setlength(n, n);
    // double p = .01;
//    alglib::setglobalthreading(alglib::parallel);

    // for (int i = 0; i < n; ++i) {
    //     for (int j = 0; j < n; ++j) {
    //         double val = randomreal();
    //         bool active = randomreal() < p;
    //         if (val * active != 0.0) {
    //             sparseset(W, i, j, val);
    //             Wdense[i][j] = val;
    //         }
    //     }
    // }
    // sparseconverttocrs(W);

    real_2d_array Wdense = "[[0, 1, 1, 0], [1, 0, 1, 0], [1, 1, 0, 1], [0, 0, 1, 0]]";
    auto n = Wdense.rows();

    tic;
    auto Ln_dense = computeNormalizedLaplacian(Wdense);
    toc;


    cout << "Normalized Laplacian Matrix L_N:\n" << Ln_dense.tostring(3) << endl; 

    // tic;
    // auto L_N = computeNormalizedLaplacianSparse(W);
    // toc;

    // real_2d_array LNd;
    // LNd.setlength(n, n);
    // for (int i = 0; i < n; ++i) {
    //     real_1d_array row;
    //     sparsegetrow(L_N, i, row);
    //     for (int j = 0; j < n; ++j) {
    //         LNd[i][j] = row[j];
    //     }
    // }
    // cout << "Normalized Laplacian L_N:\n" << LNd.tostring(3) << endl;

    // sparsefree(W);

    return 0;
}
