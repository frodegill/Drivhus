#ifndef _LOG_H_
#define _LOG_H_

#ifdef TESTING
# include "testing.h"
#endif

#include <string>

#include "component.h"
#include "settings.h"


namespace Drivhus {

class Log : public Component, public OnConfigChangeListener
{
public:
  enum class LogLevel {
    LEVEL_NONE,
    LEVEL_ERROR,
    LEVEL_INFO,
    LEVEL_DEBUG
  };

  enum class LogMode
  {
    MODE_NONE=0,
    MODE_SERIAL=1<<0,
    MODE_MQTT=1<<1,
    MODE_SERIAL_AND_MQTT=MODE_SERIAL|MODE_MQTT,
    MODE_ALL=MODE_SERIAL|MODE_MQTT
  };

public:
  Log(LogLevel log_level, LogMode log_mode);
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "Log";}

protected:
  virtual void onConfigChanged(OnConfigChangeListener::Type type, uint8_t id) override;

public:
  void setLogLevel(LogLevel log_level);
  void setLogMode(LogMode log_mode);

  void print(LogLevel level, const std::string& msg, LogMode filter=LogMode::MODE_ALL);

public:
  LogLevel m_log_level;
  LogMode m_log_mode;
};

} //namespace

#endif // _LOG_H_
