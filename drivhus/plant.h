#ifndef _PLANT_H_
#define _PLANT_H_

#ifdef TESTING
# include "testing.h"
#else
#endif

#include <memory>
#include <stdint.h>
#include <string>

#include "global.h"


namespace Drivhus {

struct Plant {
  bool enabled;
  bool watering_requested;
  bool in_watering_cycle;
  float current_value;
  float wet_value;
  float dry_value;
  unsigned long watering_duration_ms;
  unsigned long watering_grace_period_ms;
  unsigned long previous_watering_time;
public:
  Plant();
  Plant& operator=(const Plant& other);
};

[[nodiscard]] static constexpr bool isValidPlantId(uint8_t plant_id) {return plant_id>=1 && plant_id<=Drivhus::MAX_PLANT_COUNT;}

} //namespace

#endif // _GLOBAL_H_
