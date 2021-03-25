/**
 * @file process.h
 * @brief タスク関連のヘッダ
 */
#ifndef _PROCESS_H_
#define _PROCESS_H_

int execHandler(char *filename);
void exitHandler(unsigned long long status);
void KernelThread(unsigned long long fn);
#endif
