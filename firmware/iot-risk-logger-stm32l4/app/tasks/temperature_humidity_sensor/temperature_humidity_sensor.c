/*!
 * @file temperature_humidity_sensor.c
 * @brief implementation of temperature_humidity_sensor
 *
 * Detailed description of the implementation file.
 *
 * @date 17/07/2024
 * @author artempolisskyi
 */

#include "temperature_humidity_sensor.h"

static osStatus_t handleTHSensorFSM(TH_SENS_Actor_t *this, message_t *message);
/** states handlers */
static osStatus_t handleInit(TH_SENS_Actor_t *this, message_t *message);
static osStatus_t handleIdle(TH_SENS_Actor_t *this, message_t *message);
static osStatus_t handleContinuousMeasure(TH_SENS_Actor_t *this, message_t *message);
static osStatus_t handleError(TH_SENS_Actor_t *this, message_t *message);
/** utils */
static uint32_t delayMs(uint32_t ms);

extern actor_t* ACTORS_LIST_SystemRegistry[MAX_ACTORS];

TH_SENS_Actor_t TH_SENS_Actor = {
        .super = {
                .actorId = TEMPERATURE_HUMIDITY_SENSOR_ACTOR_ID,
                .messageHandler = (messageHandler_t) handleTHSensorFSM,
                .osMessageQueueId = NULL,
                .osThreadId = NULL,
        },
        .state = TH_SENS_NO_STATE,
        .rawTemperature = 0x0000,
        .rawHumidity = 0x0000,
};

uint32_t thSensorTaskBuffer[DEFAULT_TASK_STACK_SIZE_WORDS];
StaticTask_t thSensorTaskControlBlock;
const osThreadAttr_t thSensorTaskDescription = {
        .name = "thSensorTask",
        .cb_mem = &thSensorTaskControlBlock,
        .cb_size = sizeof(thSensorTaskControlBlock),
        .stack_mem = &thSensorTaskBuffer[0],
        .stack_size = sizeof(thSensorTaskBuffer),
        .priority = (osPriority_t) osPriorityNormal,
};

actor_t* TH_SENS_TaskInit(void) {
  TH_SENS_Actor.super.osMessageQueueId = osMessageQueueNew(DEFAULT_QUEUE_SIZE, DEFAULT_QUEUE_MESSAGE_SIZE, &(osMessageQueueAttr_t){
          .name = "thSensorQueue"
  });
  TH_SENS_Actor.super.osThreadId = osThreadNew(TH_SENS_Task, NULL, &thSensorTaskDescription);

  return (actor_t*) &TH_SENS_Actor;
 }

void TH_SENS_Task(void *argument) {
  (void) argument; // Avoid unused parameter warning
  message_t msg;

  fprintf(stdout, "Task %s started\n", thSensorTaskDescription.name);

  for (;;) {
    // Wait for messages from the queue
    if (osMessageQueueGet(TH_SENS_Actor.super.osMessageQueueId, &msg, NULL, osWaitForever) == osOK) {
      osStatus_t handleMessageStatus = TH_SENS_Actor.super.messageHandler((actor_t *) &TH_SENS_Actor, &msg);

      if (handleMessageStatus != osOK) {
        fprintf(stderr, "%s: Error handling event %u in state %ul\n", thSensorTaskDescription.name, msg.event, TH_SENS_Actor.state);
        osMessageQueueId_t evManagerQueue = ACTORS_LIST_SystemRegistry[EV_MANAGER_ACTOR_ID]->osMessageQueueId;
        osMessageQueuePut(evManagerQueue, &(message_t){GLOBAL_ERROR, .payload.value = TEMPERATURE_HUMIDITY_SENSOR_ACTOR_ID}, 0, 0);
        TO_STATE(&TH_SENS_Actor, TH_SENS_STATE_ERROR);
      }
    }
  }
}

