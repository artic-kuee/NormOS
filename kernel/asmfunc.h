#pragma once

#include <stdint.h>
extern "C" {
void SetStack(void);
void DisableInt(void);
void EnableInt(void);
void LoadIDT(uint16_t limit, uint64_t offset);
void LoadGDT(uint16_t limit, uint64_t offset);
void LoadTR(uint16_t sel);
void InitSegmentResistors(uint16_t cs, uint16_t ss ,uint16_t others);
void SetCR3(uint64_t value);
uint64_t GetCR3(void);


void SwitchContext(void* new_context, void* current_context);
void LoadContext(void* context);
void IntHandlerLAPICTimer();
}
