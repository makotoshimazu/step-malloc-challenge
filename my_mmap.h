#pragma once
#include <stddef.h>

extern void* mmap_from_system(size_t size);
extern void munmap_to_system(void* ptr, size_t size);