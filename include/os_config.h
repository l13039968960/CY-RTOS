#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__

#define Memstack_size 512

#define PortSysTickHandler SysTick_Handler
#define PortPendSVCHandler PendSV_Handler
#define PortSVCHandler SVC_Handler

#define __OS_Max_Interrupt_Priority 5;
#define __OS_MIN_Interrupt_Priority 15;

#define __OS_Max_SYSInterrupt_Priority (__OS_Max_Interrupt_Priority << 4)

#endif