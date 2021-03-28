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

#include <process.h>

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
  unsigned int lba;
  int SizeOfBytes;
  unsigned char IsComplete;
  unsigned char *buffer;
  struct Process *process;
};

struct ATARequestQueue {
  int HeadIndex;
  int Size;
  struct ATARequestQueueEntry queueBody[ATA_REQUEST_QUEUE_CAPACITY];
};

void ATAInit();
unsigned char ATAIdentify(enum ATABusSelector busSelector, enum ATADriveSelector driveSelector);
int ATARead(unsigned int lba, int sizeOfBytes, unsigned char *buffer, char enablePoll);

void ATASendRequest(int index);
int ATARequestQueuePush(unsigned char isWrite, unsigned int lba, int sizeOfBytes, unsigned char *buffer);
struct ATARequestQueueEntry *ATARequestQueueFront();
char ATARequestComplete(int index);
void ATARequestQueuePop();
#endif
