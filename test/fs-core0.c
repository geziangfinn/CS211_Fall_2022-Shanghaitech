#include "lib.h"

int main() {
  char *a;
  a = 0x100000;
  a[0] = 'C';
  a[211] = 'D';
  print_c((a[666]));  // print_c无法打印0
  print_c(a[1]);
  exit_proc();
}