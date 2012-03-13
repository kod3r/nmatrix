
int %%INT_ABBREV%%gemm(enum CBLAS_TRANSPOSE TransA, enum CBLAS_TRANSPOSE TransB,
  const int M, const int N, const int K, const %%INT%% alpha,
  const %%INT%%* A, const int lda,
  const %%INT%%* B, const int ldb, const %%INT%% beta,
  %%INT%%* C, const int ldc)
{
  int num_rows_a, /*num_cols_a,*/ num_rows_b; // nrowa, ncola, nrowb
  int64_t temp; // use int64 since we're accumulating here, and it may well exceed the max int size.
  int i, j, l;

  if (TransA == CblasNoTrans) {
    num_rows_a = M;
    //num_cols_a = K;
  } else {
    num_rows_a = K;
    //num_cols_a = M;
  }

  if (TransB == CblasNoTrans) num_rows_b = K;
  else                        num_rows_b = N;

  // Test the input parameters
  if (TransA < 111 || TransA > 113) {
    fprintf(stderr, "RGEMM: TransA must be CblasNoTrans, CblasTrans, or CblasConjTrans\n");
    return 0;
  } else if (TransB < 111 || TransB > 113) {
    fprintf(stderr, "RGEMM: TransB must be CblasNoTrans, CblasTrans, or CblasConjTrans\n");
    return 0;
  } else if (M < 0) {
    fprintf(stderr, "RGEMM: Expected M >= 0\n");
    return 0;
  } else if (N < 0) {
    fprintf(stderr, "RGEMM: Expected N >= 0\n");
    return 0;
  } else if (K < 0) {
    fprintf(stderr, "RGEMM: Expected K >= 0\n");
    return 0;
  } else if (lda < NM_MAX(1, num_rows_a)) {
    fprintf(stderr, "RGEMM: Expected lda >= max(1, num_rows_a), with num_rows_a = %d; got lda=%d\n", num_rows_a, lda);
    return 0;
  } else if (ldb < NM_MAX(1, num_rows_b)) {
    fprintf(stderr, "RGEMM: Expected ldb >= max(1, num_rows_b), with num_rows_b = %d; got ldb=%d\n", num_rows_b, ldb);
    return 0;
  } else if (ldc < NM_MAX(1,M)) {
    fprintf(stderr, "RGEMM: Expected ldc >= max(1,M) with M=%d; got ldc=%d\n", M, ldc);
    return 0;
  }

  // Quick return if possible
  if (!M || !N || (!alpha || !K) && beta == 1) return 0;

  // For alpha = 0
  if (alpha == 0) {
    if (beta == 0) {
      for (j = 0; j < N; ++j)
        for (i = 0; i < M; ++i) C[i+j*ldc] = 0;
    } else {
      for (j = 0; j < N; ++j)
        for (i = 0; i < M; ++i) C[i+j*ldc] *= beta;
    }
    return 0;
  }

  // Start the operations
  if (TransB == CblasNoTrans) {
    if (TransA == CblasNoTrans) {
      // C = alpha*A*B+beta*C
      for (j = 0; j < N; ++j) {
        if (beta == 0) {
          for (i = 0; i < M; ++i) C[i+j*ldc] = 0;
        } else if (beta != 1) {
          for (i = 0; i < M; ++i) C[i+j*ldc] *= beta;
        }

        for (l = 0; l < K; ++l) {
          if (B[l+j*ldb] != 0) {
            temp = alpha * B[l+j*ldb];
            for (i = 0; i < M; ++i) C[i+j*ldc] += A[i+l*lda] * temp;
          }
        }
      }

    } else {

      // C = alpha*A**T*B + beta*C
      for (j = 0; j < N; ++j) {
        for (i = 0; i < M; ++i) {
          temp = 0;
          for (l = 0; l < K; ++l) temp += A[l+i*lda] * B[l+j*ldb];

          if (beta == 0)  C[i+j*ldc] = alpha*temp;
          else            C[i+j*ldc] = alpha*temp + beta*C[i+j*ldc];
        }
      }

    }

  } else if (TransA == CblasNoTrans) {

    // C = alpha*A*B**T + beta*C
    for (j = 0; j < N; ++j) {
      if (beta == 0) {
        for (i = 0; i < M; ++i) C[i+j*ldc] = 0;
      } else if (beta != 1) {
        for (i = 0; i < M; ++i) C[i+j*ldc] *= beta;
      }

      for (l = 0; l < K; ++l) {
        if (B[j+l*ldb] != 0) {
          temp = alpha * B[j+l*ldb];
          for (i = 0; i < M; ++i) C[i+j*ldc] += A[i+l*lda] * temp;
        }
      }

    }

  } else {

    // C = alpha*A**T*B**T + beta*C
    for (j = 0; j < N; ++j) {
      for (i = 0; i < M; ++i) {
        temp = 0;
        for (l = 0; l < K; ++l) temp += A[l+i*lda] * B[j+l*ldb];

        if (beta == 0) C[i+j*ldc] = alpha*temp;
        else           C[i+j*ldc] = alpha*temp + beta*C[i+j*ldc];
      }
    }

  }

  return 0;
}
