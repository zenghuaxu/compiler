#include<stdio.h>
#include<string.h>

static int foo();
extern int foo() {
  return 0;
}

static int i;
extern int i;

int main() {
      extern int i;
      int aaa[2][sizeof(typeof(int[++i]))];
      int a = 10, array[2][(int)(1?1:0.5)];
      int b = (a++, a++);
      printf("%lld\n", sizeof(i++));
      printf("%d\n", i);
      return 0;
}
