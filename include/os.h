/*************************************************************************
 * @file     : os_.h
 * @brief    : 中断/系统相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#ifndef __OS_H__
#define __OS_H__

#define DISABLE_INTERRUPT() RASIE_Basepri()
#define ENABLE_INTERRUPT() Clear_Basepri()

void PendTaskSwitch(void);

void EnterCritical(void);
void ExitCritical(void);
void RASIE_Basepri(void);
void Clear_Basepri(void);

#endif
