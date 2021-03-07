/**
 * @file ata.c
 * @brief ATAのドライバ処理
 */
#include "include/ata.h"
#include "include/fat.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/pic.h"
#include "include/x86_64.h"

struct ATARequestQueue ATArequestQueue;

void ATAHandlerASM();

void ATAInit() {
  ATArequestQueue.HeadIndex = 0;
  ATArequestQueue.Size = 0;

  for (int drive = 0; drive < 4; drive++) {
    enum ATABusSelector bus;
    if ((drive & 1) == 0) {
      bus = ATABusPrimary;
    } else {
      bus = ATABusSecondary;
    }

    enum ATADriveSelector master_slave;
    if (((drive >> 1) & 1) == 0) {
      master_slave = ATADriveMaster;
    } else {
      master_slave = ATADriveSlave;
    }

    unsigned char isActive = ATAIdentify(bus, master_slave);
    if (isActive) {
      unsigned short identifyReturn[256];
      for (int i = 0; i < 256; i++) {
        if (bus == ATABusPrimary) {
          identifyReturn[i] = InShort(ATA_IO_BASE_PRIMARY);
        } else {
          identifyReturn[i] = InShort(ATA_IO_BASE_SECONDARY);
        }
      }
      drives[drive].isValid = 1;
    }
  }

  void *handler;
  asm volatile("lea ATAHandlerASM, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(ATA_INTERRUPT_NUM, handler, 1);
  EnableInterruptOnPIC(ATA_INTERRUPT_NUM);
}

void ATAHandler() {
  // TODO プライマリ決め打ちはよくない
  unsigned short ioBase = ATA_IO_BASE_PRIMARY;

  unsigned char isError = 0;
  unsigned char status = InByte(ioBase + 7);
  if (ATA_STATUS_REGISTER_ERR(status) || ATA_STATUS_REGISTER_DF(status)) {
    isError = 1;
  }

  unsigned char buffer[512];
  struct ATARequestQueueEntry* request = ATARequestQueueFront();
  if (isError) {
    puts("ERROR\n");
  } else {
    for (int i = 0; i < 256; i++) {
      unsigned short data = InShort(ioBase);
      // maybe little endian
      request->buffer[2 * i] = data & 0xff;
      request->buffer[2 * i + 1] = (data >> 8) & 0xff;
    }
  }

  request->IsComplete = 1;
  ATARequestQueuePop();

  SendEndOfInterrupt(ATA_INTERRUPT_NUM);
}

// send IDENTIFY command and return whether device exists
unsigned char ATAIdentify(enum ATABusSelector busSelector, enum ATADriveSelector driveSelector) {
  unsigned short ioBase;

  switch (busSelector) {
  case ATABusPrimary:
    ioBase = ATA_IO_BASE_PRIMARY;
    break;
  case ATABusSecondary:
    ioBase = ATA_IO_BASE_SECONDARY;
    break;
  }

  switch (driveSelector) {
  case ATADriveMaster:
    OutByte(ioBase + 6, 0xa0);
    break;
  case ATADriveSlave:
    OutByte(ioBase + 6, 0xb0);
    break;
  }

  // set sector
  OutByte(ioBase + 2, 0x0);
  OutByte(ioBase + 3, 0x0);
  OutByte(ioBase + 4, 0x0);
  OutByte(ioBase + 5, 0x0);
  // send IDENTIFY
  OutByte(ioBase + 7, 0xec);

  unsigned char status = InByte(ioBase + 7);
  // if 0xff, drive is floating
  if (status == 0x0 || status == 0xff) {
    return 0;
  }

  while (ATA_STATUS_REGISTER_BSY(status) != 0) {
    status = InByte(ioBase + 7);
  }

  // not ATA device
  if (InByte(ioBase + 4) || InByte(ioBase + 5)) {
    return 0;
  }

  while (ATA_STATUS_REGISTER_DRQ(status) == 0 && ATA_STATUS_REGISTER_ERR(status) == 0) {
    status = InByte(ioBase + 7);
  }

  // err bit is set
  if (ATA_STATUS_REGISTER_ERR(status)) {
    return 0;
  }

  return 1;
}

int ATARead(unsigned int lba, unsigned char buffer[512]) {
  int pushedIndex = ATARequestQueuePush(0, lba, 512, buffer);
  if (pushedIndex == -1) {
    return -1;
  }

  if (ATArequestQueue.Size == 1) {
    int sectorCount = 1;
    unsigned short ioBase = ATA_IO_BASE_PRIMARY;
    unsigned short controlBase = ATA_CONTROL_BASE_PRIMARY;
    OutByte(ioBase + 6, 0xe0 | ((lba >> 24) & 0x0f));
    OutByte(controlBase, 0x0);

    OutByte(ioBase + 2, (unsigned char)sectorCount);
    OutByte(ioBase + 3, (unsigned char)(lba & 0xff));
    OutByte(ioBase + 4, (unsigned char)(lba >> 8));
    OutByte(ioBase + 5, (unsigned char)(lba >> 16));

    OutByte(ioBase + 7, 0x20);
  }
  return pushedIndex;
}

// インデックスを返す
int ATARequestQueuePush(unsigned char isWrite, unsigned int lba, unsigned int sizeOfBytes, unsigned char *buffer) {
  if (ATArequestQueue.Size == ATA_REQUEST_QUEUE_CAPACITY) {
    return -1;
  }

  int pushedIndex = (ATArequestQueue.HeadIndex + ATArequestQueue.Size) % ATA_REQUEST_QUEUE_CAPACITY;
  ATArequestQueue.Size++;

  ATArequestQueue.queueBody[pushedIndex].IsWrite = isWrite;
  ATArequestQueue.queueBody[pushedIndex].lba = lba;
  ATArequestQueue.queueBody[pushedIndex].SizeOfBytes = sizeOfBytes;
  ATArequestQueue.queueBody[pushedIndex].buffer = buffer;
  ATArequestQueue.queueBody[pushedIndex].IsComplete = 0;

  return pushedIndex;
}

struct ATARequestQueueEntry* ATARequestQueueFront(){
  return &(ATArequestQueue.queueBody[ATArequestQueue.HeadIndex]);
}

char ATARequestComplete(int index){
  return ATArequestQueue.queueBody[index].IsComplete;
}

void ATARequestQueuePop() {
  if (ATArequestQueue.Size == 0) {
    return;
  }

  ATArequestQueue.HeadIndex = (ATArequestQueue.HeadIndex + 1) % ATA_REQUEST_QUEUE_CAPACITY;
  ATArequestQueue.Size--;
}
