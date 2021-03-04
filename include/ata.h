#ifndef _ATA_H_
#define _ATA_H_

#define ATA_IO_BASE_PRIMARY 0x1f0
#define ATA_CONTROL_BASE_PRIMARY 0x3f6

#define ATA_IO_BASE_SECONDARY 0x170
#define ATA_CONTROL_BASE_SECONDARY 0x376

#define ATA_STATUS_REGISTER_ERR(status) (status & 0x1)
#define ATA_STATUS_REGISTER_DRQ(status) ((status >> 3) & 0x1)
#define ATA_STATUS_REGISTER_DF(status) ((status >> 5) & 0x1)
#define ATA_STATUS_REGISTER_BSY(status) ((status >> 7) & 0x1)

#define ATA_INTERRUPT_NUM 46

#define ATA_REQUEST_QUEUE_CAPACITY 10

enum ATABusSelector { ATABusPrimary, ATABusSecondary };
enum ATADriveSelector { ATADriveMaster, ATADriveSlave };

struct ATADriveInfo {
  unsigned char Present : 1;
  unsigned short CylinderNumber;
  unsigned short HeadNumber;
  unsigned short BytesOfSector;
  unsigned short SectorNumberPerTrack;
};

struct ATARequestQueueEntry {
  unsigned char IsWrite; // 0 for read, 1 for write
  unsigned int SizeOfBytes;
  unsigned char IsComplete;
  unsigned char *buffer;
};

struct ATARequestQueue {
  int HeadIndex;
  int Size;
  struct ATARequestQueueEntry queueBody[ATA_REQUEST_QUEUE_CAPACITY];
};

struct __attribute__((packed)) MBRPartitionTableEntry {
  unsigned char Attributes;
  unsigned char PartitionStartCHS[3];
  unsigned char PartitionType;
  unsigned char PartitionEndCHS[3];
  unsigned int PartitionStartLBA;
  unsigned int NumberOfSectors;
};

struct __attribute__((packed)) MasterBootRecord {
  unsigned char BootStrap[440];
  unsigned int DiskID;
  unsigned short Reserved;
  struct MBRPartitionTableEntry FirstPartitionTable;
  struct MBRPartitionTableEntry SecondPartitionTable;
  struct MBRPartitionTableEntry ThirdPartitionTable;
  struct MBRPartitionTableEntry FourthPartitionTable;
  unsigned short Signature;
};

void ATAInit();
unsigned char ATAIdentify(enum ATABusSelector busSelector, enum ATADriveSelector driveSelector);
void ATASetDriveInfo(unsigned short identifyReturn[], int driveInforIndex);
void ATARead(unsigned int lba, unsigned char buffer[512]);
unsigned char ATARequestQueuePush(unsigned char isWrite, unsigned int sizeOfBytes, unsigned char *buffer);
unsigned char ATARequestQueueFront(struct ATARequestQueueEntry *requestQueueEntry);
void ATARequestQueuePop();
#endif
