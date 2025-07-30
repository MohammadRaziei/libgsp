#include <iostream>
#include <linalg.h>
#include "alglibmisc.h"

using namespace std;
using namespace alglib;

void computeNormalizedLaplacian(const real_2d_array &W) {
    int n = W.rows(); // تعداد گره‌ها
    real_2d_array L;  // ماتریس لاپلاسین
    
    // محاسبه ماتریس درجه D (مجموع وزن‌های هر راس)
    real_1d_array degrees, d_inv_sqrt;
    degrees.setlength(n);
    for (int i = 0; i < n; i++) {
        degrees[i] = 0;
        for (int j = 0; j < n; j++) {
            degrees[i] += W[i][j];
        }
    }

    d_inv_sqrt.setlength(n);
    for (int i = 0; i < n; i++) {
        if (degrees[i] != 0) {
            d_inv_sqrt[i] = 1.0 / sqrt(degrees[i]);
        } else {
            d_inv_sqrt[i] = 0;
        }
    }

    L.setlength(n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                L[i][j] = degrees[i];
            } else {
                L[i][j] = -W[i][j];
            }
        }
    }

    real_2d_array L_N;
    L_N.setlength(n, n);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            L_N[i][j] += d_inv_sqrt[i] * L[i][j] * d_inv_sqrt[j];
        }
    }
    
    

    cout << "Normalized Laplacian Matrix L_N:" << L_N.tostring(3) << endl;
    
}

int main() {
    real_2d_array W = "[[0, 1, 1, 0], [1, 0, 1, 0], [1, 1, 0, 1], [0, 0, 1, 0]]";
    
    computeNormalizedLaplacian(W);

    return 0;
}
