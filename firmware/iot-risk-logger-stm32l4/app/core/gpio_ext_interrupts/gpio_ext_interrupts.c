/*!
 * @file gpio_ext_interrupts.c
 * @brief implementation of gpio_ext_interrupts
 *
 * Detailed description of the implementation file.
 *
 * @date 07/10/2024
 * @author artempolisskyi
 */

#include "gpio_ext_interrupts.h"

extern actor_t* ACTORS_LOOKUP_SystemRegistry[MAX_ACTORS];

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == USB_VBUS_SENSE_Pin) {
    GPIO_PinState usbVBusPin = HAL_GPIO_ReadPin(USB_VBUS_SENSE_GPIO_Port, USB_VBUS_SENSE_Pin);
    osMessageQueueId_t evManagerQueue = ACTORS_LOOKUP_SystemRegistry[EV_MANAGER_ACTOR_ID]->osMessageQueueId;

    if (usbVBusPin == GPIO_PIN_SET) {
      osMessageQueuePut(evManagerQueue, &(message_t) {USB_CONNECTED}, 0, 0);
#if DEBUG
      fprintf(stdout, "USB connected\n");
#endif
    } else {
      osMessageQueuePut(evManagerQueue, &(message_t) {USB_DISCONNECTED}, 0, 0);
#if DEBUG
      fprintf(stdout, "USB disconnected\n");
#endif
    }
  }

  // TODO maybe check only falling edge (configure in CubeMX)
  if (GPIO_Pin == _NFC_INT_Pin) {
#ifdef DEBUG
    fprintf(stdout, "NFC GPO Interrupt\n");
#endif
    osMessageQueuePut(NFC_Actor.super.osMessageQueueId, &(message_t){NFC_GPO_INTERRUPT}, 0, 0);
  }
}