// Include Particle Device OS APIs
#include "Particle.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
#ifndef SYSTEM_VERSION_v620
SYSTEM_THREAD(ENABLED); // System thread defaults to on in 6.2.0 and later and this line is not required
#endif

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

FuelGauge fuel;

// Battery Service + Battery Level characteristic UUIDs
const BleUuid batteryService(0x180F);

BleCharacteristic batteryLevelChar(
    "bat", // short description
    BleCharacteristicProperty::READ | BleCharacteristicProperty::NOTIFY,
    BleUuid(0x2A19), // Battery Level characteristic
    batteryService   // Service UUID this characteristic belongs to
);

// setup() runs once, when the device is first turned on
void setup()
{
  BLE.addCharacteristic(batteryLevelChar);
  BLE.setDeviceName("Particle");
  BleAdvertisingData adv;
  adv.appendServiceUUID(batteryService);
  BLE.advertise(&adv);

  SystemPowerConfiguration cfg = System.getPowerConfiguration();
  cfg.feature(SystemPowerFeature::PMIC_DETECTION) // external PMIC + fuel gauge detection
      .auxiliaryPowerControlPin(D7)               // Muon uses D7 for 3V3_AUX control
      .interruptPin(A7);                          // Muon uses A7 for PMIC/fuel gauge interrupt
  System.setPowerConfiguration(cfg);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  float v = fuel.getVCell();            // volts (returns -1.0 if unreadable)
  float soc = fuel.getSoC();            // 0–100% (returns -1.0 if unreadable)
  float nsoc = fuel.getNormalizedSoC(); // normalized 0–100%
  uint8_t pct = (uint8_t)nsoc;          // battery percent for characteristic

  batteryLevelChar.setValue(&pct, 1);
  Log.info("V=%.2fV SoC=%.1f%% nSoC=%.1f%%", v, soc, nsoc);

  delay(15000);
}
