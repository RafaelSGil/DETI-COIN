//
// Tomás Oliveira e Silva,  October 2024
//
// Arquiteturas de Alto Desempenho 2024/2025
//
// deti_coins_cpu_search() --- find DETI coins using md5_cpu()
//

typedef unsigned int u32_t;
typedef unsigned char u08_t;

#include<stdio.h>
#include<stdlib.h>

#include "md5.h"

int inc(u32_t* v){
    u32_t vv = *v;
    
    vv++;
    if ((vv & 0xFF) == 0x7F) {
        vv += 0xA1;
        
        if ((vv & 0xFF00) == 0x7F00) {
            vv += 0xA100;

            if ((vv & 0xFF0000) == 0x7F0000) {
                vv += 0xA10000;
                
                if ((vv & 0xFF000000) == 0x7F000000) {
                    vv += 0xA1000000;
                    *v = vv;
                    return 1;
                }
            }
        }
    }
    
    *v = vv;
    return 0;
}

int main(void)
{
  u32_t idx,coin[13u],hash[4u];
  u32_t n_attempts,n_coins;
  u08_t *bytes;

  bytes = (u08_t *)&coin[0];
  //
  // mandatory for a DETI coin
  //
  bytes[0u] = 'D';
  bytes[1u] = 'E';
  bytes[2u] = 'T';
  bytes[3u] = 'I';
  bytes[4u] = ' ';
  bytes[5u] = 'c';
  bytes[6u] = 'o';
  bytes[7u] = 'i';
  bytes[8u] = 'n';
  bytes[9u] = ' ';
  //
  // arbitrary, but printable utf-8 data terminated with a '\n' is hightly desirable
  //
  for(idx = 10u;idx < 13u * 4u - 1u;idx++)
    bytes[idx] = ' ';
  //
  // mandatory termination
  //
  bytes[13u * 4u -  1u] = '\n';
  //
  // find DETI coins
  //

  u32_t v1 = 0x20202020; // '_ _ _ _' -> 4 spaces
  u32_t v2 = 0x20202020; // '_ _ _ _' -> 4 spaces
  for(n_attempts = n_coins = 0ul;n_attempts <= 1000000000u;n_attempts++)
  {

    coin[10] = v1;
    coin[11] = v2;
    //
    // compute MD5 hash
    //
      u32_t a,b,c,d,state[4],x[16];
# define C(c)         (c)
# define ROTATE(x,n)  (((x) << (n)) | ((x) >> (32 - (n))))
# define DATA(idx)    coin[idx]
# define HASH(idx)    hash[idx]
# define STATE(idx)   state[idx]
# define X(idx)       x[idx]
  CUSTOM_MD5_CODE();
# undef C
# undef ROTATE
# undef DATA
# undef HASH
# undef STATE
# undef X
    //
    // byte-reverse each word (that's how th
    //
    // if the number of trailing zeros is >= 32 we have a DETI coin
    //
    if(hash[3] == 0)
    {
      for (u32_t i = 0; i < 13u; i++)
      {
        printf("0x%08X%c", coin[i], (i==12) ? '\n' : ' ');
      }
      n_coins++;
    }

    if (inc(&v1) == 1) {
            inc(&v2);
        }
    
  }

  printf("deti_coins_webassembly_search: %u DETI coin%s found in %u attempt%s (expected %.2f coins)\n",n_coins,(n_coins == 1u) ? "" : "s",n_attempts,(n_attempts == 1ul) ? "" : "s",(double)n_attempts / (double)(1ul << 32));
}
