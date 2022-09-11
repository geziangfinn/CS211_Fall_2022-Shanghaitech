#include "lib.h"

int main() {
  long test = 114514;
  long* testadd = &test;
  int sum = 0;
  int sum2 = 0;
  int add1 = 1;
  int add2 = 3;
  asm volatile("lr.d %[dest],(%[src1]);"
               : [dest] "=r"(sum)
               : [src1] "r"(&test));
  /*asm volatile(
      "add %[dest], %[srcl], %[src2];"
     //使用add指令，一个目标操作数（命名为dest）， : [dest] "=r"(sum2)
     //将add指令的目标操作数dest和C程序中的sum变量绑定。 : [srcl] "r"(add1),
        [src2] "r"(add2));  //将add指令的源操作数srec1和C程序中的add1变量绑定*/
  print_d(sum);
}
