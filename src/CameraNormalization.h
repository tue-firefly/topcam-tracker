void pixeltonormalized(float * x_normalized, float * x_pixel) ;
void normalize(float * x_n, float * x_kk, float * fc, float * cc, float * kc, float * alpha_c);
void calculate_norm(float * norm_of_vector, float * vector, int length_vector);
void comp_distortion_oulu(float * x_n, float * x_distort, float * kc, int length_of_kc);
void comp_distortion(float * x_n, float * x_distort, float * kc);