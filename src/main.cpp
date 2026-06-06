#include "helper.h"
  PorchLightSystem sys;
  

void thread_LED();
void thread_System();
void thread_Battery();
void thread_Sensors();
void thread_Time();
void print_EPPROM(uint16_t daystoprint);

  
void setup() {
  wdt_enable(WDTLen);
  sys.thrd[t_battery].init(thread_Battery);
  sys.thrd[t_sensors].init(thread_Sensors);
  sys.thrd[t_time].init(thread_Time);
  sys.thrd[t_system].init(thread_System);
  sys.thrd[t_led].init(thread_LED);

  wdt_reset();
  uint8_t defaultcolors[3][4] = {{0,0,0,255}, {0,0,0,0}, {0,0,0,0}};
  sys.setDefaultColors(defaultcolors);

  wdt_reset();
  if (sys.begin()) {
    if (EEPROM.read(0) != 17) { for (uint16_t cep = 1; cep < EEPROM.length(); cep++) { EEPROM.update(cep,0); }; EEPROM.update(0,17); };

    if (!DebugMode) { sys.ledstrip.demo(); }
    else if (sys.getFoundRTC()) {
      Serial.println(F("Type anything to print a battery history report from EEPROM"));
      uint32_t stime = millis();
      while ((millis() - stime) < 7000) { if (Serial.available()) { print_EPPROM(EEPROMDaysToPrint); break; }; };
    };
  }
  else {
    if (DebugMode) { Serial.println(F("Failed system begin, pausing all tasks except battery management")); };
    for (uint8_t pt = 0; pt < thrd_count; pt++) { if (pt != t_battery) { sys.thrd[pt].setPaused(true); }; }; 
  };

  sys.setThreadPriority(tp_HIGH);
  wdt_reset();
}


void loop() { for (byte i = 0; i < thrd_count; i++) { sys.thrd[i].run(false); }; sys.sleep(SLEEP_8S,8); }
  



  


void print_EPPROM(uint16_t daystoprint) {
  daystoprint = daystoprint > 365 ? 365 : daystoprint;
  Serial.print(F(" -Printing last (")); Serial.print(daystoprint); Serial.println(F(") days of EEPROM data"));
  Serial.println(F("   DOY |   Morning    |   Evening    |    Combined     | Charged | Light Usage"));

  uint16_t dayslastyear = isLeapYear(sys.now.getYear()-1) ? 366 : 365;
  uint16_t startdoy = sys.now.getDayOfYear() >= daystoprint ? sys.now.getDayOfYear() - daystoprint : dayslastyear - (daystoprint - sys.now.getDayOfYear()); // 0 based day of year
  uint16_t currdoy = startdoy;
  bool     inpreviousyear = (sys.now.getDayOfYear() < daystoprint);
  uint16_t nextdoy = currdoy+1;
  if (inpreviousyear && (nextdoy >= dayslastyear)) { nextdoy = 0; inpreviousyear = false; }; // from here forward, inpreviousyear relates to nextdoy, NOT currdoy (look at the math at the end of the loop)
  
  uint8_t ttemp1; uint8_t ttemp2; double ttempa;
  double  tvolt1; double  tvolt2; double tvolta; double tvoltn;

  uint16_t cadr; uint16_t nadr;
  for (uint16_t printday = 0; printday < daystoprint; printday++) {
    nadr = EPP_DATA_START + (nextdoy * (EPP_DATA_SIZE*2));
    cadr = EPP_DATA_START + (currdoy * (EPP_DATA_SIZE*2));

    Serial.print(F("   ")); Serial.print(currdoy+1); if (currdoy < 9) { Serial.print(F("  ")); } else if (currdoy < 99) { Serial.print(F(" ")); }; Serial.print(F(" | "));
    ttemp1 = EEPROM.read(cadr);                     ttemp2 = EEPROM.read(cadr+EPP_DATA_SIZE);                     ttempa = (double(ttemp1)+double(ttemp2))/2;
    tvolt1 = (double(EEPROM.read(cadr+1))+200)/100; tvolt2 = (double(EEPROM.read(cadr+EPP_DATA_SIZE+1))+200)/100; tvolta = (tvolt1+tvolt2)/2;                 tvoltn = (double(EEPROM.read(nadr+1))+200)/100;
    Serial.print(ttemp1); Serial.print(F("*F")); if (ttemp1 < 10) { Serial.print(" "); }; Serial.print(F(" - ")); Serial.print(tvolt1); Serial.print(F("v | "));
    Serial.print(ttemp2); Serial.print(F("*F")); if (ttemp2 < 10) { Serial.print(" "); }; Serial.print(F(" - ")); Serial.print(tvolt2); Serial.print(F("v | "));
    Serial.print(ttempa); Serial.print(F("*F")); if (ttempa < 10) { Serial.print(" "); }; Serial.print(F(" - ")); Serial.print(tvolta); Serial.print(F("v |  "));
    Serial.print(tvolt2-tvolt1); Serial.print(F("v  |    ")); Serial.print(printday == (daystoprint-1) ? 0.00 : tvolt2-tvoltn); Serial.println(F("v"));

    currdoy = nextdoy; nextdoy++;
    if (inpreviousyear && nextdoy > (dayslastyear-1)) { inpreviousyear = false; nextdoy = 0; };
  };
}









