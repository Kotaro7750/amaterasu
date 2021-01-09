#ifndef _PAGING_H_
#define _PAGING_H_

#define L4_PT_INDEX(address) ((address >> 39) & 0x1ff)
#define L3_PT_INDEX(address) ((address >> 30) & 0x1ff)
#define L2_PT_INDEX(address) ((address >> 21) & 0x1ff)
#define L1_PT_INDEX(address) ((address >> 12) & 0x1ff)

struct L4PTEntry {
  unsigned long long Present : 1;        // if 0, entry is disabled
  unsigned long long ReadWrite : 1;      // if 0, cannot write to 512GB region
  unsigned long long UserSupervisor : 1; // if 0, cannot access by user mode to 512GB region
  unsigned long long PageWriteThrough : 1;
  unsigned long long PageCacheDisable : 1;
  unsigned long long Accessed : 1;
  unsigned long long _ignored1 : 1;
  unsigned long long PageSize : 1; // must be 0
  unsigned long long _ignored2 : 4;
  unsigned long long L3PTPhysAddr : 40;
  unsigned long long _ignored3 : 11;
  unsigned long long ExecuteDisable : 1; // if 1, cannot fetch 512GB region
};

struct L3PTEntry {
  unsigned long long Present : 1;        // if 0, entry is disabled
  unsigned long long ReadWrite : 1;      // if 0, cannot write to 1GB region
  unsigned long long UserSupervisor : 1; // if 0, cannot access by user mode to 1GB region
  unsigned long long PageWriteThrough : 1;
  unsigned long long PageCacheDisable : 1;
  unsigned long long Accessed : 1;
  unsigned long long _ignored1 : 1;
  unsigned long long PageSize : 1; // must be 0
  unsigned long long _ignored2 : 4;
  unsigned long long L2PTPhysAddr : 40;
  unsigned long long _ignored3 : 11;
  unsigned long long ExecuteDisable : 1; // if 1, cannot fetch 1GB region
};

struct L3PTEntry1GB {
  unsigned long long Present : 1;        // if 0, entry is disabled
  unsigned long long ReadWrite : 1;      // if 0, cannot write to 1GB region
  unsigned long long UserSupervisor : 1; // if 0, cannot access by user mode to 1GB region
  unsigned long long PageWriteThrough : 1;
  unsigned long long PageCacheDisable : 1;
  unsigned long long Accessed : 1;
  unsigned long long Dirty : 1;
  unsigned long long PageSize : 1; // must be 1
  unsigned long long Global : 1;
  unsigned long long _ignored : 3;
  unsigned long long PAT : 1;
  unsigned long long Reserved : 17; // must be 0
  unsigned long long PageFramePhysAddr : 22;
  unsigned long long _ignored3 : 7;
  unsigned long long ProtectionKey : 4;
  unsigned long long ExecuteDisable : 1; // if 1, cannot fetch 1GB region
};

struct L2PTEntry {
  unsigned long long Present : 1;        // if 0, entry is disabled
  unsigned long long ReadWrite : 1;      // if 0, cannot write to 2MB region
  unsigned long long UserSupervisor : 1; // if 0, cannot access by user mode to 2MB region
  unsigned long long PageWriteThrough : 1;
  unsigned long long PageCacheDisable : 1;
  unsigned long long Accessed : 1;
  unsigned long long _ignored1 : 1;
  unsigned long long PageSize : 1; // must be 0
  unsigned long long _ignored2 : 4;
  unsigned long long L1PTPhysAddr : 40;
  unsigned long long _ignored3 : 11;
  unsigned long long ExecuteDisable : 1; // if 1, cannot fetch 2MB region
};

struct L2PTEntry2MB {
  unsigned long long Present : 1;        // if 0, entry is disabled
  unsigned long long ReadWrite : 1;      // if 0, cannot write to 2MB region
  unsigned long long UserSupervisor : 1; // if 0, cannot access by user mode to 2MB region
  unsigned long long PageWriteThrough : 1;
  unsigned long long PageCacheDisable : 1;
  unsigned long long Accessed : 1;
  unsigned long long Dirty : 1;
  unsigned long long PageSize : 1; // must be 1
  unsigned long long Global : 1;
  unsigned long long _ignored : 3;
  unsigned long long PAT : 1;
  unsigned long long Reserved : 8; // must be 0
  unsigned long long PageFramePhysAddr : 31;
  unsigned long long _ignored3 : 7;
  unsigned long long ProtectionKey : 4;
  unsigned long long ExecuteDisable : 1; // if 1, cannot fetch 2MB region
};

struct L1PTEntry {
  unsigned long long Present : 1;        // if 0, entry is disabled
  unsigned long long ReadWrite : 1;      // if 0, cannot write to 4KB region
  unsigned long long UserSupervisor : 1; // if 0, cannot access by user mode to 4KB region
  unsigned long long PageWriteThrough : 1;
  unsigned long long PageCacheDisable : 1;
  unsigned long long Accessed : 1;
  unsigned long long Dirty : 1;
  unsigned long long PAT : 1;
  unsigned long long Global : 1; // must be 0
  unsigned long long _ignored : 3;
  unsigned long long PageFramePhysAddr : 40;
  unsigned long long _ignored2 : 7;
  unsigned long long ProtectionKey : 4;
  unsigned long long ExecuteDisable : 1; // if 1, cannot fetch 4KB region
};

void PagingInit(void);
void CausePageFoult(void);
unsigned long long CalcPhyAddr(unsigned long long LinearAddr);
void _dumpPTEntry(struct L2PTEntry2MB l1ptEntry);
void _dumpPageTableInfo();

#endif
