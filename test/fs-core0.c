#include "lib.h"

int main() {
  char *a;
  a = 0x100000;
  a[0] = 'A';
  a[2] = 'C';
  a[4] = 'E';
  a[211] = 'L';
  print_c((a[666]));  // print_c无法打印0
  print_c(a[1]);
  print_c(a[3]);
  print_c(a[5]);
  exit_proc();
}