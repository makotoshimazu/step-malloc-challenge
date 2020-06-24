#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "my_mmap.h"

typedef struct my_metadata {
  struct my_metadata* next;
  // The size of this free or used block including sizeof(my_metadata).
  uint32_t size;
} my_metadata_t;

const int kNumBuckets = 9;

typedef struct my_global_state {
  // 0 | 1 | 2 | 3  | 4  | 5  |  6  |  7  |  8  |
  // 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
  // bucket_id = min(n) for size + sizeof(my_metadata) < 2 ^ (n + 3), n >= 0
  my_metadata_t* buckets[kNumBuckets];
} my_global_state_t;

my_global_state_t g_state;

void my_finalize() {
  uint32_t bucket_size = 16;
  for (int i = 0; i < kNumBuckets; ++i, bucket_size *= 2) {
    int n_block = 0;
    my_metadata_t* head = g_state.buckets[i];
    while (head) {
      ++n_block;
      head = head->next;
    }
    printf("size: %d, block: %d\n", bucket_size, n_block);
  }
}

int my_global_state_bucket_id(int size) {
  int bucket_size = 16;
  int n = 0;
  while (bucket_size < size) {
    bucket_size *= 2;
    ++n;
  }
  return n;
}

void my_global_state_push(my_metadata_t* area, int bucket_id) {
  my_metadata_t* head = g_state.buckets[bucket_id];
  g_state.buckets[bucket_id] = area;
  area->next = head;
}

my_metadata_t* my_global_state_pop(int bucket_id) {
  my_metadata_t* head = g_state.buckets[bucket_id];
  if (head) {
    g_state.buckets[bucket_id] = head->next;
    head->next = NULL;
  }
  return head;
}

//
// [My malloc]
//
// Your job is to invent a smarter malloc algorithm here :)

// This is called only once at the beginning of each challenge.
void my_initialize() {
  for (int i = 0; i < 9; ++i)
    g_state.buckets[i] = NULL;
}


int my_malloc_count = 0;

// Create |size| byte (including metadata) of bucket from |area|.
// All area split are put into the global state.
void split_bucket_into(size_t size, my_metadata_t* area) {
  int target_bucket_id = my_global_state_bucket_id(size);
  int area_bucket_id = my_global_state_bucket_id(area->size);
  assert(target_bucket_id <= area_bucket_id);
  if (target_bucket_id == area_bucket_id) {
    // It's the block we want. Let's stop splitting.
    my_global_state_push(area, area_bucket_id);
    return;
  }

  // |area| needs to be split into two.
  uint32_t new_size = area->size / 2;
  my_metadata_t* area_1 = area;
  area_1->size = new_size;
  area_1->next = NULL;
  // Split the first half.
  split_bucket_into(size, area_1);
  // The latter half will be put in the bucket.
  my_metadata_t* area_2 = (my_metadata_t*)((void*)area + new_size);
  area_2->size = new_size;
  area_2->next = NULL;
  my_global_state_push(area_2, area_bucket_id - 1);
}

// This is called every time an object is allocated. |size| is guaranteed
// to be a multiple of 8 bytes and meets 8 <= |size| <= 4000. You are not
// allowed to use any library functions other than mmap_from_system /
// munmap_to_system.
void* my_malloc(size_t size) {
  size += sizeof(my_metadata_t);
  int n = my_global_state_bucket_id(size);
  my_metadata_t* area = my_global_state_pop(n);
  if (area) {
    return (void*)area + sizeof(my_metadata_t);
  }

  for (int nearest_n = n + 1; nearest_n < kNumBuckets; ++nearest_n) {
    area = my_global_state_pop(nearest_n);
    if (area) {
      split_bucket_into(size, area);
      area = my_global_state_pop(n);
      assert(area);
      return (void*)area + sizeof(my_metadata_t);
    }
  }

  // No bucket found. Create mmap.
  my_metadata_t* mapped_area = (my_metadata_t*)mmap_from_system(4096);
  mapped_area->size = 4096;
  mapped_area->next = NULL;
  split_bucket_into(size, mapped_area);
  my_metadata_t* allocated_area = my_global_state_pop(n);
  assert(allocated_area);
  return (void*)allocated_area + sizeof(my_metadata_t);
}

// This is called every time an object is freed.  You are not allowed to use
// any library functions other than mmap_from_system / munmap_to_system.
void my_free(void* ptr) {
  my_metadata_t* area = (my_metadata_t*)(ptr - sizeof(my_metadata_t));
  int n = my_global_state_bucket_id(area->size);
  my_global_state_push(area, n);
}