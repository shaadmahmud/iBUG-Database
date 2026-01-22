/*******************************************************************
  Standalone SD Analog Logger for RAK11300 (RP2040)
  - Logs time(ms since boot), A0, A1, A2 to a new CSV each boot
  - No Serial required
*******************************************************************/

#if !defined(ARDUINO_ARCH_RP2040)
  #error For RP2040 only
#endif

#if defined(ARDUINO_ARCH_MBED)
  #define PIN_SD_SS   PIN_SPI_SS
#else
  #define PIN_SD_SS   PIN_SPI0_SS
#endif

#include <SPI.h>
#include <RP2040_SD.h>

static const uint32_t SAMPLE_MS = 1000;     // sample every 1s
static const uint32_t FLUSH_EVERY = 10;     // flush every N samples

File logFile;
char logName[16] = {0};

uint32_t lastSample = 0;
uint32_t sampleCount = 0;

// Find an unused filename like LOG0001.CSV
bool makeNewLogFilename(char *out, size_t outLen) {
  for (int i = 1; i <= 9999; i++) {
    snprintf(out, outLen, "LOG%04d.CSV", i);
    if (!SD.exists(out)) return true;
  }
  return false;
}

void setup()
{
  // IMPORTANT: no Serial, no waiting for USB

  // Initialize SD
  if (!SD.begin(PIN_SD_SS)) {
    // If SD fails, do nothing (or you could blink an LED if you want)
    while (1) { delay(1000); }
  }

  // Create a new log file each boot
  if (!makeNewLogFilename(logName, sizeof(logName))) {
    while (1) { delay(1000); }
  }

  logFile = SD.open(logName, FILE_WRITE);
  if (!logFile) {
    while (1) { delay(1000); }
  }

  // CSV header
  logFile.println("time_ms,A0,A1,A2");
  logFile.flush();

  // Optional: set ADC resolution if your core supports it
  // analogReadResolution(12); // RP2040 typically 12-bit (0..4095)
  // analogReadResolution(10); // if you want Arduino-like 0..1023
}

void loop()
{
  uint32_t now = millis();
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    int a0 = analogRead(A0);
    int a1 = analogRead(A1);
    int a2 = analogRead(A2);

    // Write one CSV line: time_ms,A0,A1,A2
    logFile.print(now);
    logFile.print(',');
    logFile.print(a0);
    logFile.print(',');
    logFile.print(a1);
    logFile.print(',');
    logFile.println(a2);

    sampleCount++;

    // Flush periodically for safety (tradeoff: more SD writes)
    if (sampleCount % FLUSH_EVERY == 0) {
      logFile.flush();
    }
  }
}
