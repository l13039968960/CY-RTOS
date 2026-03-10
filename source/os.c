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
#include "../include/task.h"

extern TaskHandle_t TaskGetNext(void);
extern TaskHandle_t Current_TCB;

void OS_Init(void)
{
}

void PortSysTickHandler(void)
{
    if (TaskIncrementTick() == pdTRUE)
    {
        *((volatile uint32_t *)(0xE000ED04)) = 0x10000000; /*写寄存器触发PendSV中断*/
    }
}


/**
 * @brief  任务切换中断
 * @note
 */
__asm void PortPendSVCHandler(void)
{
    PRESERVE8

    mrs r0, psp     /*加载进程堆栈指针到r0*/
    isb
    stmdb r0!, {r4-r11}     /*旧任务r4-r11入栈*/

    ldr r1, =Current_TCB    /*将入栈后的栈顶保存到旧任务中*/ 
    ldr r2, [r1]         
    str r0, [r2]

    push sp!, {r1, r14}     /*保存r1，lr*/
    mov r0, #327,680    
    msr basepri, r0         /*关中断*/
    dsb
    isb
    bl TaskSwitchTCB        /*任务控制块切换*/
    mov r0, #__OS_Max_SYSInterrupt_Priority
    msr basepri, r0         /*开中断*/
    dsb
    isb
    pop sp!, {r1, r14}      /*出栈r1，lr*/

    ldr r2, [r1]            /*加载新任务栈顶*/
    ldr r0, [r2]
    ldmia r0!, {r4-r11}     /*出栈新任务的r4-r11*/

    msr psp, r0             /*保存新任务栈顶到进程堆栈*/
    isb
    bx r14
}

void PortSVCHandler(void)
{
}

__asm void StartFirstTask(void)
{
    
}
