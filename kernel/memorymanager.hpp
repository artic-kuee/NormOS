#pragma once

#include "memory_map.hpp"
#include "error.hpp"

#include <array>
#include <limits>

size_t ShowAvailablePages(void);
void SetupIPageTable(void);

WithError<size_t> Allocate(size_t num_frames);
void Free(size_t start_frame, size_t num_frames);

void SetupMemoryManager(const MemoryMap& memmap);
Error SetupHeap(void);