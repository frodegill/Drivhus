#ifndef _LOG_H_
#define _LOG_H_

#ifdef TESTING
# include "testing.h"
#endif

#include <string>

#include "component.h"


namespace Drivhus {

class Log : public Component
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
    MODE_SERIAL_AND_MQTT=MODE_SERIAL|MODE_MQTT
  };

public:
  static constexpr LogLevel LOG_LEVEL = LogLevel::LEVEL_ERROR;
  static constexpr LogMode LOG_MODE = LogMode::MODE_SERIAL_AND_MQTT;
  static constexpr const char* MQTT_LOG_TOPIC = "debug";

public:
  Log(LogLevel log_level, LogMode log_mode);
  virtual [[nodiscard]] bool init() override;
  virtual void loop() override;
  virtual std::string&& getName() {return "Log";}

  void setLogLevel(LogLevel log_level);
  void setLogMode(LogMode log_mode);

  void print(LogLevel level, const std::string& msg);

public:
  LogLevel m_log_level;
  LogMode m_log_mode;
};

} //namespace

#endif // _LOG_H_
