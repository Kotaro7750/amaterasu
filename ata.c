#include "include/ata.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/pic.h"
#include "include/x86_64.h"

struct ATADriveInfo driveInfo[4];
struct ATARequestQueue ATArequestQueue;

void ATAHandlerASM();

void ATAInit() {
  for (int i = 0; i < 4; i++) {
    driveInfo[i].Present = 0;
  }

  unsigned char isActive = ATAIdentify(ATABusPrimary, ATADriveMaster);
  if (isActive) {
    unsigned short identifyReturn[256];
    for (int i = 0; i < 256; i++) {
      identifyReturn[i] = InShort(ATA_IO_BASE_PRIMARY);
    }
    ATASetDriveInfo(identifyReturn, 0);
    driveInfo[0].Present = 1;
  }

  isActive = ATAIdentify(ATABusPrimary, ATADriveSlave);
  if (isActive) {
    unsigned short identifyReturn[256];
    for (int i = 0; i < 256; i++) {
      identifyReturn[i] = InShort(ATA_IO_BASE_PRIMARY);
    }
    ATASetDriveInfo(identifyReturn, 1);
    driveInfo[1].Present = 1;
  }

  isActive = ATAIdentify(ATABusSecondary, ATADriveMaster);
  if (isActive) {
    unsigned short identifyReturn[256];
    for (int i = 0; i < 256; i++) {
      identifyReturn[i] = InShort(ATA_IO_BASE_PRIMARY);
    }
    ATASetDriveInfo(identifyReturn, 2);
    driveInfo[2].Present = 1;
  }

  isActive = ATAIdentify(ATABusSecondary, ATADriveSlave);
  if (isActive) {
    unsigned short identifyReturn[256];
    for (int i = 0; i < 256; i++) {
      identifyReturn[i] = InShort(ATA_IO_BASE_PRIMARY);
    }
    ATASetDriveInfo(identifyReturn, 3);
    driveInfo[3].Present = 1;
  }

  void *handler;
  asm volatile("lea ATAHandlerASM, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(ATA_INTERRUPT_NUM, handler, 1);
  EnableInterruptOnPIC(ATA_INTERRUPT_NUM);
}

void ATAHandler() {
  puts("ATA\n");
  // TODO プライマリ決め打ちはよくない
  unsigned short ioBase = ATA_IO_BASE_PRIMARY;

  unsigned char isError = 0;
  unsigned char status = InByte(ioBase + 7);
  if (ATA_STATUS_REGISTER_ERR(status) || ATA_STATUS_REGISTER_DF(status)) {
    isError = 1;
  }

  unsigned char buffer[512];
  if (isError) {
    puts("ERROR\n");
  } else {
    for (int i = 0; i < 256; i++) {
      unsigned short data = InShort(ioBase);
      // maybe little endian
      if (data != 0) {
        puth(data);
        puts("\n");
      }
      buffer[2 * i] = data & 0xff;
      buffer[2 * i + 1] = (data >> 8) & 0xff;
    }
  }

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

void ATASetDriveInfo(unsigned short identifyReturn[], int driveInforIndex) {
  for (int i = 0; i < 256; i++) {
    unsigned short data = identifyReturn[i];
    switch (i) {
    case 1:
      driveInfo[driveInforIndex].CylinderNumber = data;
      break;
    case 3:
      driveInfo[driveInforIndex].HeadNumber = data;
      break;
    case 5:
      driveInfo[driveInforIndex].BytesOfSector = data;
      break;
    case 6:
      driveInfo[driveInforIndex].SectorNumberPerTrack = data;
      break;
    default:
      break;
    }
  }
}

void ATARead(unsigned int lba, unsigned char buffer[512]) {
  if (ATARequestQueuePush(0, 512, buffer) == 0) {
    return;
  }

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

  // unsigned char status = InByte(ioBase + 7);
  // unsigned char isError = 0;
  // while (1) {
  //  if (ATA_STATUS_REGISTER_ERR(status) || ATA_STATUS_REGISTER_DF(status)) {
  //    isError = 1;
  //    break;
  //  }
  //  if (ATA_STATUS_REGISTER_DRQ(status) && ATA_STATUS_REGISTER_BSY(status) == 0) {
  //    break;
  //  }
  //  status = InByte(ioBase + 7);
  //}

  // if (isError) {
  //} else {
  //  for (int i = 0; i < 256; i++) {
  //    unsigned short data = InShort(ioBase);
  //    // maybe little endian
  //    buffer[2 * i] = data & 0xff;
  //    buffer[2 * i + 1] = (data >> 8) & 0xff;
  //  }
  //}
}

// インデックスを返すようにしないと完了かどうかわからない
unsigned char ATARequestQueuePush(unsigned char isWrite, unsigned int sizeOfBytes, unsigned char *buffer) {
  if (ATArequestQueue.Size == ATA_REQUEST_QUEUE_CAPACITY) {
    return 0;
  }

  ATArequestQueue.Size++;
  int pushedIndex = (ATArequestQueue.HeadIndex + ATArequestQueue.Size) % ATA_REQUEST_QUEUE_CAPACITY;

  ATArequestQueue.queueBody[pushedIndex].IsWrite = isWrite;
  ATArequestQueue.queueBody[pushedIndex].SizeOfBytes = sizeOfBytes;
  ATArequestQueue.queueBody[pushedIndex].buffer = buffer;
  ATArequestQueue.queueBody[pushedIndex].IsComplete = 0;

  return 1;
}

unsigned char ATARequestQueueFront(struct ATARequestQueueEntry *requestQueueEntry) {
  if (ATArequestQueue.Size == 0) {
    return 0;
  }

  requestQueueEntry->IsWrite = ATArequestQueue.queueBody[ATArequestQueue.HeadIndex].IsWrite;
  requestQueueEntry->SizeOfBytes = ATArequestQueue.queueBody[ATArequestQueue.HeadIndex].SizeOfBytes;
  requestQueueEntry->buffer = ATArequestQueue.queueBody[ATArequestQueue.HeadIndex].buffer;

  return 1;
}

void ATARequestQueuePop() {
  if (ATArequestQueue.Size == 0) {
    return;
  }

  ATArequestQueue.HeadIndex = (ATArequestQueue.HeadIndex + 1) % ATA_REQUEST_QUEUE_CAPACITY;
  ATArequestQueue.Size--;
}
