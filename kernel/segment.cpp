#include "segment.hpp"
#include "asmfunc.h"
#include "x86_descriptor.hpp"

namespace {
	const int SizeofGDTEntry =  256;
	std::array<SegmentDescriptor, SizeofGDTEntry> gdt;
	std::array<uint32_t, 26> tss;
	alignas(16) uint8_t tss_stack[4096];

	void SetGDTEntry(SegmentDescriptor& desc, DescriptorType type, unsigned int descriptor_privilege_level, uint32_t base, uint32_t limit){
		desc.data = 0;
		desc.bits.base_low = base & 0xffffu;
		desc.bits.base_middle = (base >> 16) & 0xffu;
		desc.bits.base_high = (base >> 24) & 0xffu;
		desc.bits.limit_low = limit & 0xffffu;
		desc.bits.limit_high = (limit >> 16) & 0xfu;
		desc.bits.type = type;
		desc.bits.descriptor_privilege_level = descriptor_privilege_level;
		desc.bits.present = 1;
		desc.bits.available = 0;
		desc.bits.granularity = 1;
	}

	void SetCodeSegment(SegmentDescriptor& desc, DescriptorType type, unsigned int descriptor_privilege_level, uint32_t base, uint32_t limit){
		SetGDTEntry(desc, type, descriptor_privilege_level, base, limit);
		desc.bits.system_segment = 1;
		desc.bits.long_mode = 1;
		desc.bits.default_operation_size = 0;
	}
	void SetDataSegment(SegmentDescriptor& desc, DescriptorType type, unsigned int descriptor_privilege_level, uint32_t base, uint32_t limit){
        SetGDTEntry(desc, type, descriptor_privilege_level, base, limit);
		desc.bits.system_segment = 1;
        desc.bits.long_mode = 0;
        desc.bits.default_operation_size = 1;
    }
	void SetSystemSegment(SegmentDescriptor& desc, DescriptorType type, unsigned int descriptor_privilege_level, uint32_t base, uint32_t limit){
		SetGDTEntry(desc, type, descriptor_privilege_level, base, limit);
		desc.bits.system_segment = 0;
		desc.bits.long_mode = 0;
		desc.bits.default_operation_size = 0;
	}

}


void SetupSegment(void){
	gdt[0].data = 0;
	SetCodeSegment(gdt[1], DescriptorType::kExecuteRead, 0, 0, 0xfffff);
	SetCodeSegment(gdt[4], DescriptorType::kExecuteRead, 3, 0, 0xfffff);
	SetDataSegment(gdt[2], DescriptorType::kReadWrite, 0, 0, 0xfffff);
	SetDataSegment(gdt[3], DescriptorType::kReadWrite, 3, 0, 0xfffff);
	tss[1] = reinterpret_cast<uint64_t>(tss_stack + 8*sizeof(tss_stack)) & 0xffffffff;
	tss[2] = reinterpret_cast<uint64_t>(tss_stack + 8*sizeof(tss_stack)) >> 32;
	SetSystemSegment(gdt[5], DescriptorType::kTSSAvailable, 0, reinterpret_cast<uint64_t>(&tss[0]) & 0xffffffff, sizeof(tss) - 1);
	LoadGDT(sizeof(gdt) - 1, reinterpret_cast<uintptr_t>(&gdt[0]));
	InitSegmentResistors(1<<3, 2<<3, 0);
	LoadTR(5<<3);
}




