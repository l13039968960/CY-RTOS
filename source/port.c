#include "../include/port.h"

#define SCB_ICS_REG (*((volatile uint32_t *)0xE000ED04))

#define NVIC_PENDSV_SYSTICK_PRIORITY_REG (*((volatile uint32_t *)0xE000ED20))

#define NVIC_SYSTICK_CTRL_REG (*((volatile uint32_t *)0xe000e010))
#define NVIC_SYSTICK_LOAD_REG (*((volatile uint32_t *)0xe000e014))
#define NVIC_SYSTICK_CURRENT_VALUE_REG (*((volatile uint32_t *)0xe000e018))


/**
 * @brief  底层配置开启任务调度器
 * @note  
 */
void vPortPendSVSysTickConfig(void)
{
    /*设置PendSV和Systick中断优先级*/
    NVIC_PENDSV_SYSTICK_PRIORITY_REG |= (uint32_t)(__OS_Min_SYSInterrupt_Priority << 16) | (uint32_t)(__OS_Min_SYSInterrupt_Priority << 24);

    /*配置Systick*/
    NVIC_SYSTICK_CTRL_REG = 0;          // CTRL
    NVIC_SYSTICK_LOAD_REG = 9000 - 1;   // LOAD 72Mhz
    NVIC_SYSTICK_CURRENT_VALUE_REG = 0; // VAL

    NVIC_SYSTICK_CTRL_REG = (0x0 << 2) | (0x1 << 1) | (0x1 << 0); // CTRL
}

/**
 * @brief  关中断
 * @note  
 */
void vPortRASIEBasepri(void)
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
void vPortClearBasepri(void)
{
    __asm
    {
        msr basepri, #0

        dsb
        isb
    }
}

/**
 * @brief  触发PendSV中断函数
 * @note
 */
void vPortPendTaskSwitch(void)
{
	SCB_ICS_REG = 0x10000000; /*写寄存器触发PendSV中断*/
}

/**
 * @brief  开启第一个任务
 * @note   底层汇编实现
 */
__asm void vPortStartFirstTask(void)
{
	PRESERVE8

	ldr r0, = 0xE000ED08 /*从向量表重新加载主堆栈指针*/
	ldr r0, [r0] 
	ldr r0, [r0]

	msr msp, r0 /*重新加载msp*/

	mov r0, #0 /*开中断*/
	msr basepri, r0
	cpsie i /*开中断*/
	cpsie f
	dsb
	isb

	svc 0 /*触发SVC中断*/
}

/**
 * @brief  PendSV中断
 * @note   实现任务切换和上下文保存与回复
 */
__asm void vPortPendSVHandler(void)
{
	extern vOSSwitchTCB 
	extern OSCurrentTCB

	PRESERVE8

	mrs r0, psp /*加载进程堆栈指针到r0*/
	isb
	stmdb r0 !, {r4 - r11} /*旧任务r4-r11入栈*/

	ldr r1, =OSCurrentTCB /*保存psp*/
	ldr r2, [r1] 
	str r0, [r2]

	stmdb sp !, {r1, r14} /*保存r1，lr*/
	mov r0, #80 
	msr basepri, r0			 /*关中断*/
	dsb 
	isb 
	bl vOSSwitchTCB /*任务控制块切换*/
	mov r0, #0 
	msr basepri, r0 /*开中断*/
	dsb 
	isb 
	ldmia sp !, {r1, r14} /*出栈r1，lr*/

	ldr r2, [r1] /*加载新任务栈顶*/
	ldr r0, [r2] 
	ldmia r0 !, {r4 - r11} /*出栈新任务的r4-r11*/

	msr psp, r0 /*保存新任务栈顶到进程堆栈*/
	isb
	bx r14 
}

/**
 * @brief  SVC中断
 * @note    在第一次启动任务调度器时调用
 */
__asm void vPortSVCHandler(void)
{
	extern OSCurrentTCB

	PRESERVE8

	ldr r0, =OSCurrentTCB /*切换当前任务上下文*/
	ldr r1, [r0] 
	ldr r2, [r1]

	ldmia r2 !, {r4 - r11} /*出栈r4-r11*/

	msr psp, r2 /*psp重新赋值*/
	isb

	orr r14, #0xd /*设置异常返回值，返回到线程模式,使用任务堆栈*/
	bx r14
}
