void bitpop(int bitsize, int genes, int individuals, int** result);
void uniform_bit_pop(int bitsize, int genes, int individuals, float factor, float bias, int normalised, int** result);
void normal_bit_pop(int bitsize, int genes, int individuals, float factor, float bias, int normalised, float loc, float scale, int** result);
void cauchy_bit_pop(int bitsize, int genes, int individuals, float factor, float bias, int normalised, float loc, float scale, int** result);