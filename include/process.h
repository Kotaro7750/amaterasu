/**
 * @file process.h
 * @brief タスク関連のヘッダ
 */
#ifndef _PROCESS_H_
#define _PROCESS_H_

void execHandler(unsigned long long entryPoint);
void exitHandler(unsigned long long status);
#endif