static osStatus_t handleTHSensorFSM(TH_SENS_Actor_t *this, message_t *message) {
  switch (this->state) {
    case TH_SENS_NO_STATE:
      return handleInit(this, message);
    case TH_SENS_IDLE_STATE:
      return handleIdle(this, message);
    case TH_SENS_CONTINUOUS_MEASURE_STATE:
      return handleContinuousMeasure(this, message);
    case TH_SENS_STATE_ERROR:
      // TODO implement error handling
      return handleError(this, message);
    default:
      return osOK;
  }
}

static osStatus_t handleInit(TH_SENS_Actor_t *this, message_t *message) {
  if (GLOBAL_CMD_INITIALIZE == message->event) {
    // provide IO functions to the sensor driver
    osStatus_t ioStatus = SHT3x_InitIO(TH_SENS_I2C_ADDRESS, BSP_I2C1_Send, BSP_I2C1_Recv, delayMs, NULL);

    if (ioStatus != osOK) return osError;

    // reset the sensor by pulling down _TEMP_RESET, at least 1uS duration required
    HAL_GPIO_WritePin(_TEMP_RESET_GPIO_Port, _TEMP_RESET_Pin, GPIO_PIN_RESET);
    delayMs(1);
    HAL_GPIO_WritePin(_TEMP_RESET_GPIO_Port, _TEMP_RESET_Pin, GPIO_PIN_SET);
    delayMs(1);

    // read sensor ID
    uint32_t sht3xId = 0x00000000;
    ioStatus = SHT3x_ReadDeviceID(&sht3xId);

    if (ioStatus != osOK) return osError;
    fprintf(stdout, "SHT3x ID: %lu\n", sht3xId);

    // publish to event manager that the sensor is initialized
    osMessageQueueId_t evManagerQueue = ACTORS_LIST_SystemRegistry[EV_MANAGER_ACTOR_ID]->osMessageQueueId;
    osMessageQueuePut(evManagerQueue, &(message_t){GLOBAL_INITIALIZE_SUCCESS, .payload.value = TEMPERATURE_HUMIDITY_SENSOR_ACTOR_ID}, 0, 0);

    fprintf(stdout, "Temperature & Humidity sensor %ul initialized\n", TEMPERATURE_HUMIDITY_SENSOR_ACTOR_ID);
    TO_STATE(this, TH_SENS_IDLE_STATE);
  }

  return osOK;
}

static osStatus_t handleIdle(TH_SENS_Actor_t *this, message_t *message) {
  osStatus_t  ioStatus = osOK;

  switch (message->event) {
    case GLOBAL_CMD_START_CONTINUOUS_SENSING:
      ioStatus = SHT3x_PeriodicAcquisitionMode(SHT3x_START_MEASUREMENT_0_5_MPS_LOW_REPEATABILITY_CMD_ID);
      if (ioStatus != osOK) return osError;

      TO_STATE(this, TH_SENS_CONTINUOUS_MEASURE_STATE);
      return osOK;
    case TH_SENS_START_SINGLE_SHOT_READ:
      // TODO run single shot measurement
      fprintf(stdout, "Raw: temperature: %d humidity %d\n", this->rawTemperature, this->rawHumidity);
      TO_STATE(this, TH_SENS_IDLE_STATE);
      return osOK;
    default:
      return osOK;
  }
}

static osStatus_t handleContinuousMeasure(TH_SENS_Actor_t *this, message_t *message) {
  osStatus_t  ioStatus = osOK;

  switch (message->event) {
    case GLOBAL_WAKE_N_READ:
      ioStatus = SHT3x_ReadMeasurements(&this->rawTemperature, &this->rawHumidity);
      if (ioStatus != osOK) return osError;

      float t = SHT3x_RawToTemperatureC(this->rawTemperature);
      float rh = SHT3x_RawToHumidityRH(this->rawHumidity);
      fprintf(stdout, "Temperature: %.2f humidity %.2f\n", t, rh);

      TO_STATE(this, TH_SENS_CONTINUOUS_MEASURE_STATE);
      return osOK;
    default:
      return osOK;
  }
}

static osStatus_t handleError(TH_SENS_Actor_t *this, message_t *message) {
  return osOK;
}

static uint32_t delayMs(uint32_t ms) {
  uint32_t ticks = (ms * configTICK_RATE_HZ) / 1000;
  return osDelay(ticks);
}