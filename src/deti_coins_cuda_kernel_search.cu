typedef unsigned int u32_t;

#include "md5.h"

extern "C" __global__ __launch_bounds__(128,1) 
void deti_coins_cuda_kernel_search(u32_t *deti_coins_storage_area, u32_t custom_word_1, u32_t custom_word_2)
{
    u32_t t, n, a, b, c, d, state[4], x[16], coin[13], hash[4];

    n = (u32_t)threadIdx.x + (u32_t)blockDim.x * (u32_t)blockIdx.x;

    coin[0] = 0x49455444u;
    coin[1] = 0x696E6320u;
    coin[2] = 0x20662020u;
    coin[3] = 0x20202020u;
    coin[4] = 0x20202020u;
    coin[5] = 0x20202020u;
    coin[6] = 0x20202020u;
    coin[7] = 0x20202020u;
    coin[8] = 0x20202020u;
    coin[9] = 0x20202020u;
    coin[10] = custom_word_1;
    coin[11] = custom_word_2;
    coin[12] = 0x0A202020u;

    coin[4] += (n % 64) << 0; n /= 64;
    coin[4] += (n % 64) << 8; n /= 64;
    coin[4] += (n % 64) << 16; n /= 64;
    coin[4] += (n % 64) << 24; n /= 64;

    for(n = 0; n < 64; n++)
    {
        // 
        // Compute MD5 hash
        // 
#       define C(c) ((c))
#       define ROTATE(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#       define DATA(idx) coin[idx]
#       define HASH(idx) hash[idx]
#       define STATE(idx) state[idx]
#       define X(idx) x[idx]

        CUSTOM_MD5_CODE();
        if(hash[3] == 0){
            u32_t n = atomicAdd(deti_coins_storage_area,13);
            if(n + 13 <= 1024){
                for(t = 0; t<=12; t++){
                    deti_coins_storage_area[n + t] = coin[t];
                }
            }
        }
    }
}