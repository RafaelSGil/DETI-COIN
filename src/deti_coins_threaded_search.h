#ifndef DETI_COINS_THREADED_SEARCH
#define DETI_COINS_THREADED_SEARCH

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#define N_LANES_AVX2_THREAD 8

//pthread_mutex_t coin_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int thread_id;
    u64_t n_coins;
    u64_t n_attempts;
} thread_args_t;

static void *deti_coins_worker(void *args) {
    thread_args_t *targs = (thread_args_t *)args;
    int thread_id = targs->thread_id;

    u32_t idx, n, lane;
    u32_t interleaved_coins[13 * N_LANES_AVX2_THREAD] __attribute__((aligned(16)));
    u32_t interleaved_hashes[4 * N_LANES_AVX2_THREAD] __attribute__((aligned(16))); 

    for (lane = 0; lane < N_LANES_AVX2; lane++) {
        for (idx = 5; idx < 12; idx++) {
            interleaved_coins[idx * N_LANES_AVX2 + lane] = 0x20202020;
        }

        interleaved_coins[12 * N_LANES_AVX2 + lane] = 0x0a202020;
    }

    for (lane = 0; lane < N_LANES_AVX2; lane++) {
        interleaved_coins[0 * N_LANES_AVX2 + lane] = 0x49544544; // 'ITED'
        interleaved_coins[1 * N_LANES_AVX2 + lane] = 0x696f6320; // 'ioc_'
    }

    for (lane = 0; lane < N_LANES_AVX2_THREAD; lane++) {
        // Y is the lane index in hexadecimal (lane & 0xF)
        // X is the thread ID in hexadecimal (thread_id & 0xF)
        // 0xYX206e -> 0x30 + lane for Y, 0x30 + thread_id for X
        interleaved_coins[2 * N_LANES_AVX2_THREAD + lane] = 
            (0x30 + (lane & 0xF)) << 24 |  // Y (lane index)
            (0x30 + (thread_id & 0xF)) << 16 |  // X (thread ID)
            0x206e;  // Fixed part ('_n')
    }

    u32_t v1 = 0x20202020; // '_ _ _ _' -> 4 spaces
    u32_t v2 = 0x20202020; // '_ _ _ _' -> 4 spaces

    while (!stop_request) {
        for (int i = 0; i < N_LANES_AVX2_THREAD; i++) {
            interleaved_coins[3 * N_LANES_AVX2_THREAD + i] = v1;
            interleaved_coins[4 * N_LANES_AVX2_THREAD + i] = v2;
        }

        md5_cpu_avx2((v8si *)&interleaved_coins[0], (v8si *)&interleaved_hashes[0]);

        for (lane = 0; lane < N_LANES_AVX2_THREAD; lane++) {
            u32_t hash[4];
            for (int i = 0; i < 4; i++) {
                hash[i] = interleaved_hashes[i * N_LANES_AVX2_THREAD + lane];
            }

            hash_byte_reverse(hash);

            n = deti_coin_power(hash);

            if (n >= 32u) {
                u32_t coin[13];
                for (int i = 0; i < 13; i++) {
                    coin[i] = interleaved_coins[i * N_LANES_AVX2_THREAD + lane];
                }

                //pthread_mutex_lock(&coin_mutex);
                save_deti_coin(coin);
                targs->n_coins++;
                //pthread_mutex_unlock(&coin_mutex);
            }
        }

        targs->n_attempts += 8;

        if (inc(&v1) == 1) {
            inc(&v2);
        }
    }

    return NULL;
}

static void deti_coins_threaded_search(void) {
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_args_t *thread_args = malloc(num_threads * sizeof(thread_args_t));

    if (!threads || !thread_args) {
        fprintf(stderr, "Failed to allocate memory for threads.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].n_coins = 0;
        thread_args[i].n_attempts = 0;

        if (pthread_create(&threads[i], NULL, deti_coins_worker, &thread_args[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d.\n", i);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    u64_t total_coins = 0;
    u64_t total_attempts = 0;
    for (int i = 0; i < num_threads; i++) {
        total_coins += thread_args[i].n_coins;
        total_attempts += thread_args[i].n_attempts;
    }

    free(threads);
    free(thread_args);

    STORE_DETI_COINS();

    double attempts_per_coin = pow(2.0, 32.0);
    double expected_coins = (double)total_attempts / attempts_per_coin;

    printf("deti_coins_threaded_search: %lu DETI coin%s found in %lu attempt%s\n",
           total_coins, (total_coins == 1ul) ? "" : "s",
           total_attempts, (total_attempts == 1ul) ? "" : "s");
    printf("Expected coins: %.2f (based on %lu attempts)\n", expected_coins, total_attempts);
    printf("Search efficiency: %.2f%%\n",
           (total_coins > 0 && expected_coins > 0) ? ((double)total_coins / expected_coins) * 100.0 : 0.0);
}

#endif
