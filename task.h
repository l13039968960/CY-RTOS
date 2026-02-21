/*************************************************************************
 * @file     : task.h
 * @brief    : 任务相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright: 
 * @note     : 
 *************************************************************************/
#include "data.h"


typedef struct {
    
}TaskHandle;


/**
 * @brief  任务创建函数
 * @param  
 * @param  
 * @return 
 * @note   
 */
TaskHandle TaskCreate(void);

/**
 * @brief  任务删除函数
 * @param  taskhandle：删除的任务句柄
 * @return 
 * @note   
 */
void TaskDelete(TaskHandle taskhandle);

