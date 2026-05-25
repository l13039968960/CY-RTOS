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
#include "../include/heap.h"

#include "stdint.h"
#include "string.h"

#include "stm32f1xx_hal.h"

#define SCB_ICS_REG (*((volatile uint32_t *)0xE000ED04))

#define NVIC_PENDSV_SYSTICK_PRIORITY_REG (*((volatile uint32_t *)0xE000ED20))

#define NVIC_SYSTICK_CTRL_REG (*((volatile uint32_t *)0xe000e010))
#define NVIC_SYSTICK_LOAD_REG (*((volatile uint32_t *)0xe000e014))
#define NVIC_SYSTICK_CURRENT_VALUE_REG (*((volatile uint32_t *)0xe000e018))

static uint32_t EnterCriticalCount;

/**
 * @brief  任务栈初始化函数
 * @param  TCB: 任务句柄
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
rState TaskStackInit(TaskHandle_t TCB)
{
//    __is_null__(TCB);

    extern void TaskExitError(void);

    /*获取任务栈顶*/
    pStack_Type TopOfStack = (pStack_Type)((uint32_t)TCB->Task_Stack + TCB->Task_SizeOfStack * sizeof(uint32_t));

    /*满足8字节对齐*/
    TopOfStack = (pStack_Type)(((uint32_t)TopOfStack + MEM_ALIGN_MASK) & ~(MEM_ALIGN_MASK));

    /*初始化栈空间*/
    memset((void *)TCB->Task_Stack, 0xa5, TCB->Task_SizeOfStack * sizeof(uint32_t));

    /*寄存器入栈*/
    TopOfStack--;
    *TopOfStack = (Mem_Type)0x01000000; // PSR
    TopOfStack--;
    *TopOfStack = (Mem_Type)TCB->Task_Fuction; // PC
    TopOfStack--;
    *TopOfStack = (Mem_Type)TaskExitError; // LR
    TopOfStack -= 5;                       // R12,R3,R2,R1,R0
    TopOfStack -= 8;                       // R11,R10,R9,R8,R7,R6,R5,R4

    /*更新栈顶指针*/
    TCB->Task_TopOfStack = TopOfStack;

    return pdTRUE;
}

/**
 * @brief  关中断
 * @note  
 */
void RASIE_Basepri(void)
{
    __asm
    {
        msr basepri, #80

        dsb
        isb
    }
}

/**
 * @brief  开中断
 * @note  
 */
void Clear_Basepri(void)
{
    __asm
    {
        msr basepri, #0

        dsb
        isb
    }
}

/**
 * @brief  进入临界区
 * @note  
 */
void EnterCritical(void)
{
    RASIE_Basepri();

    EnterCriticalCount++;
}

/**
 * @brief  退出临界区
 * @note  
 */
void ExitCritical(void)
{
    EnterCriticalCount--;

    if (EnterCriticalCount <= 0)
    {
        Clear_Basepri();
    }
}

/**
 * @brief  开启第一个任务
 * @note
 */
__asm void StartFirstTask(void)
{
    PRESERVE8

    ldr r0, =0xE000ED08 /*从向量表重新加载主堆栈指针*/
    ldr r0, [r0] 
    ldr r0, [r0]

    msr msp, r0 /*重新加载msp*/

    cpsie i /*开中断*/
    cpsie f 
    dsb 
    isb

    svc 0 /*触发SVC中断*/
    nop 
    nop
}

/**
 * @brief  底层配置开启任务调度器
 * @note  
 */
void StartTaskSchedluer(void)
{

	/*关中断*/
	

    /*设置PendSV和Systick中断优先级为最低*/
    NVIC_PENDSV_SYSTICK_PRIORITY_REG |= (uint32_t)(__OS_Min_SYSInterrupt_Priority << 16) | (uint32_t)(__OS_Min_SYSInterrupt_Priority << 24);

    /*配置Systick*/
    NVIC_SYSTICK_CTRL_REG = 0;          // CTRL
    NVIC_SYSTICK_LOAD_REG = 9000 - 1;   // LOAD
    NVIC_SYSTICK_CURRENT_VALUE_REG = 0; // VAL

    NVIC_SYSTICK_CTRL_REG = (0x0 << 2) | (0x1 << 1) | (0x1 << 0); // CTRL

    /*临界区计数器初始化*/
    EnterCriticalCount = 0;

    /*加载第一个任务*/
    StartFirstTask();
}

/**
 * @brief  停止任务调度器
 * @note  
 */
void EndTaskScheduler(void)
{
}

/**
 * @brief  触发PendSV中断函数
 * @note  
 */
void PendTaskSwitch(void)
{
    SCB_ICS_REG = 0x10000000; /*写寄存器触发PendSV中断*/
}

/**
 * @brief  Systick中断
 * @note   每1ms触发一次，判断是否需要任务切换
 */
void PortSysTickHandler(void)
{
    HAL_IncTick();
    if (TaskIncrementTick() == pdTRUE)
    {
        PendTaskSwitch();
    }
}
/**
 * @brief  PendSV中断
 * @note   实现任务切换和上下文保存与回复
 */
__asm void PortPendSVCHandler(void)
{
    extern TaskSwitchTCB
    extern CurrentTCB

    PRESERVE8

    mrs r0, psp                                     /*加载进程堆栈指针到r0*/
    isb 
    stmdb r0!, {r4 - r11}                           /*旧任务r4-r11入栈*/

    ldr r1, =CurrentTCB                             /*将入栈后的栈顶保存到旧任务中*/
    ldr r2, [r1] 
    str r0, [r2]

    stmdb sp!, {r1, r14}                             /*保存r1，lr*/
    mov r0, #80 
    msr basepri, r0                                 /*关中断*/
    dsb
    isb
    bl TaskSwitchTCB                                /*任务控制块切换*/
    mov r0, #0 
    msr basepri, r0                                 /*开中断*/
    dsb
    isb
    ldmia sp!, {r1, r14}                              /*出栈r1，lr*/

    ldr r2, [r1]                                    /*加载新任务栈顶*/
    ldr r0, [r2] 
    ldmia r0!, {r4 - r11}                           /*出栈新任务的r4-r11*/

    msr psp, r0                                     /*保存新任务栈顶到进程堆栈*/
    isb
    bx r14
    nop
}

/**
 * @brief  SVC中断
 * @note    在第一次启动任务调度器时调用
 */
__asm void PortSVCHandler(void)
{
    extern CurrentTCB

    PRESERVE8

    ldr r0, =CurrentTCB     /*切换当前任务上下文*/
    ldr r1, [r0] 
    ldr r2, [r1]			/*获取栈顶指针*/

    ldmia r2!, {r4 - r11}   /*出栈r4-r11*/

    msr psp, r2              /*任务堆栈重新赋值*/
	dsb
    isb

    mov r0, #0              /*开中断*/
    msr basepri, r0

    orr r14, # 0xd           /*设置异常返回值，返回到用户级线程模式*/
    bx r14
}




