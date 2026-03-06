#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <stdarg.h>
#include <RTClib.h>
#include "Version.h"

enum DebugOutput {
  DEBUG_NONE    = 0,
  DEBUG_SERIAL  = 1 << 0,
  DEBUG_MQTT    = 1 << 1,
  DEBUG_DISPLAY = 1 << 2,
  DEBUG_LOG     = 1 << 3
};

enum class LogLevel {
  DEBUG     = 1,
  INFO      = 2,
  STATUS    = 3,
  WARNING   = 4,
  ERROR     = 5,
  BOOT      = 6,
  OLED      = 7
};

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
  uint8_t logOutput = DEBUG_SERIAL | DEBUG_MQTT;

  HSTlog(uint8_t output = DEBUG_SERIAL, uint16_t max_lines = 40);

  using LogCallback = std::function<void(uint32_t, LogLevel, const char*)>;

  void log(LogLevel level, const String& msg);
  String getLastLines(uint8_t count) const;
  uint8_t getLastEntries(LogEntry* out,
                         uint8_t maxCount,
                         LogLevel minLevel = LogLevel::DEBUG,
                         uint8_t page = 0) const;
  uint32_t getTime();

  void setOutput(uint8_t output);
  void setSerialCallback(LogCallback cb);
  void setMqttCallback(LogCallback cb);
  void setDisplayCallback(LogCallback cb);
  void setTime(uint32_t unixtime);

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

  // Buffer circulaire pour conserver les messages formatés
  volatile uint8_t _head = 0;
  volatile uint8_t _tail = 0;
  LogEntry* _lines;
  uint8_t _index = 0;
  uint8_t _count = 0;

  LogCallback _serialCb = nullptr;
  LogCallback _mqttCb = nullptr;
  LogCallback _displayCb = nullptr;

  void dispatch(const char* msg);
};
