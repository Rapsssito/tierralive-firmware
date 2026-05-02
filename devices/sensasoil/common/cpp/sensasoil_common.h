#pragma once

namespace sensasoil {

/**
 * @brief Device setup states for BLE provisioning flow
 */
enum class BleSetupState : uint8_t {
  INIT = 0,         ///< Initial/idle state
  WIFI_CONFIG = 1,  ///< Configuring WiFi
  MQTT_CONFIG = 2   ///< Configuring MQTT
};

/**
 * @brief Extended Improv RPC command IDs
 * 
 * These commands extend the standard Improv protocol with custom functionality
 * for TierraLive devices
 */
enum class ExtendedImprovCommandId : uint8_t {
  MQTT_CHANGE_SETTINGS = 1,      ///< Change MQTT broker settings
  FACTORY_RESET = 2,             ///< Reset device to factory defaults
  MQTT_VIEW_SETTINGS = 3,        ///< View current MQTT settings
  WIFI_VIEW_SETTINGS = 4,        ///< View current WiFi settings
  ENABLE_DIAGNOSTIC = 5,         ///< Enable diagnostic mode
  MQTT_DISCOVERY_PREFIX = 6,     ///< Set MQTT discovery prefix
  REFRESH_SENSORS = 7            ///< Refresh sensor readings
};

/**
 * @brief Error codes for device operation states
 */
enum class ErrorCode : uint8_t {
  NONE = 0,                ///< No error
  WIFI = 1,                ///< WiFi connection error
  MQTT = 2,                ///< MQTT connection error
  MQTT_ACK_TIMEOUT = 3,    ///< MQTT acknowledgment timeout
  OTA = 4                  ///< Over-The-Air update error
};

}  // namespace sensasoil
