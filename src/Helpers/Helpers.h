#include <Arduino.h>

void uinttochar(char a[], uint32_t n) {
  if (n == 0) {
    *a = '0';
    *(a+1) = '\0';
    return;
  }
  char aux[11];
  aux[10] = '\0';
  char* auxp = aux + 10;
  int c = 1;
  while (n != 0) {
    int mod = n % 10;
    *(--auxp) = mod | 0x30;
    n /=  10;
    c++;
  }
  memcpy(a, auxp, c);
}
