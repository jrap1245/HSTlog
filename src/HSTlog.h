#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <stdarg.h>
#include <RTClib.h>
#include "Version.h"

enum LogOutput {
  LOG_NONE    = 0,
  LOG_SERIAL  = 1 << 0,
  LOG_MQTT    = 1 << 1,
  LOG_DISPLAY = 1 << 2,
};

/* LogLevel defines a set of
 * logging/notification levels (DEBUG = 1, INFO = 2, STATUS = 3, WARNING = 4, ERROR = 5, BOOT = 6)
 * plus an OLED value (7) intended for sending short messages to an OLED display.
 */
enum class LogLevel {
  DEBUG       = 1,
  INFO        = 2,
  STATUS      = 3,
  WARNING     = 4,
  ERROR       = 5,
  BOOT        = 6,
  OLED        = 7       // if you want to show short messages on your OLED
};

/* LogEntry is a Plain‑Old‑Data struct holding a single log record
 *  - LogLevel indicating severity
 *  - String for the message
 *  - uint32_t timestamp.
 */
struct LogEntry {
  LogLevel level;
  String message;
  uint32_t timestamp;
};

typedef void (*LogCallback)(const char* msg);

class HSTlog {
public:
  LogLevel mqttLogLevel = LogLevel::DEBUG;
  LogLevel displayLogLevel = LogLevel::DEBUG;
  uint8_t logOutput = LOG_SERIAL;

  HSTlog(uint8_t output = LOG_SERIAL, uint16_t max_lines = 40);   // SERIAL by default, you can add more ( | LOG_MQTT | ...)

  using LogCallback = std::function<void(uint32_t, LogLevel, const char*)>;

  void log(LogLevel level, const String& msg);
  LogEntry getLastLog();
  uint8_t getLastEntries(LogEntry* out,
                         uint8_t maxCount,
                         LogLevel minLevel = LogLevel::DEBUG,
                         uint8_t page = 0) const;
  uint32_t getTime();

  // to be called from your setup in main.cpp (yourProject.ino)
  void setOutput(uint8_t output);           // optional, default: LOG_SERIAL
  void setSerialCallback(LogCallback cb);   // if you need it
  void setMqttCallback(LogCallback cb);     // if you need it
  void setDisplayCallback(LogCallback cb);  // if you need it
  void setTime(uint32_t unixtime);          // optional if not called, HSTlog will show upTime

  // --- print() ---
    void print(const char* msg);
    void print(char c);
    void print(bool b);
    void print(int v);
    void print(unsigned int v);
    void print(long v);
    void print(unsigned long v);
    void print(float v);
    void print(double v);

  // --- println() ---
    void println(const char* msg);
    void println(char c);
    void println(bool b);
    void println(int v);
    void println(unsigned int v);
    void println(long v);
    void println(unsigned long v);
    void println(float v);
    void println(double v);

    void printf(const char* fmt, ...);

private:
  uint8_t _output;
  uint16_t _maxLines;
  int32_t _baseEpoch = 0;             // Timestamp received to set clock
  uint32_t _baseMillis = 0;           // Millis @ setTime() call

  // Circular buffer
  volatile uint8_t _head = 0;
  volatile uint8_t _tail = 0;
  LogEntry* _lines;
  uint8_t _index = 0;
  uint8_t _count = 0;

  // Callbacks defined in main.cpp (yourProject.ino)
  LogCallback _serialCb = nullptr;
  LogCallback _mqttCb = nullptr;
  LogCallback _displayCb = nullptr;

  void dispatch(const char* msg);
};
