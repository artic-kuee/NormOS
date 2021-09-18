#include "interrupt.hpp"

#include <csignal>




#include "asmfunc.h"
#include "segment.hpp"
#include "timer.hpp"

namespace{
  std::array<InterruptDescriptor, 256> idt;
}


void SetIDTEntry(int entry_num,
                 InterruptDescriptorAttribute attr,
                 uint64_t offset,
                 uint16_t segment_selector) {
  InterruptDescriptor& desc = idt[entry_num];
  desc.attr = attr;
  desc.offset_low = offset & 0xffffu;
  desc.offset_middle = (offset >> 16) & 0xffffu;
  desc.offset_high = offset >> 32;
  desc.segment_selector = segment_selector;
}

void InitInterrupt(void){
  InitTimer();
  SetIDTEntry(InterruptVector::kLAPICTimer,MakeIDTAttr(DescriptorType::kInterruptGate, 0, true, 1), reinterpret_cast<uint64_t>(IntHandlerLAPICTimer), 1<<3);
  LoadIDT(sizeof(idt) - 1, reinterpret_cast<uintptr_t>(&idt[0]));
  EnableInt();
}

void NotifyEndOfInterrupt(void) {
  volatile auto end_of_interrupt = reinterpret_cast<uint32_t*>(0xfee000b0);
  *end_of_interrupt = 0;
}

