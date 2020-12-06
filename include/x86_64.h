#ifndef _X86_64_H_
#define _X86_64_H_
extern unsigned long long gdt[3];
extern unsigned long long gdtr[2];

unsigned char CALC_ACCESS_BYTE(unsigned char Pr, unsigned char Priv,
                               unsigned char S, unsigned char Ex,
                               unsigned char DC, unsigned char RW,
                               unsigned char Ac);

unsigned char CALC_FLAGS(unsigned char Gr, unsigned char Sz, unsigned char L);

unsigned long long CalcGDTEntry(unsigned long long base,
                                unsigned long long limit,
                                unsigned long long AccessByte,
                                unsigned long long Flags);

void gdtInit();
unsigned char InByte(unsigned short addr);
#endif
