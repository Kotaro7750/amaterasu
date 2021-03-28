/**
 * @file ata.c
 * @brief ATAのドライバ処理
 */
#include "process.h"
#include <ata.h>
#include <fat.h>
#include <graphic.h>
#include <interrupt.h>
#include <pic.h>
#include <scheduler.h>
#include <x86_64.h>

/**
 * @brief ATAへの読み書きリクエストの待ち行列
 */
struct ATARequestQueue ATArequestQueue;

void ATAHandlerASM();

char ATAPollingUntilReady(ioBase) {
  unsigned char status = InByte(ioBase + 7);
  while ((ATA_STATUS_REGISTER_BSY(status) != 0 || ATA_STATUS_REGISTER_DRQ(status) == 0) &&
         (ATA_STATUS_REGISTER_ERR(status) == 0 && ATA_STATUS_REGISTER_DF(status) == 0)) {
    status = InByte(ioBase + 7);
  }

  if (ATA_STATUS_REGISTER_ERR(status) == 1 || ATA_STATUS_REGISTER_DF(status) == 1) {
    return 0;
  } else {
    return 1;
  }
}

/**
 * @brief ディスク一覧の初期化，割り込みハンドラ登録
 */
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
  // EnableInterruptOnPIC(ATA_INTERRUPT_NUM);
}

/**
 * @brief 読み書き終了時の割り込みハンドラ
 */
void ATAHandler() {
  struct ATARequestQueueEntry *request = ATARequestQueueFront();

  wakeup(request->process);

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

/**
 * @brief 読み取りリクエストを発行する
 */
int ATARead(unsigned int lba, int sizeOfBytes, unsigned char *buffer, char enablePoll) {
  int pushedIndex = ATARequestQueuePush(0, lba, sizeOfBytes, buffer);
  if (pushedIndex == -1) {
    return -1;
  }

  if (ATArequestQueue.Size == 1) {
    ATASendRequest(pushedIndex);
  }

  if (enablePoll) {
    ATAPollingUntilReady(ATA_IO_BASE_PRIMARY);
  } else {
    sleep();
  }

  // TODO プライマリ決め打ちはよくない
  unsigned short ioBase = ATA_IO_BASE_PRIMARY;

  unsigned char isError = 0;
  unsigned char status = InByte(ioBase + 7);
  if (ATA_STATUS_REGISTER_ERR(status) || ATA_STATUS_REGISTER_DF(status)) {
    isError = 1;
  }

  struct ATARequestQueueEntry *request = ATARequestQueueFront();
  if (isError) {
    puts("ERROR\n");
    return -1;
  } else {
    if (request->IsWrite == 0) {
      for (int i = 0; i < (request->SizeOfBytes + 1) / 2; i++) {
        unsigned short data = InShort(ioBase);
        // maybe little endian
        request->buffer[2 * i] = data & 0xff;

        // when read size is odd, 2*i+1 should not be read
        if (i != ((request->SizeOfBytes + 1) / 2 - 1) || request->SizeOfBytes % 2 == 0) {
          request->buffer[2 * i + 1] = (data >> 8) & 0xff;
        }
      }
    }
  }

  request->IsComplete = 1;
  ATARequestQueuePop();

  if (ATArequestQueue.Size >= 1) {
    ATASendRequest(ATArequestQueue.HeadIndex);
  }

  return request->SizeOfBytes;
}

/**
 * @brief ATAリクエストキューの内容に従って実際にリクエストを発行する
 */
void ATASendRequest(int index) {
  struct ATARequestQueueEntry *request = &(ATArequestQueue.queueBody[index]);

  if (request->IsWrite) {
    puts("write is not supported\n");
  } else {
    int lba = request->lba;
    int sectorCount = request->SizeOfBytes % 512 == 0 ? (request->SizeOfBytes / 512) : (request->SizeOfBytes / 512 + 1);

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
}

/**
 * @brief ATAリクエストキューにプッシュする
 * @return プッシュされたインデックス．プッシュできないなかったなら-1を返す．
 */
int ATARequestQueuePush(unsigned char isWrite, unsigned int lba, int sizeOfBytes, unsigned char *buffer) {
  if (ATArequestQueue.Size == ATA_REQUEST_QUEUE_CAPACITY) {
    return -1;
  }

  int pushedIndex = (ATArequestQueue.HeadIndex + ATArequestQueue.Size) % ATA_REQUEST_QUEUE_CAPACITY;
  ATArequestQueue.Size++;

  struct ATARequestQueueEntry *entry = &(ATArequestQueue.queueBody[pushedIndex]);

  entry->IsWrite = isWrite;
  entry->lba = lba;
  entry->SizeOfBytes = sizeOfBytes;
  entry->buffer = buffer;
  entry->IsComplete = 0;
  entry->process = currentProcess;

  return pushedIndex;
}

/**
 * @brief ATAリクエストキューの先頭を取得する
 */
struct ATARequestQueueEntry *ATARequestQueueFront() {
  return &(ATArequestQueue.queueBody[ATArequestQueue.HeadIndex]);
}

char ATARequestComplete(int index) { return ATArequestQueue.queueBody[index].IsComplete; }

/**
 * @brief ATAリクエストキューからポップする
 */
void ATARequestQueuePop() {
  if (ATArequestQueue.Size == 0) {
    return;
  }

  ATArequestQueue.HeadIndex = (ATArequestQueue.HeadIndex + 1) % ATA_REQUEST_QUEUE_CAPACITY;
  ATArequestQueue.Size--;
}
