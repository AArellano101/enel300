// Add to the very bottom of the file of the same name

/* USER CODE BEGIN 1 */
void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
/* USER CODE END 1 */