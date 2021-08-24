#ifndef FREERTOS_HELPER_H
#define FREERTOS_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__CCRL__)

#define R_CG_FREERTOS_INTERRUPT_EI(function) \
    function(void); \
    static void __near _##function(void); \
    static void __near function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_USING_FREERTOS_API(_##function); \
    } \
    static void __near _##function

#define R_CG_INTERRUPT_EI(function) \
    function(void); \
    static void __near _##function(void); \
    static void __near function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_WITHOUT_USING_FREERTOS_API(_##function); \
    } \
    static void __near _##function

#elif defined(__llvm__)

#define R_CG_FREERTOS_INTERRUPT_EI(function) \
    function(void) __attribute__((section(".lowtext"), interrupt)); \
    static void _##function(void) __attribute__((section(".lowtext"))); \
    void function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_USING_FREERTOS_API(_##function); \
    } \
    static void _##function

#define R_CG_INTERRUPT_EI(function) \
    function(void) __attribute__((section(".lowtext"), interrupt)); \
    static void _##function(void) __attribute__((section(".lowtext"))); \
    void function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_WITHOUT_USING_FREERTOS_API(_##function); \
    } \
    static void _##function

#elif defined(__GNUC__)

#if 1 /* Type of function attribute: "#if 1" is naked, "#if 0" is interrupt. */

#define R_CG_FREERTOS_INTERRUPT_EI(function) \
    function(void) __attribute__((section(".lowtext"), naked)); \
    static void _##function(void) __attribute__((section(".lowtext"))); \
    void function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_USING_FREERTOS_API(_##function); \
    } \
    static void _##function

#define R_CG_INTERRUPT_EI(function) \
    function(void) __attribute__((section(".lowtext"), naked)); \
    static void _##function(void) __attribute__((section(".lowtext"))); \
    void function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_WITHOUT_USING_FREERTOS_API(_##function); \
    } \
    static void _##function

#else

/* The function attribute 'naked' is available for our purpose
 * but it causes the following compiler warning. (confusingly only one warning per file.)
 *
 * warning: stack usage computation not supported for this target
 *
 * So the function attribute 'interrupt' is used instead.
 */

