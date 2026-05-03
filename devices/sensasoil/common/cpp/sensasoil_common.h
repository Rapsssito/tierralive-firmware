#pragma once

namespace sensasoil {

/**
 * @brief Device setup states for BLE provisioning flow
 */
enum class BleSetupState : uint8_t {
  INIT,        ///< Initial/idle state
  WIFI_CONFIG, ///< Configuring WiFi
  MQTT_CONFIG  ///< Configuring MQTT
};

/**
 * @brief Extended Improv RPC command IDs
 *
 * These commands extend the standard Improv protocol with custom functionality
 * for TierraLive devices
 */
enum class ExtendedImprovCommand : uint8_t {
  MQTT_CHANGE_SETTINGS = 1,     ///< Change MQTT broker settings
  FACTORY_RESET = 2,            ///< Reset device to factory defaults
  MQTT_VIEW_SETTINGS = 3,       ///< View current MQTT settings
  WIFI_VIEW_SETTINGS = 4,       ///< View current WiFi settings
  ENABLE_DIAGNOSTIC = 5,        ///< Enable diagnostic mode
  MQTT_DISCOVERY_PREFIX = 6,    ///< Set MQTT discovery prefix
  REFRESH_SENSORS = 7,          ///< Refresh sensor readings
  CALIBRATE_SENSORS = 8,        ///< Calibrate soil moisture sensor
  CALIBRATION_VIEW_SETTINGS = 9 ///< View current calibration settings
};

/**
 * @brief Error codes for device operation states
 */
enum class ErrorCode : uint8_t {
  NONE,             ///< No error
  WIFI,             ///< WiFi connection error
  MQTT,             ///< MQTT connection error
  MQTT_ACK_TIMEOUT, ///< MQTT acknowledgment timeout
  OTA               ///< Over-The-Air update error
};

/**
 * @brief Soil capacitor sensor versions
 */
enum class SoilProbeVersion : uint8_t { V3, V4, V5 };

/**
 * @brief Convert raw bytes to std::string with explicit length
 * 
 * This helper avoids repeated lambda definitions and provides portable
 * byte-to-string conversion without relying on std::string SSO behavior.
 * Safe for binary payloads including embedded null bytes.
 * 
 * @param bytes Pointer to byte buffer (cast as const uint8_t*)
 * @param length Number of bytes to include
 * @return std::string constructed from the byte range
 */
inline std::string make_string(const uint8_t *bytes, size_t length) {
  return std::string(reinterpret_cast<const char *>(bytes), length);
}

/**
 * @brief Compute soil volumetric water content (VWC) based on sensor readings
 * @param voltage Current raw voltage reading from the soil sensor
 * @param temperature Current temperature reading
 * @param CALIBRATED_TEMP The temperature at which the sensor was calibrated
 * @param CALIBRATED_MIN_V The calibrated voltage at 100% VWC
 * @param CALIBRATED_MAX_V The calibrated voltage at 0% VWC
 * @param version The soil sensor version
 * @return The computed VWC as a percentage
 */
const float compute_soil_vwc(const float voltage, const float temperature,
                             const float CALIBRATED_TEMP,
                             const float CALIBRATED_MIN_V,
                             const float CALIBRATED_MAX_V,
                             const SoilProbeVersion version) {
  if (std::isnan(voltage) || voltage < 0) {
    return NAN; // Invalid voltage reading
  }
  float result = 0;
  switch (version) {
  case SoilProbeVersion::V3: {
    // Empirically determined exponential relationship for V3
    result = 778 * exp(-2.51 * voltage);
    break;
  }
  case SoilProbeVersion::V4:
  case SoilProbeVersion::V5: {
    // Compensate the voltage based on temperature
    const float TEMP_COEFFICIENT = -0.0025f;
    float x_true =
        !std::isnan(temperature)
            ? voltage + TEMP_COEFFICIENT * (temperature - CALIBRATED_TEMP)
            : voltage;
    // Normalize the voltage between 0 and 1
    float x_norm =
        (x_true - CALIBRATED_MIN_V) / (CALIBRATED_MAX_V - CALIBRATED_MIN_V);
    // Apply the exponential model
    result = 1.0974 * exp(-2.4215 * x_norm) - 0.0974;
    break;
  }
  }
  // Ensure the final result is between 0% and 100%
  result = MAX(0, MIN(1, result));
  return result * 100; // Convert to percentage
}

const float compute_soil_temperature(const float voltage,
                                     const SoilProbeVersion version) {
  const float VCC = 3.3;       // Supply voltage for the sensor
  const float T_0 = 298.15;    // 25°C in Kelvin
  const float K_TO_C = 273.15; // Conversion from Kelvin to Celsius
  // Guard against division by zero
  if (std::isnan(voltage) || voltage <= 0 || voltage >= VCC) {
    return NAN;
  }
  switch (version) {
  case SoilProbeVersion::V3:
  case SoilProbeVersion::V4: {
    const float R_S = 47000;  // Resistor in series connected to GND
    const float R_25 = 47000; // NTC thermistor resistance at 25°C
    const float B_VALUE = 4050;
    // NTC to VCC
    float r_ntc = R_S * (VCC - voltage) / voltage;
    float temp_k = 1.0 / ((1.0 / T_0) + (1.0 / B_VALUE) * log(r_ntc / R_25));
    return temp_k - K_TO_C;
  }
  case SoilProbeVersion::V5: {
    const float R_S = 100000;  // Resistor in series connected to 3.3V
    const float R_25 = 100000; // NTC thermistor resistance at 25°C
    const float B_VALUE = 4250;
    // NTC to ground
    float r_ntc = R_S * voltage / (VCC - voltage);
    float temp_k = 1.0 / ((1.0 / T_0) + (1.0 / B_VALUE) * log(r_ntc / R_25));
    return temp_k - K_TO_C;
  }
  }
  return NAN;
}

} // namespace sensasoil
