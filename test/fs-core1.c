#include "lib.h"

int main() {
  char *a;
  a = 0x100000;
  a[1] = 'B';
  a[3] = 'D';
  a[5] = 'F';
  a[666] = 'K';
  print_c((a[211]));  // print_c无法打印0
  print_c(a[0]);
  print_c(a[2]);
  print_c(a[4]);
  exit_proc();
}