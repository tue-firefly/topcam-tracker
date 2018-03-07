// Includes
#include "CameraNormalization.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Type definitions

void pixeltonormalized(float * x_normalized, float * x_pixel) {
    float * focal_length = (float *) malloc(2*sizeof(float));
    *(focal_length + 0) = 1373.1;
    *(focal_length + 1) = 1377.9;
    float * principal_point = (float *) malloc(2*sizeof(float));
    *(principal_point + 0) =  959.5000;
    *(principal_point + 1) = 539.5000;
    float * distortion_coeff = (float *) malloc(5*sizeof(float));
    *(distortion_coeff + 0) = -0.0204296;
    *(distortion_coeff + 1) =  0.2404045;
    *(distortion_coeff + 2) = -0.0014367;
    *(distortion_coeff + 3) =  0.0017241;
    *(distortion_coeff + 4) =  0.0000;
    float * skewness_coeff = (float *) malloc(1*sizeof(float));
    *(skewness_coeff + 0) = 0.000;
    normalize(x_normalized, x_pixel, focal_length, principal_point, distortion_coeff, skewness_coeff);
    printf("Normalized pixel coordinates: X: %.5f; Y: %.5f\n", *(x_normalized + 0), *(x_normalized + 1));
}

// Functions
void normalize(float * x_n, float * x_kk, float * fc, float * cc, float * kc, float * alpha_c) {
    float * x_distort = (float *)  malloc(2*sizeof(float));
    // First: Subtract principal point, and divide by the focal length:
    *(x_distort + 0) = ( *(x_kk + 0) - *(cc + 0) ) / *(fc + 0);
    *(x_distort + 1) = ( *(x_kk + 1) - *(cc + 1) ) / *(fc + 1);
    // Second: undo skew
    *(x_distort + 0) -= *(alpha_c) * *(x_distort + 1);
    // Third: Compensate for lens distortion:
    float * norm_of_kc = (float *) malloc(1*sizeof(float));
    calculate_norm(norm_of_kc, kc, 5);
    if (fabs(*norm_of_kc) > pow(10,-8.0)) {
//        printf("Norm of distortion coefficients is not equal to 0. Calling comp_distortion_oulu.\n");
        comp_distortion_oulu(x_n, x_distort, kc, 5);
    }
    else {
//        printf("Norm of distortion coefficients is equal to 0. No need to remove distortion.\n");
        *(x_n + 0) = *(x_distort + 0);
        *(x_n + 1) = *(x_distort + 1);
    }
    free(x_distort);
    free(norm_of_kc);
    return;
}

void calculate_norm(float * norm_of_vector, float * vector, int length_vector) {
    // printf("Norm of vector is: %f. First element of vector is given by %f. Length is %d.", *norm_of_vector, *vector, length_vector);
    int i;
    for (i = 0; i < length_vector - 1; i++) {
        *(norm_of_vector) += pow(*(vector + i), 2.0);
    }
    *(norm_of_vector) = sqrt(*(norm_of_vector));
    return;
}

void comp_distortion_oulu(float * x_n, float * x_distort, float * kc, int length_of_kc) {
    if (length_of_kc == 1) {
        comp_distortion(x_n, x_distort, kc);
    }
    else {
        // Initial guess
        *(x_n + 0) = *(x_distort + 0);
        *(x_n + 1) = *(x_distort + 1);
 //       printf("Initial value of x[1]:%.5f ; x[2]:%.5f\n",*(x_n+0),*(x_n+1));
        // Solve iteratively
        int i;
        float * r_2 = (float *) malloc(sizeof(float));
        float * k_radial = (float *) malloc(sizeof(float));
        float * delta_x = (float *) malloc(2*sizeof(float));
        for (i = 0; i < 20; i++) {
            *(r_2) = pow(*(x_n + 0),2.0) + pow(*(x_n + 1),2.0);
            *(k_radial) = 1 + *(kc + 0) * *(r_2) + *(kc + 1) * pow(*(r_2),2.0) + *(kc + 4) * pow(*(r_2),2.0);
            *(delta_x + 0) = 2 * *(kc + 2) * *(x_n + 0) * *(x_n + 1) + *(kc + 3) * ( *(r_2) + 2 * pow(*(x_n + 0),2.0) );
            *(delta_x + 1) = *(kc + 2) * (*(r_2) + 2 * pow(*(x_n + 1),2.0)) + 2 * *(kc + 3) * *(x_n + 0) * *(x_n + 1);
            *(x_n + 0) = ( *(x_distort + 0) - *(delta_x + 0) ) / *(k_radial);
            *(x_n + 1) = ( *(x_distort + 1) - *(delta_x + 1) ) / *(k_radial);
        }
        free(r_2);
        free(k_radial);
        free(delta_x);
    }
    return;
}

void comp_distortion(float * x_n, float * x_distort, float * kc) {
    float * radius_2 = (float *) malloc(sizeof(float));
    float * radial_distortion = (float *) malloc(sizeof(float));
    float * radius_2_comp = (float *) malloc(sizeof(float));
    *(radius_2) = pow(*(x_distort + 0),2.0) + pow(*(x_distort + 1),2.0);
    *(radial_distortion) = 1 + *(kc + 0) * *(radius_2);
    *(radius_2_comp) = ( pow(*(x_distort + 0),2.0) + pow(*(x_distort + 1),2.0) ) / *(radial_distortion);
    *(radial_distortion) = 1 + *(kc + 0) * *(radius_2);
    *(x_n + 0) = *(x_distort + 0) / *(radial_distortion);
    *(x_n + 1) = *(x_distort + 1) / *(radial_distortion);
    return;
}
