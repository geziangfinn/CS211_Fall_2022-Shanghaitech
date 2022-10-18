#include "lib.h"

const int M = 10;
int ackermann(int m, int n) {
  if (m == 0) {
    return n + 1;
  } else if (m > 0 && n == 0) {
    return ackermann(m - 1, 1);
  } else {
    return ackermann(m - 1, ackermann(m, n - 1));
  }
}

void matmulti(int a[M][M], int b[M][M], int c[M][M], int M) {
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < M; ++j) {
      c[i][j] = 0;
      for (int k = 0; k < M; ++k) {
        c[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

void quicksort(int *a, int begin, int end) {
  int i, j, t, pivot;
  if (begin > end) return;

  pivot = a[begin];
  i = begin;
  j = end;
  while (i != j) {
    while (a[j] >= pivot && i < j) j--;
    while (a[i] <= pivot && i < j) i++;
    if (i < j) {
      t = a[i];
      a[i] = a[j];
      a[j] = t;
    }
  }
  a[begin] = a[i];
  a[i] = pivot;

  quicksort(a, begin, i - 1);
  quicksort(a, i + 1, end);
}

int main() {
  int A[M][M], B[M][M], C[M][M];

  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < M; ++j) {
      A[i][j] = i;
      B[i][j] = j;
      C[i][j] = 0;
    }
  }

  print_s("The content of A is: \n");
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < M; ++j) {
      print_d(A[i][j]);
      print_s(" ");
    }
    print_s("\n");
  }

  print_s("The content of B is: \n");
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < M; ++j) {
      print_d(B[i][j]);
      print_s(" ");
    }
    print_s("\n");
  }

  matmulti(A, B, C, M);

  print_s("The content of C=A*B is: \n");
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < M; ++j) {
      print_d(C[i][j]);
      print_s(" ");
    }
    print_s("\n");
  }

  const int N = 10;
  int a[10] = {5, 3, 5, 6, 7, 1, 3, 5, 6, 1};

  print_s("Prev A: ");
  for (int i = 0; i < N; ++i) {
    print_d(a[i]);
    print_s(" ");
  }
  print_s("\n");

  print_s("Sorted A: ");
  quicksort(a, 0, N - 1);
  for (int i = 0; i < N; ++i) {
    print_d(a[i]);
    print_s(" ");
  }
  print_s("\n");

  const int M = 100;
  int b[100];
  for (int i = 0; i < 100; ++i) {
    b[i] = 100 - i;
  }
  print_s("Prev B: ");
  for (int i = 0; i < M; ++i) {
    print_d(b[i]);
    print_s(" ");
  }
  print_s("\n");

  print_s("Sorted B: ");
  quicksort(b, 0, M - 1);
  for (int i = 0; i < M; ++i) {
    print_d(b[i]);
    print_s(" ");
  }
  print_s("\n");

  for (int i = 0; i <= 3; ++i) {
    for (int j = 0; j <= 4; ++j) {
      int result = ackermann(i, j);
      print_s("Ackermann(");
      print_d(i);
      print_s(",");
      print_d(j);
      print_s(") = ");
      print_d(result);
      print_c('\n');
    }
  }

  exit_proc();
}