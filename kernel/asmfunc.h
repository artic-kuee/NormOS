#pragma once

extern "C" {
void SetStack(uint8_t stack_addr);
void DisableInt();
void EnableInt();
void LoadIDT(uint16_t limit, uint64_t offset);
void LoadGDT(uint16_t limit, uint64_t offset);
void LoadTR(uint16_t sel);
void InitSegmentResistors(uint16_t cs, uint16_t ss);
void SetCR3(uint64_t value);
}
