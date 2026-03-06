#include "HSTlog.h"
#include <stdio.h>
#include <string.h>

HSTlog::HSTlog(uint8_t output, uint16_t maxLines)
:   _output(output),
    _maxLines(maxLines)
{
    _lines = new LogEntry[_maxLines];
}

void HSTlog::log(LogLevel level, const String& msg) {
  uint32_t now = getTime();

  LogEntry e;               // Create log entry
  e.level = level;
  e.message = msg;
  e.timestamp = now;

  if (level >= displayLogLevel) {     // log only display levels
    _lines[_head] = e;
    _head = (_head + 1) % _maxLines;
    if (_count < _maxLines) {
      _count++;
    } else {
      _tail = (_tail + 1) % _maxLines;
    }
  }

  // --- Dispatch vers les callbacks actifs ---
  const char* safe = e.message.c_str(); // copie stable
  if (_serialCb)  _serialCb(now, level, safe);
  if (_mqttCb)    _mqttCb(now, level, safe);
  if (_displayCb) _displayCb(now, level, safe);
}

String HSTlog::getLastLines(uint8_t count) const {
    if (count > _count) count = _count;

    String out;
    uint8_t idx = (_head + _maxLines - count) % _maxLines;

    for (uint8_t i = 0; i < count; i++) {
        const LogEntry& e = _lines[(idx + i) % _maxLines];
        out += e.message;
        if (i < count - 1) out += "\n";
    }
//Serial.println(String("HSTlog::getLastLines: ") + out);  // Debug
    return out;
}

void HSTlog::setOutput(uint8_t output) {
    _output = output;
}

void HSTlog::setSerialCallback(LogCallback cb) {
    _serialCb = cb;
}

void HSTlog::setMqttCallback(LogCallback cb) {
    _mqttCb = cb;
}

void HSTlog::setDisplayCallback(LogCallback cb) {
    _displayCb = cb;
}

uint32_t HSTlog::getTime() {
  return ((millis() - _baseMillis) / 1000 + _baseEpoch);
}

void HSTlog::setTime(uint32_t unixtime) {
  _baseMillis = millis();
  _baseEpoch = unixtime;
}

uint8_t HSTlog::getLastEntries(LogEntry* out,
                               uint8_t maxCount,
                               LogLevel minLevel,
                               uint8_t page) const
{
    // 1) On collecte d'abord toutes les entrées filtrées (>= minLevel)
    LogEntry filtered[_maxLines];
    uint16_t fcount = 0;

    for (int i = _count - 1; i >= 0; --i) {
        const LogEntry& e = _lines[i];
        if (e.level >= minLevel) {
            filtered[fcount++] = e;
        }
    }

    if (fcount == 0) {
        return 0; // rien à afficher
    }

    // 2) Calcul de l'offset de pagination
    uint16_t start = page * maxCount;

    if (start >= fcount) {
      if (fcount > maxCount) {
        start = fcount - maxCount;
      } else {
        start = 0;
      }
    }

    // 3) On copie maxCount entrées à partir de start
    uint8_t written = 0;
    for (uint16_t i = start; i < fcount && written < maxCount; ++i) {
        out[written++] = filtered[i];
    }
    return written;
}

void HSTlog::dispatch(const char* msg) {
  log(LogLevel::DEBUG, msg);  // Enregistrer le message dans le buffer circulaire
//    if (_serialCb)  _serialCb(LogLevel::DEBUG, msg);
//    if (_mqttCb)    _mqttCb(LogLevel::DEBUG, msg);
//    if (_displayCb) _displayCb(LogLevel::DEBUG, msg);
}

// --- print() ---

void HSTlog::print(const char* msg) {
    dispatch(msg);
}

void HSTlog::print(char c) {
    char buf[2] = {c, 0};
    dispatch(buf);
}

void HSTlog::print(bool b) {
    dispatch(b ? "true" : "false");
}

void HSTlog::print(int v) {
    char buf[16];
    sprintf(buf, "%d", v);
    dispatch(buf);
}

void HSTlog::print(unsigned int v) {
    char buf[16];
    sprintf(buf, "%u", v);
    dispatch(buf);
}

void HSTlog::print(long v) {
    char buf[32];
    sprintf(buf, "%ld", v);
    dispatch(buf);
}

void HSTlog::print(unsigned long v) {
    char buf[32];
    sprintf(buf, "%lu", v);
    dispatch(buf);
}

void HSTlog::print(float v) {
    char buf[32];
    sprintf(buf, "%f", v);
    dispatch(buf);
}

void HSTlog::print(double v) {
    char buf[32];
    sprintf(buf, "%lf", v);
    dispatch(buf);
}

// --- println() ---

void HSTlog::println(const char* msg) {
    dispatch(msg);
}

void HSTlog::println(char c) {
    char buf[2] = {c, 0};
    dispatch(buf);
}

void HSTlog::println(bool b) {
    dispatch(b ? "true" : "false");
}

void HSTlog::println(int v) {
    char buf[16];
    sprintf(buf, "%d", v);
    dispatch(buf);
}

void HSTlog::println(unsigned int v) {
    char buf[16];
    sprintf(buf, "%u", v);
    dispatch(buf);
}

void HSTlog::println(long v) {
    char buf[32];
    sprintf(buf, "%ld", v);
    dispatch(buf);
}

void HSTlog::println(unsigned long v) {
    char buf[32];
    sprintf(buf, "%lu", v);
    dispatch(buf);
}

void HSTlog::println(float v) {
    char buf[32];
    sprintf(buf, "%f", v);
    dispatch(buf);
}

void HSTlog::println(double v) {
    char buf[32];
    sprintf(buf, "%lf", v);
    dispatch(buf);
}

// --- printf() ---

void HSTlog::printf(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    dispatch(buffer);
}
