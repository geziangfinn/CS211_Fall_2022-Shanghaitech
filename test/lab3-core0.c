#include "lib.h"

int main() {
  char *a;
  a = 0x100000;
  a[0] = 'C';
  print_c(a[257]);
  print_d(a[666]);
  exit_proc();
}