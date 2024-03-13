#include <stdio.h>
#include <stdbool.h>

void bits_to_hex(bool const bits[4], char *c) {

  *c = 0;

  for (char i = 0; i < 4; ++i)
    *c |= bits[i] << (3 - i);

  *c += *c > 9 ? 'A' - 10 : '0';

}

void hex_to_bits(bool bits[4], char const *c) {
  
  char const d = *c < 'A' ? *c - '0' : *c - 'A' + 10; 

  for (char i = 0; i < 4; ++i)
    bits[i] = (d >> (3 - i)) & 1;

}

int main() {

    bool byte[8] = {1, 1, 0, 1, 0, 1, 0, 1};

    char c1, c2;

    bits_to_hex(byte, &c1);
    bits_to_hex(&byte[4], &c2);

    printf("%c%c\n", c1, c2);

    bool bits1[4], bits2[4];

    hex_to_bits(bits1, &c1);
    hex_to_bits(bits2, &c2);

    for (int i = 0; i < 4; ++i)
      printf("%d", bits1[i]);

    for (int i = 0; i < 4; ++i)
      printf("%d", bits2[i]);

    printf("\n");

}
