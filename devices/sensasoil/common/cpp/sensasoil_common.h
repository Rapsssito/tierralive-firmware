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
  FACTORY_RESET = 1,      ///< Reset device to factory defaults
  DIAGNOSTIC = 2,         ///< Read or write diagnostic settings
  REFRESH_SENSORS = 3,    ///< Trigger refresh of all sensor readings
  CALIBRATION = 4,        ///< Read or write calibration settings
  WIFI_VIEW_SETTINGS = 5, ///< Read current WiFi settings
  MQTT_SETTINGS = 6,      ///< Read or write MQTT settings
  CELLULAR_SETTINGS = 7,  ///< Read or write cellular settings
};

/**
 * @brief Error codes for device operation states
 */
enum class ErrorCode : uint8_t {
  NONE,                ///< No error
  WIFI,                ///< WiFi connection error
  MQTT,                ///< MQTT connection error
  MQTT_ACK_TIMEOUT,    ///< MQTT acknowledgment timeout
  MQTT_PUBLISH_FAILED, ///< MQTT publish failure
  OTA                  ///< Over-The-Air update error
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
 * @brief Convert float to a rounded decimal string with fixed precision.
 *
 * NaN and Infinity are preserved as textual values.
 */
inline std::string rounded_float_to_string(const float value,
                                           const uint8_t precision) {
  char buf[VALUE_ACCURACY_MAX_LEN];
  size_t len = value_accuracy_to_buf(buf, value, precision);
  return std::string(buf, len);
}

static constexpr size_t MAC_RAW_LENGTH = 6;
static constexpr size_t MAC_BASE64URL_LENGTH = 8;
static constexpr size_t MAC_BASE64URL_BUFFER_SIZE = MAC_BASE64URL_LENGTH + 1;
static constexpr size_t MAC_BASE32_LENGTH = 10;
static constexpr size_t MAC_BASE32_BUFFER_SIZE = MAC_BASE32_LENGTH + 1;
static constexpr size_t MAC_UNIQUE_ID_MAX_ENTITY_ID_LENGTH = 6;
static constexpr size_t MAC_UNIQUE_ID_BUFFER_SIZE =
    MAC_BASE32_BUFFER_SIZE + 1 + MAC_UNIQUE_ID_MAX_ENTITY_ID_LENGTH + 1;

/**
 * @brief Encode device MAC into a null-terminated Base32 string (case-insensitive).
 *
 * Encodes the 6-byte MAC address into 10 Base32 characters (RFC 4648) using only
 * stack buffers (no heap allocation). Base32 is case-insensitive, using uppercase
 * letters A-Z and digits 2-7.
 */
inline void get_mac_base32(char (&out)[MAC_BASE32_BUFFER_SIZE]) {
  static constexpr char BASE32_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

  uint8_t mac[MAC_RAW_LENGTH];
  get_mac_address_raw(mac);

  size_t out_idx = 0;
  // Process 5 bytes at a time (40 bits = 8 Base32 characters)
  // First 5 bytes
  out[out_idx++] = BASE32_TABLE[(mac[0] >> 3) & 0x1F];
  out[out_idx++] =
      BASE32_TABLE[(((mac[0] & 0x07) << 2) | (mac[1] >> 6)) & 0x1F];
  out[out_idx++] = BASE32_TABLE[(mac[1] >> 1) & 0x1F];
  out[out_idx++] =
      BASE32_TABLE[(((mac[1] & 0x01) << 4) | (mac[2] >> 4)) & 0x1F];
  out[out_idx++] =
      BASE32_TABLE[(((mac[2] & 0x0F) << 1) | (mac[3] >> 7)) & 0x1F];
  out[out_idx++] = BASE32_TABLE[(mac[3] >> 2) & 0x1F];
  out[out_idx++] =
      BASE32_TABLE[(((mac[3] & 0x03) << 3) | (mac[4] >> 5)) & 0x1F];
  out[out_idx++] = BASE32_TABLE[mac[4] & 0x1F];

  // Last byte (6th byte)
  out[out_idx++] = BASE32_TABLE[(mac[5] >> 3) & 0x1F];
  out[out_idx++] = BASE32_TABLE[((mac[5] & 0x07) << 2) & 0x1F];

  out[out_idx] = '\0';
}

/**
 * @brief Encode device MAC into a null-terminated Base64URL string.
 *
 * Encodes the 6-byte MAC address into 8 Base64URL characters using only
 * stack buffers (no heap allocation).
 */
inline void get_mac_base64url(char (&out)[MAC_BASE64URL_BUFFER_SIZE]) {
  static constexpr char BASE64URL_TABLE[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

  uint8_t mac[MAC_RAW_LENGTH];
  get_mac_address_raw(mac);

  size_t out_idx = 0;
  for (size_t i = 0; i < MAC_RAW_LENGTH; i += 3) {
    const uint32_t block = (static_cast<uint32_t>(mac[i]) << 16) |
                           (static_cast<uint32_t>(mac[i + 1]) << 8) |
                           static_cast<uint32_t>(mac[i + 2]);
    out[out_idx++] = BASE64URL_TABLE[(block >> 18) & 0x3F];
    out[out_idx++] = BASE64URL_TABLE[(block >> 12) & 0x3F];
    out[out_idx++] = BASE64URL_TABLE[(block >> 6) & 0x3F];
    out[out_idx++] = BASE64URL_TABLE[block & 0x3F];
  }
  out[out_idx] = '\0';
}


/**
 * @brief Build MQTT unique_id in the format "<mac_hex>-<entity_id>".
 */
inline void build_mac_unique_id(const char *mac_hex, const char *entity_id,
                                char (&out)[MAC_UNIQUE_ID_BUFFER_SIZE]) {
  snprintf(out, MAC_UNIQUE_ID_BUFFER_SIZE, "%s-%s", mac_hex, entity_id);
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