#define R_CG_FREERTOS_INTERRUPT_EI(function) \
    function(void) __attribute__((section(".lowtext"), interrupt)); \
    static void _##function(void) __attribute__((section(".lowtext"))); \
    void function(void) \
    { \
        portASM("  MOV A, CS  "); \
        portASM("  MOV [SP], A  "); \
        portASM("  MOV CS, #0  "); \
        portASM("  MOVW BC, %0  " portCDT_NO_PARSE( :: ) "g" ( _##function )  ); \
        /* vPortCall_ISR_C_Handler_Using_FreeRTOS_API() enables nested interrupts. */ \
        portASM("  BR !_vPortCall_ISR_C_Handler_Using_FreeRTOS_API  "); \
        /* The following code is intended to remove the epilog code. */ \
        while (1) {} \
    } \
    static void _##function

#define R_CG_INTERRUPT_EI(function) \
    function(void) __attribute__((section(".lowtext"), interrupt)); \
    static void _##function(void) __attribute__((section(".lowtext"))); \
    void function(void) \
    { \
        portASM("  MOV A, CS  "); \
        portASM("  MOV [SP], A  "); \
        portASM("  MOV CS, #0  "); \
        portASM("  MOVW BC, %0  " portCDT_NO_PARSE( :: ) "g" ( _##function )  ); \
        /* vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API() enables nested interrupts. */ \
        portASM("  BR !_vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API  "); \
        /* The following code is intended to remove the epilog code. */ \
        while (1) {} \
    } \
    static void _##function

#endif

#elif defined(__ICCRL78__)

#define R_CG_FREERTOS_INTERRUPT_EI(function) \
    function(void); \
    static __near_func void _##function(void); \
    R_CG_PRAGMA(required = _##function) \
    static __interrupt void function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_USING_FREERTOS_API(_##function); \
    } \
    static __near_func void _##function

#define R_CG_INTERRUPT_EI(function) \
    function(void); \
    static __near_func void _##function(void); \
    R_CG_PRAGMA(required = _##function) \
    static __interrupt void function(void) \
    { \
        /* This enables nested interrupts and doesn't return here. */ \
        portCALL_ISR_C_HANDLER_WITHOUT_USING_FREERTOS_API(_##function); \
    } \
    static __near_func void _##function

#endif

extern void vTaskNotifyGiveFromISR_R_Helper(TaskHandle_t *pxTask);
extern void xTaskNotifyFromISR_R_Helper(TaskHandle_t *pxTask, uint32_t ulValue);
extern uint32_t ulTaskNotifyTake_R_Helper(TickType_t xTicksToWait);
extern uint32_t ulTaskNotifyTake_R_Helper_Ex2__helper(TaskHandle_t *pxTask, MD_STATUS xStatus, TickType_t xTicksToWait);
extern void ulTaskNotifyTake_R_Abort_Helper(TaskHandle_t *pxTask);
extern TaskHandle_t xTaskGetCurrentTaskHandle_R_Helper(void);

#define xTaskCreateStatic_R_Helper(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask) \
do{ \
    static StaticTask_t pxTaskCode##_xTCBBuffer; \
    static StackType_t  pxTaskCode##_xStackBuffer[usStackDepth]; \
    TaskHandle_t xCreatedTask; \
    xCreatedTask = xTaskCreateStatic( pxTaskCode, (pcName), (usStackDepth), (pvParameters), (uxPriority), pxTaskCode##_xStackBuffer, &pxTaskCode##_xTCBBuffer ); \
    if (0U != (uint8_t *)(TaskHandle_t *)(pxCreatedTask) - (uint8_t *)NULL) \
    { \
        *(TaskHandle_t *)(pxCreatedTask) = xCreatedTask; \
    } \
}while (0)

#define xSemaphoreCreateMutexStatic_R_Helper(pxCreatedSemaphore) \
do{ \
    static StaticSemaphore_t xStaticSemaphore; \
    QueueHandle_t xCreatedQueue; \
    xCreatedQueue = xSemaphoreCreateMutexStatic( &xStaticSemaphore ); \
    if (0U != (uint8_t *)(QueueHandle_t *)(pxCreatedSemaphore) - (uint8_t *)NULL) \
    { \
        *(QueueHandle_t *)(pxCreatedSemaphore) = xCreatedQueue; \
    } \
}while (0)

#define ulTaskNotifyTake_R_Helper_Ex(pxTask, vStartFunc, xTicksToWait) \
( \
    /* Setup the interrupt/callback ready to post a notification */ \
    (*(pxTask) = xTaskGetCurrentTaskHandle_R_Helper()), \
    /* Wait for a notification from the interrupt/callback */ \
    ulTaskNotifyTake_R_Helper( \
        ( \
            (vStartFunc), \
            (xTicksToWait) \
        ) \
    ) \
)

#define ulTaskNotifyTake_R_Helper_Ex2(pxTask, xStartFunc, xTicksToWait) \
( \
    /* Setup the interrupt/callback ready to post a notification */ \
    (*(pxTask) = xTaskGetCurrentTaskHandle_R_Helper()), \
    /* Wait for a notification from the interrupt/callback */ \
    ulTaskNotifyTake_R_Helper_Ex2__helper( \
        (pxTask), \
        (xStartFunc), \
        (xTicksToWait) \
    ) \
)

#define ulTaskNotifyTake_R_Abort_Helper_Ex(pxTask, vAbortFunc) \
do{ \
    /* Abort the interrupt/callback posting a notification */ \
    (vAbortFunc); \
    ulTaskNotifyTake_R_Abort_Helper(pxTask); \
}while (0)

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_HELPER_H */
