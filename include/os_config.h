#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__

#define Memstack_size 2048

#define PortSysTickHandler SysTick_Handler
#define PortPendSVCHandler PendSV_Handler
#define PortSVCHandler SVC_Handler

#define __OS_Highest_Interrupt_Priority 5
#define __OS_Lowest_Interrupt_Priority 15

#define __OS_TASK_HIGHEST_PRIORITY__ 31
#define __OS_TASK_LOWEST_PRIORITY__ 0

#define __OS_TASK_MINIMUN_STACKSIZE__ 24

#define __OS_Max_SYSInterrupt_Priority (uint32_t)(__OS_Highest_Interrupt_Priority << 4)
#define __OS_Min_SYSInterrupt_Priority (uint32_t)(__OS_Lowest_Interrupt_Priority << 4)

#endif
