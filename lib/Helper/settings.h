#ifndef PORCHLIGHTSSETTINGS_H
#define PORCHLIGHTSSETTINGS_H
  #define DEBUGMODE         false // Turning this on will enable debug serial printing
  #define EEPROMREPORTDAYS  30    // This report can be requested via the serial monitor by passing any charter during a window of time when booting

  #define BATTERYSAVERVOLTS 3.35  // Below this voltage, the system will do everything it can to conserve power & lighting will be fully disabled

  #define TEMP_MAX_CHARGE   110   // Maximum temperature charging is allowed at
  #define TEMP_MIN_CHARGE   34    // Minimum temperature charging is allowed at
  #define TEMP_MAX_HEAT     40    // Maximum temperature that the battery heater is able to be enabled
  #define TEMP_MIN_OPERATE  30    // Minimum temperature lighting is allowed at
  #define TEMP_MAINTAIN     28    // Temperature the system will attempt to maintain when there is enough charge to do so

  #define MAXBRIGHTNESS     80    // Maximum brightness the led strip can be
  #define LEDFADEDELAY      15    // Speed at which the leds will fade between different events
  #define DARKTRIGGER       5     // Light % at or which below it is considered dark (this gives more precision over led turn on time vs just time based)

  #define TEMP_RANGE_UPPER  55    // 
  #define TEMP_RANGE_LOWER  45    // 
#endif