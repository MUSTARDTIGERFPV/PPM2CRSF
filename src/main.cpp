#include <Arduino.h>
#include <PPMReader.h>
#include <crsf.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// General parameters

#define DEFAULT_CHANNEL_VALUE 1500
//#define DEBUG

// PPM Parameters

#define PPM_IN_PIN 4
#define PPM_CHANNEL_COUNT 8
#define PPM_DIRECTION FALLING

// CRSF Parameters

#define CRSF_OUT_PIN 15
#define CRSF_IN_PIN 2
#define CRSF_INVERT true

// PPM Handlers

void IRAM_ATTR onPPMInterruptCallback();
PPMReader reader(PPM_CHANNEL_COUNT);

// PPM variables
volatile uint8_t pulseCounter = 0; // Incremented when a new PPM pulse is ingested
volatile uint8_t lastPulse = 0;    // Set when a new PPM pulse has been handled

// Transitive variables
int channels[CRSF_MAX_CHANNEL_COUNT]; // Holds ingested PPM values so they can be sent over CRSF

// CRSF variables
uint8_t packet[CRSF_FRAME_SIZE_MAX]; // Buffer holding CRSF frame in construction
volatile uint8_t frameCounter = 0;   // Incremented when a new PPM frame is ready
volatile uint8_t lastFrame = 0;      // Set when a CRSF frame has been sent

void onPPMReady()
{
  noInterrupts();
  // Fill channels from PPM values
  for (uint8_t i = 0; i < PPM_CHANNEL_COUNT; i++)
  {
    channels[i] = reader.latestValidChannelValue(i + 1, 1500);
  }
  // Fill in the remaining channels
  for (uint8_t i = PPM_CHANNEL_COUNT; i < CRSF_MAX_CHANNEL_COUNT; i++)
  {
    channels[i] = DEFAULT_CHANNEL_VALUE;
  }
  frameCounter++;
  interrupts();
}

// Called when a PPM pulse has been ingested.
void IRAM_ATTR onPPMInterruptCallback()
{
  // Set the current time into the PPMReader as soon as the pulse fires so that nothing delays our PPM read accuracy
  reader.interruptMicros = micros();
  // Ingest the pulse is ready so we ingest it
  reader.handleInterrupt();
}

void crsfTask(void *param)
{
#ifdef DEBUG
  Serial.println("Starting CRSF task");
#endif
  for (;;)
  {
    // Handle a PPM frame ready, converting to CRSF
    if (lastFrame != frameCounter)
    {
      crsfPrepareChannelsPacket(packet, channels);
      lastFrame = frameCounter;
#ifdef DEBUG
      for (uint8_t i = 0; i < CRSF_TYPE_CHANNELS_PAYLOAD_LENGTH + CRSF_OVERHEAD_LENGTH; i++)
      {
        Serial.print(packet[i], HEX);
      }
      Serial.printf(" %d\n", frameCounter);
      Serial.printf("%d %d %d %d %d\n", channels[0], channels[1], channels[2], channels[3], channels[4]);
#endif
      Serial1.write(packet, CRSF_TYPE_CHANNELS_PAYLOAD_LENGTH + CRSF_OVERHEAD_LENGTH);
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // TODO: can we reduce?
  }
}

void setup()
{
#ifdef DEBUG
  Serial.begin(921600);
#endif
  Serial1.begin(CRSF_SERIAL_BAUDRATE, SERIAL_8N1, CRSF_IN_PIN, CRSF_OUT_PIN, CRSF_INVERT);

  pinMode(PPM_IN_PIN, INPUT);
  attachInterrupt(PPM_IN_PIN, onPPMInterruptCallback, PPM_DIRECTION);

  // Called when a full PPM frame is ready with all channels
  reader.onFrameComplete = onPPMReady;
#ifdef DEBUG
  Serial.println("Starting PPM2CRSF");
#endif
  xTaskCreatePinnedToCore(crsfTask, "crsf", 4096, NULL, 5, NULL, 1);
}

void loop()
{
}
