#include "simple_malloc.h"

//
// [My malloc]
//
// Your job is to invent a smarter malloc algorithm here :)

// This is called only once at the beginning of each challenge.
void my_initialize() {
  simple_initialize();  // Rewrite!
}

// This is called every time an object is allocated. |size| is guaranteed
// to be a multiple of 8 bytes and meets 8 <= |size| <= 4000. You are not
// allowed to use any library functions other than mmap_from_system /
// munmap_to_system.
void* my_malloc(size_t size) {
  return simple_malloc(size);  // Rewrite!
}

// This is called every time an object is freed.  You are not allowed to use
// any library functions other than mmap_from_system / munmap_to_system.
void my_free(void* ptr) {
  simple_free(ptr);  // Rewrite!
}