#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <string>


namespace Drivhus {

class Component {
public:
  [[nodiscard]] virtual bool init() = 0;
  [[nodiscard]] virtual bool postInit() {return true;}
  virtual void loop() = 0;
  virtual const char* getName() const = 0;
};

} //namespace

#endif // _COMPONENT_H_