void thread_Sensors() { sys.thrd[t_system].setTriggered(true); sys.getNewSensorReadings(); }
void thread_Time()    { sys.thrd[t_system].setTriggered(true); sys.getNewTimeData(); }




void thread_Battery() {
  sys.thrd[t_system].setTriggered(true);
  sys.getNewBatteryReadings();

  while (sys.battery.getTemperature() < TempToMaintain && sys.battery.isHeatingAllowed()) {
    if (!sys.battery.isHeating()) { if (!sys.battery.enableHeater()) { break; }; };
    if (DebugMode) { sys.serialPrintDateTime(); Serial.print(F("Heating to maintain temperature of ")); Serial.print(TempToMaintain); Serial.println(F("* F")); };
    sys.sleep(SLEEP_8S,4);
    sys.getNewBatteryReadings();
  };

  if (sys.battery.isChargingAvailable()) {
    while (sys.battery.getTemperature() < MinimumChargeTemperature && sys.battery.isHeatingAllowed()) {
      if (!sys.battery.isHeating()) { if (!sys.battery.enableHeater()) { break; }; };
      if (DebugMode) { sys.serialPrintDateTime(); Serial.print(F("Heating to enable charging at ")); Serial.print(MinimumChargeTemperature); Serial.println(F("* F")); };
      sys.sleep(SLEEP_8S,4);
      sys.getNewBatteryReadings();
    };
    if (!sys.battery.isChargingEnabled() && sys.battery.isChargingAllowed()) { if (sys.battery.enableCharging() && DebugMode) { sys.serialPrintDateTime(); Serial.println(F("Charging enabled")); }; };
  };

  if (sys.battery.isChargingEnabled() && !sys.battery.isChargingAllowed()) { if (sys.battery.disableCharging() && DebugMode) { sys.serialPrintDateTime(); Serial.println(F("Charging disabled")); }; };
  if (sys.battery.isHeating()) { sys.battery.disableHeater(); };

  if (sys.inBatterySaverMode()) {
    if (sys.getThreadPriority() != tp_HIGH) {
      if (DebugMode) { sys.serialPrintDateTime(); Serial.println(F("Entering battery saver mode")); };
      if (sys.ledstrip.isLit()) { sys.ledstrip.setBrightness(0,LEDStripFadeDelay); };
      sys.setThreadPriority(tp_HIGH);
    };
    sys.sleep(SLEEP_8S,8);
  }
  else { if (sys.getThreadPriority() == tp_HIGH) { if (DebugMode) { sys.serialPrintDateTime(); Serial.println(F("Exiting battery saver mode")); }; sys.setThreadPriority(tp_MED); }; };
}




void thread_System() { 
  bool allowleds = false;

  if (sys.getFoundRTC()) { 
    TimeOfDay tod = sys.now.getTimeOfDay();
    if      (tod == TOD_NIGHT || tod == TOD_MIDNIGHT || tod == TOD_MORNNIGHT)                 { allowleds = true; }
    else if (tod == TOD_DAWN  || tod == TOD_SUNRISE  || tod == TOD_SUNSET || tod == TOD_DUSK) { allowleds = (sys.getAmbientLight() <= LightPercentConsideredDark); };
  }
  else { allowleds = (sys.getAmbientLight() <= LightPercentConsideredDark); };

  if (sys.battery.getTemperature() < MinimumLightTemperature || sys.battery.isChargingAvailable() || sys.thrd[t_led].getPaused()) { allowleds = false; };

  if (allowleds) {
    if (sys.getThreadPriority() > tp_LOW) { sys.setThreadPriority(tp_LOW); if (DebugMode) { sys.serialPrintDateTime(); Serial.println(F("LEDs enabled")); }; };
  }
  else {
    if (sys.ledstrip.isLit()) { sys.ledstrip.setBrightness(0,LEDStripFadeDelay); };
    if (sys.getThreadPriority() < tp_MED) { sys.setThreadPriority(tp_MED); if (DebugMode) { sys.serialPrintDateTime(); Serial.println(F("LEDs disabled")); }; };
  };
  sys.thrd[t_led].setTriggered(allowleds);
}




