#include "lib.h"

int main() {
  char *a;
  a = 0x100000;
  a[257] = 'S';
  print_d(a[211]);
  print_c(a[0]);

  exit_proc();
}