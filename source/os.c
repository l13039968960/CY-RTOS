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

#define SCB_ICS_REG (*((volatile uint32_t *)0xE000ED04))

#define NVIC_PENDSV_SYSTICK_PRIORITY_REG (*((volatile uint32_t *)0xE000ED20))

#define NVIC_SYSTICK_CTRL_REG (*((volatile uint32_t *)0xe000e010))
#define NVIC_SYSTICK_LOAD_REG (*((volatile uint32_t *)0xe000e014))
#define NVIC_SYSTICK_CURRENT_VALUE_REG (*((volatile uint32_t *)0xe000e018))

static void RASIE_Basepri(void);
static void Clear_Basepri(void);

static uint32_t EnterCriticalCount;

/**
 * @brief  任务栈初始化函数
 * @param  TCB: 任务句柄
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
static rState TaskStackInit(TaskHandle_t TCB)
{
    __IS_NULL__(TCB)

    extern TaskExitError();
    /*获取任务栈顶*/
    pStack_Type TopOfStack = &(TCB->Task_Stack[TCB->Task_SizeOfStack - 1]);

    /*满足8字节对齐*/
    TopOfStack = ((uint32_t)TopOfStack + MEM_ALIGN_MASK) & ~(MEM_ALIGN_MASK);

    /*初始化栈空间*/
    memset(TCB->Task_Stack, 0xa5, TCB->Task_SizeOfStack * sizeof(uint32_t));

    /*寄存器入栈*/
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

void EnterCritical(void)
{
    RASIE_Basepri();
    EnterCriticalCount++;
}

void ExitCritical(void)
{
    EnterCriticalCount--;

    if (EnterCriticalCount == 0)
    {
        Clear_Basepri();
    }
}

void StartTaskSchedluer(void)
{

    /*设置PendSV和Systick中断优先级为最低*/
    NVIC_PENDSV_SYSTICK_PRIORITY_REG |= (uint32_t)(__OS_Min_SYSInterrupt_Priority << 16) | (uint32_t)(__OS_Min_SYSInterrupt_Priority << 24); 

    /*配置Systick*/
    NVIC_SYSTICK_CTRL_REG = 0;          // CTRL
    NVIC_SYSTICK_LOAD_REG = 8000 - 1;   // LOAD
    NVIC_SYSTICK_CURRENT_VALUE_REG = 0; // VAL

    NVIC_SYSTICK_CURRENT_VALUE_REG = (0x0 << 2) | (0x1 << 1) | (0x1 << 0); // CTRL

    /*临界区计数器初始化*/
    EnterCriticalCount = 0;

    /*加载第一个任务*/
    StartFirstTask();
}

void EndTaskScheduler(void)
{
}

void PortSysTickHandler(void)
{
    if (TaskIncrementTick() == pdTRUE)
    {
        SCB_ICS_REG = 0x10000000; /*写寄存器触发PendSV中断*/
    }
}

/**
 * @brief  任务上下文切换中断
 * @note
 */
__asm void PortPendSVCHandler(void)
{
    extern TaskSwitchTCB();
    extern Current_TCB;


    PRESERVE8

    mrs r0, psp                         /*加载进程堆栈指针到r0*/
    isb 
    stmdb r0!, {r4 - r11}               /*旧任务r4-r11入栈*/

    ldr r1, =Current_TCB                /*将入栈后的栈顶保存到旧任务中*/
    ldr r2, [r1] 
    str r0, [r2]

    push sp!, {r1, r14}                 /*保存r1，lr*/
    mov r0, #__OS_Max_SYSInterrupt_Priority 
    msr basepri, r0                     /*关中断*/
    dsb 
    isb 
    bl TaskSwitchTCB                    /*任务控制块切换*/
    mov r0,#0 
    msr basepri, r0                     /*开中断*/
    dsb 
    isb 
    pop sp!, {r1, r14}                  /*出栈r1，lr*/

    ldr r2, [r1]                        /*加载新任务栈顶*/
    ldr r0, [r2] 
    ldmia r0!, {r4 - r11}               /*出栈新任务的r4-r11*/

    msr psp, r0                         /*保存新任务栈顶到进程堆栈*/
    isb
    bx r14
}

/**
 * @brief  SVC中断：加载第一个任务
 * @note    在第一次启动任务调度器时调用
 */
__asm void PortSVCHandler(void)
{
    PRESERVE8

    ldr r0, =Current_TCB                /*切换当前任务上下文*/
    ldr r1, [r0] 
    ldr r2, [r1]

    ldmia r2!, {r4 - r11}               /*出栈r4-r11*/

    msr psp r2                          /*psp重新赋值*/
    isb

    mov r0, #0                          /*开中断*/
    msr basepri, r0

    orr r14, #0xd                       /*设置异常返回值，返回到任务堆栈，线程模式*/
    bx r14
}

/**
 * @brief  开启第一个任务
 * @note
 */
__asm void StartFirstTask(void)
{
    PRESERVE8

    ldr r0, =0xE000ED00                 /*从向量表重新加载主堆栈指针*/
    ldr r0, [r0] 
    ldr r0, [r0]

    msr msp r0                          /*重新加载msp*/

    cpsie i                             /*开中断*/
    cpsie f 
    dsb 
    isb

    svc 0                               /*触发SVC中断*/
    nop 
    nop
}

void RASIE_Basepri(void)
{
    __asm
    {
        msr basepri, #__OS_Max_SYSInterrupt_Priority

        dsb
        isb
    }
}

void Clear_Basepri(void)
{
    __asm
    {
        msr basepri, #0

        dsb
        isb
    }
}
