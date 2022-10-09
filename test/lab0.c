#include "lib.h"

/**
 * Compare and Swap
 * @param dest
 * @param new_value
 * @param old_value
 * @return success `1' or not `0'
 */
int CAS(long* dest, long new_value, long old_value) {
  int ret = 0;
  // print_d(old_value);
  asm volatile(
      "cas:lr.w t0,(%[src1]);"
      "bne t0,%[src2],fail;"
      "sc.w t0,%[src3],(%[src1]);"
      "bne t0,x0,cas;"
      "li %[dest1],0;"
      "jal x0,finish;"
      "fail:li %[dest1],1;"
      "finish:"
      : [dest1] "=r"(ret)
      : [src1] "r"(dest), [src2] "r"(old_value), [src3] "r"(new_value));

  return !ret;
}

static long dst;

int main() {
  int res;
  dst = 77;

  for (int i = 0; i < 100; ++i) {
    res = CAS(&dst, 79, i);
    if (res)
      print_s("CAS SUCCESS\n");
    else
      print_s("CAS FAIL\n");

    print_d(res);
    print_c('\n');
    print_d(dst);
    print_c('\n');
  }
}