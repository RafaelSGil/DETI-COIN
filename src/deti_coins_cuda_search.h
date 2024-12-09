// deti_coins_cuda_search() — find DETI coins using CUDA

#if USE_CUDA > 0
#ifndef DETI_COINS_CUDA_SEARCH
#define DETI_COINS_CUDA_SEARCH

void deti_coins_cuda_search(u32_t n_random_words)
{
    u32_t idx, max_idx, random_word, custom_word_1, custom_word_2;
    u64_t n_attempts, n_coins;
    void *cu_params[4];

    random_word = (n_random_words == 0u) ? 0x20202020u : random_value_to_try_ascii();
    custom_word_1 = custom_word_2 = 0x20202020u;
    
    initialize_cuda(0, "deti_coins_cuda_kernel_search.cubin","deti_coins_cuda_kernel_search", 1024u, 0u);
    max_idx = 1u;

    for(n_attempts = n_coins = 0ul; stop_request == 0 ; n_attempts += (64ul << 20))
    {
        host_data[0] = 1u; // index zero stores the first free position in the host data array
        CU_CALL( cuMemcpyHtoD, ( device_data, (void *)host_data, (size_t)1024 * sizeof(u32_t) ) );
        cu_params[0] = &device_data;
        cu_params[1] = &random_word;
        cu_params[2] = &custom_word_1;
        cu_params[3] = &custom_word_2;

        CU_CALL( cuLaunchKernel, ( cu_kernel, (1u << 20) / 128u, 1u, 1u, 128u, 1u, 1u, 0u, (CUstream)0, &cu_params[0], NULL ) );
        CU_CALL( cuMemcpyDtoH, ( (void *)host_data, device_data, (size_t)1024 * sizeof(u32_t) ) );

        if(host_data[0] > max_idx)
        {
            max_idx = host_data[0];
        }
        for(idx = 1u; idx < host_data[0] && idx <= 1024u - 13u; idx += 13u)
        {
	    if(idx<=1024u-13u){
                save_deti_coin(&host_data[idx]);
                n_coins++;
	    }else{
            fprintf(stderr, "deti_coins_cuda_search: wasted DETI coin\n");
        }

    if(custom_word_1 != 0x7E7E7E7Eu)
    {
        custom_word_1 = next_value_to_try_ascii(custom_word_1);
    }
    else
    {
        custom_word_1 = 0x20202020u;
        custom_word_2 = next_value_to_try_ascii(custom_word_2);
    }
	}

    
    // STORE DETI COINS


    printf("deti coins cuda search: %lu DETI coin%s found in %lu attempts (expected %.2f coins)\n",
        n_coins,
        (n_coins == 1ul) ? "" : "s",
        n_attempts,
        (double)n_attempts / (double)(1ul << 32));
    printf("max_idx=%u (%u)\n", max_idx, (max_idx - 1u) / 13u);
    printf("max_idx=%u (%u)\n", max_idx, (max_idx - 1u) / 13u);

    terminate_cuda();
}
#endif
#endif
}