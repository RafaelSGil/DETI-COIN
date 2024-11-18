#ifndef DETI_COINS_AVX_SEARCH
#define DETI_COINS_AVX_SEARCH

#include <immintrin.h>
#include <stdio.h>
#include <math.h>

#define N_LANES 4

// https://excalidraw.com/#room=6a520f5eedd97fcfb782,9vH5OWVrSmifDALlJm0x_g

static void deti_coins_avx_search(void)
{
    u32_t n, idx, lane;
    u64_t n_attempts = 0, n_coins = 0;

    static u32_t interleaved_coins[13 * N_LANES] __attribute__((aligned(16)));
    static u32_t interleaved_hashes[4 * N_LANES] __attribute__((aligned(16))); 

    for (lane = 0; lane < N_LANES; lane++) {
        for (idx = 5; idx < 12; idx++) {
            interleaved_coins[idx * N_LANES + lane] = 0x20202020;
        }

        interleaved_coins[12 * N_LANES + lane] = 0x0a202020;
    }

    for (lane = 0; lane < N_LANES; lane++) {
        interleaved_coins[0 * N_LANES + lane] = 0x49544544; // 'ITED'
        interleaved_coins[1 * N_LANES + lane] = 0x696f6320; // 'ioc_'
    }

    interleaved_coins[2 * N_LANES + 0] = 0x3030206e; // '00_n'
    interleaved_coins[2 * N_LANES + 1] = 0x3131206e; // '11_n'
    interleaved_coins[2 * N_LANES + 2] = 0x3232206e; // '22_n'
    interleaved_coins[2 * N_LANES + 3] = 0x3333206e; // '33_n'

    u32_t v1 = 0x20202020; // '_ _ _ _' -> 4 spaces
    u32_t v2 = 0x20202020; // '_ _ _ _' -> 4 spaces

    while (!stop_request) {
        interleaved_coins[3 * N_LANES + 0] = interleaved_coins[3 * N_LANES + 1] = interleaved_coins[3 * N_LANES + 2] = interleaved_coins[3 * N_LANES + 3] = v1;
        interleaved_coins[4 * N_LANES + 0] = interleaved_coins[4 * N_LANES + 1] = interleaved_coins[4 * N_LANES + 2] = interleaved_coins[4 * N_LANES + 3] = v2;

        md5_cpu_avx((v4si *)&interleaved_coins[0], (v4si *)&interleaved_hashes[0]);

        for (lane = 0; lane < N_LANES; lane++) {
            // as the bytes are not stored sequently in memory, we have to get the values in to another array first
            u32_t hash[4];
            for(int i = 0; i < 4; i++){
                hash[i] = interleaved_hashes[i*N_LANES+lane];
            }

            hash_byte_reverse(hash);

            n = deti_coin_power(hash);

            if (n >= 32u) {
                // as the bytes are not stored sequently in memory, we have to get the values in to another array first
                u32_t coin[13];
                for (int i = 0; i < 13; i++)
                {
                    coin[i] = interleaved_coins[i * N_LANES + lane];
                }
                

                save_deti_coin(coin);
                n_coins++;
            }
        }

        n_attempts += 4;

        if (inc(&v1) == 1) {
            inc(&v2);
        }
    }

    STORE_DETI_COINS();

    double attempts_per_coin = pow(2.0, 32.0);
    double expected_coins = (double)n_attempts / attempts_per_coin;

    printf("deti_coins_avx_search: %lu DETI coin%s found in %lu attempt%s\n",
           n_coins, (n_coins == 1ul) ? "" : "s",
           n_attempts, (n_attempts == 1ul) ? "" : "s");
    printf("Expected coins: %.2f (based on %lu attempts)\n", expected_coins, n_attempts);
    printf("Search efficiency: %.2f%%\n",
           (n_coins > 0 && expected_coins > 0) ? ((double)n_coins / expected_coins) * 100.0 : 0.0);
}

#endif
