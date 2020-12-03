#include "grbl.h"

/** @defgroup GPIO_Exported_Types
  * @{
  */
#define IS_GPIO_ALL_PERIPH(PERIPH) (((PERIPH) == GPIOA) || \
                                    ((PERIPH) == GPIOB) || \
                                    ((PERIPH) == GPIOC) || \
                                    ((PERIPH) == GPIOD) || \
                                    ((PERIPH) == GPIOE) || \
                                    ((PERIPH) == GPIOF) || \
                                    ((PERIPH) == GPIOG))


uint16_t HAL_GPIO_Read(GPIO_TypeDef* GPIOx);
void HAL_GPIO_Write(GPIO_TypeDef* GPIOx, uint16_t PortVal);