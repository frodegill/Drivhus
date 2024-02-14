#include "plant.h"


Drivhus::Plant::Plant()
: enabled(false),
  watering_requested(false),
  in_watering_cycle(false),
  wet_value(0.0f),
  dry_value(0.0f),
  watering_duration_ms(0L),
  watering_grace_period_ms(0L),
  previous_watering_time(0L) {
}

Drivhus::Plant& Drivhus::Plant::operator=(const Plant& other) {
  if (this == &other)
      return *this;
      
  enabled = other.enabled;
  watering_requested = other.watering_requested;
  in_watering_cycle = other.in_watering_cycle;
  wet_value = other.wet_value;
  dry_value = other.dry_value;
  watering_duration_ms = other.watering_duration_ms;
  watering_grace_period_ms = other.watering_grace_period_ms;
  previous_watering_time = other.previous_watering_time;
}
