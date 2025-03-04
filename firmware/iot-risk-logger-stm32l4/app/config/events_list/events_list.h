/*!
 * @file events_list.h
 * @brief Brief description of the file.
 *
 * Detailed description of the file.
 *
 * @date 17/07/2024
 * @author artempolisskyi
 */

#ifndef EVENTS_LIST_H
#define EVENTS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/**
 * @brief Event type
 * Contains all possible events for the application
 */
typedef enum {
  EVENT_NONE = 0,
  /**
   * @brief Global Commands, their codes equal to the command codes in the protocol across the system
   * @note Mobile phone application should send these commands to the device
   */
  // GLOBAL COMMANDS
  GLOBAL_CMD_START_LOGGING    = 0xC0, ///< Start logging measurements
  GLOBAL_CMD_STOP_LOGGING     = 0xC1, ///< Stop logging measurements
  GLOBAL_CMD_WRITE_SETTINGS   = 0xC2, ///< Write settings to the device
  GLOBAL_CMD_READ_SETTINGS    = 0xC3, ///< Read settings from the device
  GLOBAL_CMD_READ_LOG_CHUNK   = 0xC4, ///< Read log chunk from the device
  GLOBAL_CMD_MAX,
  /**
   * @brief Global Events in the system
   */
  // GLOBAL Events
  GLOBAL_CMD_INITIALIZE,
  GLOBAL_INITIALIZE_SUCCESS,
  GLOBAL_WAKE_N_READ, ///> RTC wakes up event, mostly leads to the sensor measurements read
  GLOBAL_TEMPERATURE_HUMIDITY_MEASUREMENTS_READY, ///< Temperature and humidity measurements are ready
  GLOBAL_LIGHT_MEASUREMENTS_READY, ///< Light measurements are ready
  GLOBAL_MEASUREMENTS_WRITE_SUCCESS, ///< Sensors measurements are successfully written to the NOR memory
  GLOBAL_LOG_CHUNK_READ_SUCCESS, ///< Log chunk is successfully read from the NOR memory
  GLOBAL_SETTINGS_WRITE_SUCCESS, ///< Settings are successfully written to the NOR memory
  GLOBAL_SETTINGS_READ_SUCCESS, ///< Settings are successfully read from the NOR memory
  GLOBAL_CMD_INFO_LED_ON,
  GLOBAL_CMD_INFO_LED_OFF,
  GLOBAL_CMD_SET_TIME_DATE, ///< Set time and date from int32 UNIX timestamp
  GLOBAL_CMD_SET_WAKE_UP_PERIOD, ///< Set wake up period in seconds
  GLOBAL_CMD_START_CONTINUOUS_SENSING, ///< Start continuous sensors measurement
  GLOBAL_CMD_TURN_OFF, ///< Turn off to power saving mode
  GLOBAL_CMD_NFC_MAILBOX_WRITE, ///< NFC mailbox write data event
  GLOBAL_ERROR,
  GLOBAL_EVENTS_MAX,
  /**
   * @brief Local Events for the system modules
   */
  // INFO_LED
  INFO_LED_FLASH,
  // NFC
  NFC_GPO_INTERRUPT,
  NEW_MAILBOX_RF_CMD,
  NFC_CRC_ERROR,
  // ACCELEROMETER_SENSOR
  // TODO: Add accelerometer events
  // TEMPERATURE_HUMIDITY_SENSOR
  TH_SENS_START_SINGLE_SHOT_READ,
  TH_SENS_TURN_OFF,
  TH_SENS_ERROR,
  // LIGHT_SENSOR
  LIGHT_SENS_SINGLE_SHOT_READ,
  LIGHT_SENS_MEASURE_CONTINUOUSLY,
  LIGHT_SENS_SET_LIMIT,
  LIGHT_SENS_TURN_OFF,
  LIGHT_SENS_LIMIT_INT,
  LIGHT_SENS_RECOVER,
  LIGHT_SENS_ERROR,
  // MEMORY
  MEMORY_MEASUREMENTS_WRITE,
  // USB
  USB_CONNECTED,
  USB_DISCONNECTED,
  MAX_EVENTS
} event_t;

#ifdef __cplusplus
}
#endif

#endif //EVENTS_LIST_H