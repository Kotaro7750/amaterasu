#ifndef _X86_64_H_
#define _X86_64_H_
extern unsigned long long gdt[2];
extern unsigned long long gdtr[2];

unsigned char
CALC_ACCESS_BYTE(unsigned char Pr, unsigned char Priv, unsigned char S, unsigned char Ex, unsigned char DC, unsigned char RW, unsigned char Ac);

unsigned char CALC_FLAGS(unsigned char Gr, unsigned char Sz, unsigned char L);

unsigned long long CalcGDTEntry(unsigned long long base, unsigned long long limit, unsigned long long AccessByte, unsigned long long Flags);

void gdtInit();
void EnableCPUInterrupt(void);
unsigned long long GetCR3(void);
unsigned char InByte(unsigned short addr);
void OutByte(unsigned short addr, unsigned char data);
void CpuHalt(void);
#endif