void thread_LED() {
  bool somethingchanged = false;
  double bv = sys.battery.foundMax() ? sys.battery.getVoltage() : 0;

  if (sys.getFoundRTC()) { if (sys.now.getHour() == 0 && sys.now.getMin() <= 10) { somethingchanged = true; }; }; // Just in case we switched to a day that now is a holiday but otherwise had no other reason to update i guess

  bool latenight = !sys.getFoundRTC() ? (bv < 3.8) : ((bv < 3.8 && sys.now.getHour() >= 23) || sys.now.getHour() <= 2);

  uint16_t lsi = 0;
  uint8_t  lss = 0;
  bool     shouldallow = false;
  bool     altstairpattern = (latenight || bv < 3.71);
  uint8_t  ledchanges  = 0;
  uint8_t  ledson      = 0;
  for (uint8_t pi = 0; pi < LEDSTRIP_UCOUNT; pi++) {
    lsi = sys.ledstrip.getLEDStripIndex(pi);
    lss = sys.ledstrip.getLEDSection(pi);
    shouldallow = false;

    if      (lsi == SP_LEFTF_1  || lsi == SP_LEFTF_3 || lsi == SP_STAIR_7  || lsi == SP_STAIR_10) { shouldallow = true; }
    else if (lsi == SP_STAIR_8  || lsi == SP_STAIR_9)  { shouldallow = !altstairpattern; }
    else if (lsi == SP_STAIR_A2)                       { shouldallow = altstairpattern; }
    else if (lsi >= SP_STAIR_5  && lsi <= SP_STAIR_6)  { shouldallow = latenight ? false : bv >= 4.00; }
    else if (lsi == SP_LEFTR_2  || lsi == SP_LEFTR_4)  { shouldallow = latenight ? false : bv >= 3.85; }
    else if (lsi == SP_FRONT_2  || lsi == SP_FRONT_4)  { shouldallow = latenight ? false : bv >= 3.81; }
    else if (lsi == SP_FRONT_6  || lsi == SP_FRONT_8)  { shouldallow = latenight ? false : bv >= 3.78; }
    else if (lsi == SP_FRONT_10 || lsi == SP_FRONT_12) { shouldallow = latenight ? false : bv >= 3.75; }
    else if (lsi == SP_RITER_2  || lsi == SP_RITER_4)  { shouldallow = latenight ? false : bv >= 3.72; }
    else if (lss == SP_LEFTF)                          { shouldallow = latenight ? false : bv >= 3.69; }
    else if (lss == SP_LEFTR)                          { shouldallow = latenight ? false : bv >= 3.67; }
    else if (lss == SP_FRONT1)                         { shouldallow = latenight ? false : bv >= 3.64; }
    else if (lss == SP_FRONT3)                         { shouldallow = latenight ? false : bv >= 3.61; }
    else if (lss == SP_FRONT2)                         { shouldallow = latenight ? false : bv >= 3.58; }
    else if (lss == SP_RITER)                          { shouldallow = bv >= 3.54; }
    else if (lss == SP_STAIR || lsi == SP_STAIR_A1)    { 
      if (bv < 3.45) { shouldallow = false; }
      else {
        if      (lsi == SP_STAIR_1 || lsi == SP_STAIR_4) { shouldallow = true; }
        else if (lsi == SP_STAIR_A1)                     { shouldallow = altstairpattern; }
        else                                             { shouldallow = !altstairpattern; };
      };
    };

    if (sys.ledstrip.getLEDIsAllowed(pi) != shouldallow) { ledchanges++; somethingchanged = true; sys.ledstrip.setLEDIsAllowed(pi, shouldallow); };
    ledson+=shouldallow;
  };
  
  uint8_t calcdbrightness = 0;
  uint8_t mbrn = latenight ? 30 : 90; mbrn = mbrn > LEDStripMaxBrightness ? LEDStripMaxBrightness : mbrn;
  if (sys.battery.foundMax()) {
    uint8_t sb = 0;
    if      (bv > 4.03) { sb = 80; }
    else if (bv > 4.00) { sb = 75; }
    else if (bv > 3.95) { sb = 70; }
    else if (bv > 3.90) { sb = 65; }
    else if (bv > 3.80) { sb = 60; }
    else if (bv > 3.75) { sb = 55; }
    else if (bv > 3.70) { sb = 50; }
    else if (bv > 3.65) { sb = 45; }
    else if (bv > 3.60) { sb = 40; }
    else if (bv > 3.55) { sb = 35; }
    else if (bv > 3.45) { sb = 25; };
    calcdbrightness = sb < 20 ? 20 : mbrn < sb ? mbrn : sb;
  };
  if (latenight) { calcdbrightness/=2; };

  calcdbrightness = calcdbrightness < 20 ? 20 : calcdbrightness > mbrn ? mbrn : calcdbrightness;
  somethingchanged = somethingchanged || (sys.ledstrip.getBrightness() != calcdbrightness);

  if (somethingchanged) { 
    if (DebugMode) { sys.serialPrintDateTime(); Serial.println("Updating (" + String(ledchanges) + ") LEDs  |  LEDs On: " + String(ledson) + "/" + String(LEDSTRIP_UCOUNT) + "  |  Brightness: " + String(calcdbrightness) + "%"); };
    if (!sys.ledstrip.setBrightness(calcdbrightness, LEDStripFadeDelay)) { sys.thrd[t_led].setPaused(true); if (DebugMode) { sys.serialPrintDateTime(); Serial.println(F("LED thread paused due to error during update")); }; }; 
  };
}