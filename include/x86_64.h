/**
 * @file x86_64.h
 * @brief x86_64アーキテクチャ関連のヘッダ
 */
#ifndef _X86_64_H_
#define _X86_64_H_

/**
 * @brief カーネル用特権リング
 */
#define PL_KERNEL 0

/**
 * @brief ユーザ用特権リング
 */
#define PL_USER 3

/**
 * @brief GDTのNULLディスクリプタの値
 */
#define GDT_NULL (CalcGDTEntry(0, 0, 0, 0))

// P,DPL=0,S,X,RW,G,L
/**
 * @brief GDTのカーネル用コードセグメントディスクリプタの値
 */
#define GDT_KERNEL_CODE (CalcGDTEntry(0, 0xfffff, CALC_ACCESS_BYTE(1, PL_KERNEL, 1, 1, 0, 1, 0), CALC_FLAGS(1, 0, 1)))

// P,DPL=0,S,RW,G,Sz
/**
 * @brief GDTのカーネル用データセグメントディスクリプタの値
 */
#define GDT_KERNEL_DATA (CalcGDTEntry(0, 0xfffff, CALC_ACCESS_BYTE(1, PL_KERNEL, 1, 0, 0, 1, 0), CALC_FLAGS(1, 1, 0)))

// P,DPL=3,S,X,RW,G,L
/**
 * @brief GDTのユーザ用コードセグメントディスクリプタの値
 */
#define GDT_USER_CODE (CalcGDTEntry(0, 0xfffff, CALC_ACCESS_BYTE(1, PL_USER, 1, 1, 0, 1, 0), CALC_FLAGS(1, 0, 1)))

// P,DPL=3,S,RW,G,Sz
/**
 * @brief GDTのユーザ用データセグメントディスクリプタの値
 */
#define GDT_USER_DATA (CalcGDTEntry(0, 0xfffff, CALC_ACCESS_BYTE(1, PL_USER, 1, 0, 0, 1, 0), CALC_FLAGS(1, 1, 0)))

// P,DPL=0,X
#define GDT_TSS_DESCRIPTOR_LOWER(base) (CalcGDTEntry(base, 104, CALC_ACCESS_BYTE(1, PL_KERNEL, 0, 1, 0, 0, 1), CALC_FLAGS(0, 0, 0)))
#define GDT_TSS_DESCRIPTOR_HIGHER(base) (CalcTSSDescriptorHigher(base))

/**
 * @brief カーネル用CSセグメントセレクタ
 * @details インデックス1,RPL0
 */
#define CS_SEGMENT_SELECTOR_KERNEL 8

/**
 * @brief ユーザ用CSセグメントセレクタ
 * @details インデックス3,RPL3
 */
#define CS_SEGMENT_SELECTOR_USER 27

/**
 * @brief ユーザ用SSセグメントセレクタ
 * @details インデックス4,RPL3
 */
#define SS_SEGMENT_SELECTOR_USER 35

extern unsigned long long gdt[7];
extern unsigned long long gdtr[2];
extern unsigned int tss[25];

unsigned char
CALC_ACCESS_BYTE(unsigned char Pr, unsigned char Priv, unsigned char S, unsigned char Ex, unsigned char DC, unsigned char RW, unsigned char Ac);
unsigned char CALC_FLAGS(unsigned char Gr, unsigned char Sz, unsigned char L);
unsigned long long CalcGDTEntry(unsigned long long base, unsigned long long limit, unsigned long long AccessByte, unsigned long long Flags);
unsigned long long CalcTSSDescriptorHigher(unsigned long long base);

void gdtInit();
void EnableCPUInterrupt(void);
unsigned long long GetCR3(void);
unsigned char InByte(unsigned short addr);
unsigned short InShort(unsigned short addr);
void OutByte(unsigned short addr, unsigned char data);
void CpuHalt(void);

#endif
