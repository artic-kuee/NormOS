#include "memorymanager.hpp"
#include "asmfunc.h"

#include <array>




namespace {
	const uint64_t kPageSize4K = 4096;
	const uint64_t kPageSize2M = 512 * kPageSize4K;
	const uint64_t kPageSize1G = 512 * kPageSize2M;

	const uint64_t kBytesPerFrame = kPageSize4K;

	const size_t kPageDirectoryCount = 64;
	
	alignas(kPageSize4K) std::array<uint64_t, 512> pml4_table;
	alignas(kPageSize4K) std::array<uint64_t, 512> pdp_table;
	alignas(kPageSize4K)
    std::array<std::array<uint64_t, 512>, kPageDirectoryCount> page_directory;

	const unsigned long MaxPhysicalMemoryBytes = 128 * kPageSize1G;
	std::array<unsigned long, MaxPhysicalMemoryBytes / (kPageSize4K * 8*sizeof(unsigned long))> alloc_map; //ビットマップ　グローバル変数で定義

	void SetBit(size_t frameid){
		alloc_map[frameid / (8*sizeof(unsigned long))] |= static_cast<unsigned long>(1) << (frameid % (8*sizeof(unsigned long)));
	}
	void ResetBit(size_t frameid){
		alloc_map[frameid / (8*sizeof(unsigned long))] &= ~(static_cast<unsigned long>(1) << (frameid % (8*sizeof(unsigned long))));
	}
	bool GetBit(size_t frameid) {
		return (alloc_map[frameid / (8*sizeof(unsigned long))] >> (frameid % (8*sizeof(unsigned long)))) & static_cast<unsigned long>(1);
	}
	size_t GetBitConsecutive(size_t frameid){
		int i = 0;
		while(!GetBit(frameid + i)){
			i++;
		}
		return i;
	}

	void MarkAllocated(size_t start_frame, size_t num_frames){
		for (int i = 0; i < num_frames; i++){
			SetBit(start_frame + i);
		}
	}


	size_t range_begin = 0; //
	size_t range_end = 0;
	size_t available_pages = 0;
}


size_t ShowAvailablePages(void){
	return available_pages;
}

void SetupIPageTable(void) {
  pml4_table[0] = reinterpret_cast<uint64_t>(&pdp_table[0]) | 0x003;
  for (int i_pdpt = 0; i_pdpt < page_directory.size(); ++i_pdpt) {
    pdp_table[i_pdpt] = reinterpret_cast<uint64_t>(&page_directory[i_pdpt]) | 0x003;
    for (int i_pd = 0; i_pd < 512; ++i_pd) {
      page_directory[i_pdpt][i_pd] = i_pdpt * kPageSize1G + i_pd * kPageSize2M | 0x083;
    }
  }

  SetCR3(reinterpret_cast<uint64_t>(&pml4_table[0]));
}





WithError<size_t> Allocate(size_t num_frames){
	for(size_t i = range_begin; i < range_end; i++){
		if(GetBitConsecutive(i) > num_frames){
			MarkAllocated(i, num_frames);
			range_begin = i + num_frames;
			available_pages -= num_frames;
			return {i, MAKE_ERROR(Error::kSuccess)};
		}
	}
	for(size_t i = 0; i < range_begin; i++){
		if(GetBitConsecutive(i) > num_frames){
			MarkAllocated(i, num_frames);
			range_begin = i + num_frames;
			available_pages -= num_frames;
			return {i, MAKE_ERROR(Error::kSuccess)};
		}
	}
	return {std::numeric_limits<size_t>::max(), MAKE_ERROR(Error::kNoEnoughMemory)};
}

void Free(size_t start_frame, size_t num_frames){
	for (int i = 0; i < num_frames; i++){
		ResetBit(start_frame + i);
		available_pages += num_frames;
	}
}

void SetupMemoryManager(const MemoryMap& memory_map){
	uintptr_t map_base = reinterpret_cast<uintptr_t>(memory_map.buffer);
	for(uintptr_t i = map_base; i < map_base + memory_map.map_size; i += memory_map.descriptor_size){
		auto desc = reinterpret_cast<const MemoryDescriptor*>(i);
		if(IsAvailable(static_cast<MemoryType>(desc->type))){
			Free(desc->physical_start / kPageSize4K, desc->number_of_pages * kUEFIPageSize / kPageSize4K);
		} else {
			MarkAllocated(desc->physical_start / kPageSize4K, desc->number_of_pages * kUEFIPageSize / kPageSize4K);
		}
		if(desc->physical_start + desc->number_of_pages * kUEFIPageSize > range_end){
			range_end = desc->physical_start + desc->number_of_pages * kUEFIPageSize;
		}
	}
}

extern "C" caddr_t program_break, program_break_end;



Error SetupHeap(void) {
    const int kHeapFrames = 64 * 512;
    const auto heap_start = Allocate(kHeapFrames);
    if (heap_start.error) {
      return heap_start.error;
    }

    program_break = reinterpret_cast<caddr_t>(heap_start.value * kBytesPerFrame);
    program_break_end = program_break + kHeapFrames * kBytesPerFrame;
    return MAKE_ERROR(Error::kSuccess);
  }