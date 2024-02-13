#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <string>


namespace Drivhus {

class Component {
public:
  virtual [[nodiscard]] bool init() = 0;
  virtual void loop() = 0;
  virtual std::string&& getName() = 0;
};

} //namespace

#endif // _COMPONENT_H_
