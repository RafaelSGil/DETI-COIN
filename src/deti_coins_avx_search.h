#ifndef DETI_COINS_AVX_SEARCH
#define DETI_COINS_AVX_SEARCH

#include <immintrin.h>
#include <stdio.h>
#include <math.h>

// https://excalidraw.com/#room=6a520f5eedd97fcfb782,9vH5OWVrSmifDALlJm0x_g

// function to increment hexadecimal interleaved_coins within the range of the hexadecimal ASCII values,
// whenever a byte reaches 0x7F, it is reset to 0x20 and the next byte is incremented,
// returning 1 when the most significant byte has been reset to 0x20
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

static void deti_coins_avx_search(void)
{
    u32_t n, idx, lane;
    u64_t n_attempts = 0, n_coins = 0;

    static u32_t interleaved_coins[13][4];  // Changed to a 1D array
    static u32_t interleaved_hashes[4][4]; 

    for (lane = 0; lane < 4; lane++) {
        for (idx = 5; idx < 12; idx++) {
            interleaved_coins[idx][lane] = 0x20202020;
        }

        interleaved_coins[12][lane] = 0x0a202020;
        /* interleaved_coins[12][lane] = 0x2020200a; */
    }

    interleaved_coins[0][0] = interleaved_coins[0][1] = interleaved_coins[0][2] = interleaved_coins[0][3] = 0x49544544; // 'ITED'
    interleaved_coins[1][0] = interleaved_coins[1][1] = interleaved_coins[1][2] = interleaved_coins[1][3] = 0x696f6320; // 'ioc_'
    interleaved_coins[2][0] = 0x3030206e; // '00_n'
    interleaved_coins[2][1] = 0x3131206e; // '11_n'
    interleaved_coins[2][2] = 0x3232206e; // '22_n'
    interleaved_coins[2][3] = 0x3333206e; // '33_n'

    /* interleaved_coins[0][0] = interleaved_coins[0][1] = interleaved_coins[0][2] = interleaved_coins[0][3] = 0x44455449; // 'DETI'
    interleaved_coins[1][0] = interleaved_coins[1][1] = interleaved_coins[1][2] = interleaved_coins[1][3] = 0x20636F69; // '_coi'
    interleaved_coins[2][0] = 0x6E203030; // 'n_00'
    interleaved_coins[2][1] = 0x6E203131; // 'n_11'
    interleaved_coins[2][2] = 0x6E203232; // 'n_22'
    interleaved_coins[2][3] = 0x6E203333; // 'n_33' */

    // variables that will change value in run-time,
    // allowing to try next combination (byte range: 0x20..0x7E)
    u32_t v1 = 0x20202020; // '_ _ _ _' -> 4 spaces
    u32_t v2 = 0x20202020; // '_ _ _ _' -> 4 spaces

    /* interleaved_coins[3][0] = interleaved_coins[3][1] = interleaved_coins[3][2] = interleaved_coins[3][3] = v1;
    interleaved_coins[4][0] = interleaved_coins[4][1] = interleaved_coins[4][2] = interleaved_coins[4][3] = v2;

    printf("interleaved_coins matrix:\n");

    for (idx = 0; idx < 13; idx++) {
        for (lane = 0; lane < 4; lane++) {
            printf("0x%08x ", interleaved_coins[idx][lane]);
        }
        printf("\n");
    }

    for (idx = 0; idx < 13; idx++) {
        for (lane = 0; lane < 4; lane++) {
            // Interpret each 32-bit integer as four characters
            char *char_ptr = (char *)&interleaved_coins[idx][lane];
            printf("%c%c%c%c ", char_ptr[0], char_ptr[1], char_ptr[2], char_ptr[3]);
        }
        printf("\n");
    }  */

    while (!stop_request) {
        // attribute the changing variables to the data matrix
        interleaved_coins[3][0] = interleaved_coins[3][1] = interleaved_coins[3][2] = interleaved_coins[3][3] = v1;
        interleaved_coins[4][0] = interleaved_coins[4][1] = interleaved_coins[4][2] = interleaved_coins[4][3] = v2;

        md5_cpu_avx((v4si *)interleaved_coins, (v4si *)interleaved_hashes);

        for (lane = 0; lane < 4; lane++) {
            hash_byte_reverse(&interleaved_hashes[0][lane]);

            n = deti_coin_power(&interleaved_hashes[0][lane]);

            if (n >= 32u) {
                for (idx = 0; idx < 13; idx++) {
                    // Print each element in hexadecimal format
                    printf("0x%08x ", interleaved_coins[idx][lane]);
                    
                    // Print each element as characters
                    char *char_ptr = (char *)&interleaved_coins[idx][lane];
                    printf("(%c%c%c%c)\n", char_ptr[0], char_ptr[1], char_ptr[2], char_ptr[3]);
                }
                printf("\n");

                u32_t arr[13];

                for(int i = 0; i < 13; i++){
                    arr[i] = interleaved_coins[i][lane];
                } 

                save_deti_coin(arr);
                n_coins++;
            }
        }

        n_attempts += 4;

        if(inc(&v1) == 1){
            inc(&v2);
        }
    }

    STORE_DETI_COINS();

    // Calculate theoretical expectation
    double attempts_per_coin = pow(2.0, 32.0);  // 2^32 attempts expected per coin
    double expected_coins = (double)n_attempts / attempts_per_coin;

    printf("deti_coins_avx_search: %lu DETI coin%s found in %lu attempt%s\n",
           n_coins, (n_coins == 1ul) ? "" : "s",
           n_attempts, (n_attempts == 1ul) ? "" : "s");
    printf("Expected coins: %.2f (based on %lu attempts)\n", expected_coins, n_attempts);
    printf("Search efficiency: %.2f%%\n",
           (n_coins > 0 && expected_coins > 0) ? ((double)n_coins / expected_coins) * 100.0 : 0.0);
}

#endif
