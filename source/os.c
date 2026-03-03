/*************************************************************************
 * @file     : os_.c
 * @brief    : 中断/系统相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#include "../include/os_config.h"
#include "../include/project_def.h"

void OS_Init(void)
{
}

void PortSysTickHandler(void)
{
    if (TaskIncrementTick() == pdTRUE)
    {
        
    }
}

void PortPendSVCHandler(void)
{
}

void PortSVCHandler(void)
{
}
