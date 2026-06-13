#include "helper.h"

Adafruit_NeoPixel _strip(LEDSTRIP_COUNT, PIN_LEDSTRIP, NEO_GRBW + NEO_KHZ800);
Adafruit_MAX17048 _max;
Thermistor*       _ntc[2];
TDS3231           _rtc;


uint32_t period_t_millis(period_t period) {
  if      (period == SLEEP_15MS)  { return 15; }
  else if (period == SLEEP_30MS)  { return 30; }
  else if (period == SLEEP_60MS)  { return 60; }
  else if (period == SLEEP_120MS) { return 120; }
  else if (period == SLEEP_250MS) { return 250; }
  else if (period == SLEEP_500MS) { return 500; }
  else if (period == SLEEP_1S)    { return 1000; }
  else if (period == SLEEP_2S)    { return 2000; }
  else if (period == SLEEP_4S)    { return 4000; }
  else if (period == SLEEP_8S)    { return 8000; };
  return 0;
}















PBattery::PBattery() {
  _setupcomplete = false;
  _foundmax = false;

  pinMode(PIN_BHEAT,   OUTPUT); disableHeater();
  pinMode(PIN_CENABLE, OUTPUT); disableCharging();
  pinMode(PIN_BTEMP1H, OUTPUT); digitalWrite(PIN_BTEMP1H, LOW);
  pinMode(PIN_BTEMP1,  INPUT);
  pinMode(PIN_BTEMP2H, OUTPUT); digitalWrite(PIN_BTEMP2H, LOW);
  pinMode(PIN_BTEMP2,  INPUT);
  pinMode(PIN_PGOOD,   INPUT_PULLUP);
  reset(); 
}

bool PBattery::reset() {
  _voltage    = 0;
  
  _temperature[0] = 0;
  _temperature[1] = 0;
  _heateron       = false;
  
  _chargeavailable = false;
  _chargingenabled = false;

  return true;
}

bool PBattery::begin() {
  if (_setupcomplete) { return true; };

  _ntc[0] = new NTC_Thermistor(PIN_BTEMP1,10000,10000,25,3950);
  _ntc[1] = new NTC_Thermistor(PIN_BTEMP2,10000,10000,25,3950);

  _foundmax = _max.begin();

  _setupcomplete = true;
  return true;
};



bool PBattery::foundMax() { return _foundmax; }



bool PBattery::getNewReadings() {
  if (!_setupcomplete) { return false; };
  
  _chargeavailable = (digitalRead(PIN_PGOOD) == LOW);

  if (_foundmax) { 
    _max.wake(); 
    delay(50); 
    _voltage = double(_max.cellVoltage());
    _max.hibernate(); 
  };

  // Battery temp
  digitalWrite(PIN_BTEMP1H, HIGH); digitalWrite(PIN_BTEMP2H, HIGH); delay(50);
  _ntc[0]->readFahrenheit(); _ntc[1]->readFahrenheit(); // Throw aways
  
  double tT = 3; double t0 = 0; double t1 = 0;
  for (uint8_t i = 0; i < tT; i++) {
    t0 += double(_ntc[0]->readFahrenheit());
    t1 += double(_ntc[1]->readFahrenheit());
    delay(5);
  };

  _temperature[0] = (t0/tT)-2; _temperature[1] = (t1/tT)-2;
  digitalWrite(PIN_BTEMP1H, LOW); digitalWrite(PIN_BTEMP2H, LOW);

  return true;
};




bool PBattery::isHeatingAllowed() { 
  if (!_setupcomplete) { return false; };
  if (getTemperature() > TEMP_MAX_HEAT) { return false; };
  if (_foundmax && _voltage < (_chargeavailable ? 3.2 : 3.35)) { return false; };
  return true;
}
bool PBattery::isHeating() { return _heateron; }

bool PBattery::enableHeater() {
  if (!_setupcomplete) { return false; };
  if (getTemperature() > TEMP_MAX_HEAT) { disableHeater(); return false; };

  _heateron = true;
  digitalWrite(PIN_BHEAT, HIGH);

  return true;
}

bool PBattery::disableHeater() {
  digitalWrite(PIN_BHEAT, LOW); 
  _heateron = false;
  return true;
}




bool   PBattery::isChargingAvailable() { return !_setupcomplete ? false : _chargeavailable; }
bool   PBattery::isChargingAllowed()   { return (_setupcomplete && (getTemperature() >= TEMP_MIN_CHARGE) && (getTemperature() <= TEMP_MAX_CHARGE)); }
bool   PBattery::isChargingEnabled()   { return !_setupcomplete ? false : _chargingenabled; }
bool   PBattery::isCharging()          { return !_setupcomplete ? false : (_chargingenabled && _chargeavailable); }
bool   PBattery::enableCharging() { 
  if (!isChargingAllowed()) { disableCharging(); return false; };
  _chargingenabled = true;
  digitalWrite(PIN_CENABLE, LOW);
  return true;
}
bool PBattery::disableCharging() {
  digitalWrite(PIN_CENABLE, HIGH);
  _chargingenabled = false;
  return true;
}




double PBattery::getVoltage()        { return (!_foundmax || !_setupcomplete) ? 0 : _voltage; }
double PBattery::getTemperature()    { return !_setupcomplete ? 0 : (_temperature[0] + _temperature[1]) / 2; }





















































PLEDStrip::PLEDStrip() { 
  _setupcomplete = false;

  uint16_t cp;
  cp=0; pled[cp].stripindex = SP_LEFTF_1; pled[cp].section = SP_LEFTF;
  cp++; pled[cp].stripindex = SP_LEFTF_2; pled[cp].section = SP_LEFTF;
  cp++; pled[cp].stripindex = SP_LEFTF_3; pled[cp].section = SP_LEFTF;
  cp++; pled[cp].stripindex = SP_LEFTF_4; pled[cp].section = SP_LEFTF;
  
  cp++; pled[cp].stripindex = SP_LEFTR_1; pled[cp].section = SP_LEFTR;
  cp++; pled[cp].stripindex = SP_LEFTR_2; pled[cp].section = SP_LEFTR;
  cp++; pled[cp].stripindex = SP_LEFTR_3; pled[cp].section = SP_LEFTR;
  cp++; pled[cp].stripindex = SP_LEFTR_4; pled[cp].section = SP_LEFTR;
  
  cp++; pled[cp].stripindex = SP_FRONT_1;  pled[cp].section = SP_FRONT1;
  cp++; pled[cp].stripindex = SP_FRONT_2;  pled[cp].section = SP_FRONT1;
  cp++; pled[cp].stripindex = SP_FRONT_3;  pled[cp].section = SP_FRONT1;
  cp++; pled[cp].stripindex = SP_FRONT_4;  pled[cp].section = SP_FRONT1;
  cp++; pled[cp].stripindex = SP_FRONT_5;  pled[cp].section = SP_FRONT2;
  cp++; pled[cp].stripindex = SP_FRONT_6;  pled[cp].section = SP_FRONT2;
  cp++; pled[cp].stripindex = SP_FRONT_7;  pled[cp].section = SP_FRONT2;
  cp++; pled[cp].stripindex = SP_FRONT_8;  pled[cp].section = SP_FRONT2;
  cp++; pled[cp].stripindex = SP_FRONT_9;  pled[cp].section = SP_FRONT3;
  cp++; pled[cp].stripindex = SP_FRONT_10; pled[cp].section = SP_FRONT3;
  cp++; pled[cp].stripindex = SP_FRONT_11; pled[cp].section = SP_FRONT3;
  cp++; pled[cp].stripindex = SP_FRONT_12; pled[cp].section = SP_FRONT3;
  
  cp++; pled[cp].stripindex = SP_STAIR_1;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_2;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_3;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_4;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_5;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_6;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_7;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_8;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_9;  pled[cp].section = SP_STAIR;
  cp++; pled[cp].stripindex = SP_STAIR_10; pled[cp].section = SP_STAIR;

  cp++; pled[cp].stripindex = SP_STAIRA_1; pled[cp].section = SP_STAIRA;
  cp++; pled[cp].stripindex = SP_STAIRA_2; pled[cp].section = SP_STAIRA;
  
  cp++; pled[cp].stripindex = SP_RITER_1; pled[cp].section = SP_RITER;
  cp++; pled[cp].stripindex = SP_RITER_2; pled[cp].section = SP_RITER;
  cp++; pled[cp].stripindex = SP_RITER_3; pled[cp].section = SP_RITER;
  cp++; pled[cp].stripindex = SP_RITER_4; pled[cp].section = SP_RITER;
  
  reset(); 
}

bool PLEDStrip::reset() {
  uint8_t cv;
  for (uint8_t ci = 0; ci < 3; ci++) { for (cv = 0; cv < 4; cv++) { _color[ci][cv] = 0; }; };
  _color[0][3] = 255;
  _colorcount = 1;

  _brightness = 0;

  for (uint8_t cpled = 0; cpled < LEDSTRIP_UCOUNT; cpled++) { pled[cpled].isallowed = false; };

  return true;
}

bool PLEDStrip::begin() {
  if (!_setupcomplete) { _strip.begin(); _setupcomplete = true; };
  if (_setupcomplete) { _strip.setBrightness(255); setBrightness(0,0); };
  return setBrightness(0,0);
};




bool PLEDStrip::demo() {
  if (!_setupcomplete) { return false; };


  return true;
}




bool PLEDStrip::setColor(uint8_t color[3][4]) {
  bool somethingchanged = false;

  _colorcount = 0;
  bool foundgreaterthanzero;
  uint8_t pi;
    for (uint8_t ci = 0 ; ci < 3; ci++) {
      foundgreaterthanzero = false;
      for (pi = 0; pi < 4; pi++) { 
        if (_color[ci][pi] != color[ci][pi]) { _color[ci][pi] = color[ci][pi]; somethingchanged = true; };
        foundgreaterthanzero = foundgreaterthanzero || color[ci][pi] > 0;
      };
      if (!foundgreaterthanzero) { break; };
      _colorcount = ci+1;
    };
  if (_colorcount <= 0) { _color[0][3] = 255; _colorcount = 1; somethingchanged = true; };
  
  return somethingchanged;
}


bool    PLEDStrip::isLit()                           { return (_setupcomplete && (_brightness > 0)); }
uint8_t PLEDStrip::getBrightness()                   { return _brightness; }
bool    PLEDStrip::updatePixels()                    { return setBrightness(_brightness,0); }
bool    PLEDStrip::setBrightness(uint8_t brightness) { return setBrightness(brightness,0); }
bool    PLEDStrip::setBrightness(uint8_t brightness, uint32_t fadedelay) {
  if (!_setupcomplete || brightness > 100 || fadedelay > 50) { return false; };
  if (_brightness == brightness) { fadedelay = 0; };

  bool increasing = (_brightness <= brightness);

  uint8_t ci, pi, tcolor[3][4];
  wdt_disable();
  wdt_enable(WDTO_8S);
  for (int16_t bl = int16_t(_brightness); (increasing ? bl <= int16_t(brightness) : bl >= int16_t(brightness)); bl = (increasing ? bl+1 : bl-1)) {
    for (ci = 0; ci < _colorcount; ci++) { for (pi = 0; pi < 4; pi++) { tcolor[ci][pi] = uint8_t((int16_t(_color[ci][pi]) * bl)/100); }; };

    ci = 0;
    for (pi = 0; pi < LEDSTRIP_UCOUNT; pi++) {
      if (ci >= _colorcount) { ci = 0; };
      if      (!pled[pi].isallowed)           { _strip.setPixelColor(pled[pi].stripindex, _strip.Color(0,0,0,0)); }
      else if (pled[pi].section == SP_STAIR
            || pled[pi].section == SP_STAIRA) { _strip.setPixelColor(pled[pi].stripindex, _strip.Color(0,0,0,uint8_t((255*bl)/100))); }
      else                                    { _strip.setPixelColor(pled[pi].stripindex, _strip.Color(tcolor[ci][0],tcolor[ci][1],tcolor[ci][2],tcolor[ci][3])); ci++; };
    };
    _strip.show();
    delay(fadedelay);
  };
  wdt_disable();
  wdt_enable(WDTO_8S);

  _brightness = uint8_t(brightness);
  return true;
}




bool     PLEDStrip::getLEDIsAllowed(uint8_t led)                 { if (led >= LEDSTRIP_UCOUNT || !_setupcomplete) { return false; }; return pled[led].isallowed; }
bool     PLEDStrip::setLEDIsAllowed(uint8_t led, bool isallowed) { if (led >= LEDSTRIP_UCOUNT || !_setupcomplete) { return false; }; pled[led].isallowed = isallowed; return true; }
uint16_t PLEDStrip::getLEDStripIndex(uint8_t led)                { if (led >= LEDSTRIP_UCOUNT || !_setupcomplete) { return 0; }; return pled[led].stripindex; };
uint8_t  PLEDStrip::getLEDSection(uint8_t led)                   { if (led >= LEDSTRIP_UCOUNT || !_setupcomplete) { return 0; }; return pled[led].section; };























  







PorchLightSystem::PorchLightSystem() {
  _setupcomplete = false;
  
  pinMode(LED_BUILTIN, OUTPUT); digitalWrite(LED_BUILTIN, LOW);
  pinMode(PIN_OLITE, INPUT);

  for (uint8_t cp = A5; cp <= A15; cp++) { pinMode(cp, INPUT_PULLUP); };
  for (uint8_t cp = 22; cp <= 49; cp++) { pinMode(cp, INPUT_PULLUP); };

  _batterysaver = false;

  _foundrtc = false;
  _holiday  = HOLIDAY_NOTSETUP;

  _ambientlight = 0;

  _morningdatastored = false;
  _eveningdatastored = false;

  now.setRegion(SER_MIDWEST);
  now.setDateFormat(FORMAT_DATE_US);
  now.setTimeFormat(FORMAT_TIME_24);
  now.showCentury(true);
  now.showSeconds(true);
  _now = now;

  setDefaultColors();
}

bool PorchLightSystem::begin() {
  if (_setupcomplete) { return true; };
  wdt_reset();

  if (DEBUGMODE) { 
    Serial.begin(115200);
    Serial.println(F("Coopers Porch Lights"));
    Serial.print(F("Setting up Threads... ")); 
  };
    TThread_Priority = tp_HIGH;
    thrd[t_battery].setPriority(tp_HIGH);
    thrd[t_sensors].setPriority(tp_MED);
    thrd[t_time].setPriority(tp_MED);
    thrd[t_system].setPriority(tp_MED);
    thrd[t_led].setPriority(tp_LOW);

    thrd[t_battery].setTrigType(tt_INTERVAL);
    thrd[t_sensors].setTrigType(tt_INTERVAL);
    thrd[t_time].setTrigType(tt_INTERVAL);
    thrd[t_system].setTrigType(tt_BOOLR);
    thrd[t_led].setTrigType(tt_BOOLR);
    
    thrd[t_battery].setInterval(DEBUGMODE ? 30000 : 600000);
    thrd[t_sensors].setInterval(DEBUGMODE ? 15000 : 60000);
    thrd[t_time].setInterval(DEBUGMODE ? 20000 : 300000);
  if (DEBUGMODE) { Serial.println(F("Complete!")); };


  if (DEBUGMODE) { Serial.print(F("Setting up LED Strip... ")); };
    wdt_reset();
    if (!ledstrip.begin()) { if (DEBUGMODE) { Serial.println(F("Error!")); }; }
    else                   { if (DEBUGMODE) { Serial.println(F("Complete!")); }; };
  

  if (DEBUGMODE) { Serial.print(F("Searching for DS3231... ")); };
    wdt_reset();
    _foundrtc = _rtc.begin();
    if (!_foundrtc) { if (DEBUGMODE) { Serial.println(F("Not Found!")); }; }
    else { 
      now = _rtc.getNow();
      if (now.isDST()) { now.addTime(1,0,0); };
      if (now.getYear() < 2026) { 
        _foundrtc = false; 
        if (DEBUGMODE) { Serial.println(F("DS3231 was found, but has lost power & the time has not been reset, so considered not found")); };
      }
      else if (DEBUGMODE) { 
        Serial.println(F("Found!"));
        Serial.print(F("  >Date:= ")); Serial.print(now.getDayStr() + ", "); Serial.print(now.getMonStr() + " "); Serial.print(now.getDay()); Serial.print(", "); Serial.println(now.getYear());
        Serial.print(F("  >Time:= ")); Serial.println(now.getTime());
      };
    };
  

  if (DEBUGMODE) { Serial.print(F("Setting up Battery... ")); };
    wdt_reset();
    if (!battery.begin()) { if (DEBUGMODE) { Serial.println(F("Error!")); }; }
    else { 
      battery.getNewReadings();
      if (DEBUGMODE) { 
        Serial.println(F("Complete!"));
        Serial.print(F("  >Found MAX   := ")); Serial.println(battery.foundMax() ? "True" : "False");
        Serial.print(F("  >Average Temp:= ")); Serial.print(battery.getTemperature()); Serial.println(F("* F"));
        Serial.print(F("  >Voltage     := ")); Serial.print(battery.getVoltage()); Serial.println(F("v"));
        Serial.print(F("  >Charge Avail:= ")); Serial.println(battery.isChargingAvailable() ? "True" : "False");
      };
    };


  if (DEBUGMODE) { serialPrintDateTime(); Serial.println(F("Setup Complete!")); };
  _setupcomplete = true;
  return true;
}



bool PorchLightSystem::setDefaultColors() { 
  uint8_t defaultcolor[3][4] = {{0,0,0,255}, {0,0,0,0}, {0,0,0,0}};
  return setDefaultColors(defaultcolor);
}
bool PorchLightSystem::setDefaultColors(uint8_t dcolor[3][4]) {
  uint8_t dcy = 0;
  for (uint8_t dcx = 0; dcx < 3; dcx++) { for (dcy = 0; dcy < 4; dcy++) { _dcolor[dcx][dcy] = dcolor[dcx][dcy]; }; };
  return true;
}




bool PorchLightSystem::sleep(period_t period) { return sleep(period,1); }
bool PorchLightSystem::sleep(period_t period, uint8_t multiplier) {
  wdt_reset();

  multiplier = DEBUGMODE || multiplier < 1 ? 1 : multiplier > 8 ? 8 : multiplier;
  uint32_t sleepmillis = period_t_millis(period) * uint32_t(multiplier);

  if (DEBUGMODE) { 
    serialPrintDateTime(); 
    Serial.print(F("Sleeping for ")); Serial.print((double(sleepmillis)/1000)); Serial.println(F(" seconds"));
    Serial.flush();
  };

  wdt_disable();
    for (uint8_t sl = 0; sl < multiplier; sl++) { LowPower.idle(period, ADC_ON, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF, USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF); };
  wdt_enable(WDTLen);

  for (uint8_t ft = 0; ft < thrd_count; ft++) { thrd[ft].setLastRunMillis(thrd[ft].getLastRunMillis() - sleepmillis); };

  wdt_reset();
  return true;
}



bool PorchLightSystem::getNewSensorReadings() {
  _ambientlight = 0;
  for (uint8_t nalr = 0; nalr < 3; nalr++) { _ambientlight += uint8_t(map(analogRead(PIN_OLITE),0,1024,100,0)); };
  _ambientlight/=3;
  _ambientlight = _ambientlight > 100 ? 100 : _ambientlight;
  
  if (DEBUGMODE) { 
    serialPrintDateTime();
    Serial.print(F("Sensor  Data  |  Light:= ")); Serial.print(String(_ambientlight) + "%");
    Serial.println();
  };

  return true;
}


bool PorchLightSystem::getNewBatteryReadings() {
  bool gotnewreadings = battery.getNewReadings();
  
  if (DEBUGMODE) { 
    serialPrintDateTime();
    Serial.print(F("Battery Data  |  Temp:= ")); Serial.print(battery.getTemperature()); Serial.print("* F");
    Serial.print(F("  |  Volt:= ")); Serial.print(battery.getVoltage());     Serial.print("v"); 
    Serial.print(F("  |  CAvail:= ")); Serial.print(battery.isChargingAvailable() ? "True" : "False");
    Serial.print(F("  |  CEnabl:= ")); Serial.println(battery.isCharging() ? "True" : "False");
  };

  _batterysaver = DEBUGMODE ? false : (!battery.foundMax() || battery.getVoltage() < BATTERYSAVERVOLTS);

  return gotnewreadings;
}


bool PorchLightSystem::_recalcHoliday() {
  _holiday = HOLIDAY_NONE;

  if (now.getMon() == JANUARY) {
    if      (now.getDay() == 1) { _holiday = HOLIDAY_NEWYEAR; }
    else if (now.getDay() > 14 && now.getDay() < 22 && now.getDayOfWeek() == MONDAY) { _holiday = HOLIDAY_MLK; };
    return true;
  };
  if (now.getMon() == FEBRUARY) {
    if      (now.getDay() == 14) { _holiday = HOLIDAY_VALENTINE; }
    else if (now.getDay() > 14 && now.getDay() < 22 && now.getDayOfWeek() == MONDAY) { _holiday = HOLIDAY_PRESIDENTS; };
    return true;
  };
  if (now.getMon() == APRIL) { 
    if      (now.getDay() == 15) { _holiday = HOLIDAY_TAXDAY; }
    else if (now.getDay() == 22) { _holiday = HOLIDAY_EARTHDAY; };
    return true;
  };
  if (now.getMon() == MAY)       { if (now.getDay() >  24 && now.getDayOfWeek() == MONDAY) { _holiday = HOLIDAY_MEMORIAL; }; return true; };
  if (now.getMon() == JUNE)      { if (now.getDay() == 22)                                 { _holiday = HOLIDAY_BDAY; };     return true; };
  if (now.getMon() == JULY)      { if (now.getDay() == 4)                                  { _holiday = HOLIDAY_JULY4TH; };  return true; };
  if (now.getMon() == SEPTEMBER) { 
    if      (now.getDay() < 8 && now.getDayOfWeek() == MONDAY) { _holiday = HOLIDAY_LABOR; }
    else if (now.getDay() == 11)                               { _holiday = HOLIDAY_911; };
    return true;
  };
  if (now.getMon() == OCTOBER) { 
    if      (now.getDay() > 7 && now.getDay() < 15 && now.getDayOfWeek() == MONDAY) { _holiday = HOLIDAY_COLOMBUS; }
    else if (now.getDay() == 31) { _holiday = HOLIDAY_HALLOWEEN; };
    return true;
  };
  if (now.getMon() == NOVEMBER)  { 
    if      (now.getDay() == 10) { _holiday = HOLIDAY_USMC; }
    else if (now.getDay() == 11) { _holiday = HOLIDAY_VETERANS; }
    else if (now.getDay() > 20 && now.getDay() < 30) {
      if ((now.getDayOfWeek() == WEDNESDAY && now.getDay() < 28)
       || (now.getDayOfWeek() == THURSDAY  && now.getDay() > 21 && now.getDay() < 29)
       || (now.getDayOfWeek() == FRIDAY    && now.getDay() > 22)) 
      { _holiday = HOLIDAY_THANKSGIVING; };
    };
    return true;
  };
  if (now.getMon() == DECEMBER) {
    if (now.getDay() == 24 || now.getDay() == 25) { _holiday = HOLIDAY_CHRISTMAS; return true; };
    if (now.getDay() == 31)                       { _holiday = HOLIDAY_NEWYEAR; };
  };
  
  return true;
}


bool PorchLightSystem::getNewTimeData() {
  if (!_foundrtc) { return false; };

  _now = now;
  now = _rtc.getNow();
  if (now.isDST()) { now.addTime(1,0,0); };

  if (_now.getTimeOfDay() != now.getTimeOfDay()) { thrd[t_sensors].setPaused((!DEBUGMODE && now.getTimeOfDay() != TOD_DAWN && now.getTimeOfDay() != TOD_SUNRISE && now.getTimeOfDay() != TOD_SUNSET && now.getTimeOfDay() != TOD_DUSK)); };

  if (now.getDay() != _now.getDay() || _holiday == HOLIDAY_NOTSETUP) { _recalcHoliday(); updateLEDColor(); };

  if      (now.getHour() == 0)                                                                                               { _morningdatastored = false; _eveningdatastored = false; }
  else if (!_morningdatastored && now.getTimeOfDay() == TOD_MORNING   && now.getSunriseMinutes()+30 < now.getTotalMinutes()) { _morningdatastored = _storeBatteryDataToEEPROM(EPP_DATA_START + (EPP_DATA_SIZE * ((now.getDayOfYear()-1)*2))); }
  else if (!_eveningdatastored && now.getTimeOfDay() == TOD_AFTERNOON && now.getSunsetMinutes()-40  < now.getTotalMinutes()) { _eveningdatastored = _storeBatteryDataToEEPROM(EPP_DATA_START + (EPP_DATA_SIZE * ((now.getDayOfYear()-1)*2)) + EPP_DATA_SIZE); };

  if (DEBUGMODE) { 
    serialPrintDateTime();
    Serial.print(F("Time    Data  |  TimeOfDay:= ")); Serial.print(now.getTimeOfDayStr());
    Serial.print(F("  |  Current Holiday:= ")); Serial.print(_holiday);
    
    uint8_t srh = uint8_t(double(now.getSunriseMinutes()) / 60);
    uint8_t srm = now.getSunriseMinutes()-(srh*60);
    uint8_t ssh = uint8_t(double(now.getSunsetMinutes()) / 60);
    uint8_t ssm = now.getSunsetMinutes()-(ssh*60);
    Serial.print(F("  |  Sunrise:= ")); Serial.print(srh); Serial.print(":"); Serial.print(srm < 10 ? "0" : ""); Serial.print(srm);
    Serial.print(F("  |  Sunset:= "));  Serial.print(ssh); Serial.print(":"); Serial.print(ssm < 10 ? "0" : ""); Serial.println(ssm);
  };

  return true;
}


bool PorchLightSystem::_storeBatteryDataToEEPROM(uint16_t sadr) {
  EEPROM.update(sadr,  uint8_t(battery.getTemperature()));
  EEPROM.update(sadr+1,uint8_t((battery.getVoltage()*100)-200));
  return true;
}


uint8_t PorchLightSystem::getAmbientLight() { return _setupcomplete ? _ambientlight : 0; }
bool    PorchLightSystem::inBatterySaverMode()     { return _batterysaver; }


bool PorchLightSystem::serialPrintDateTime() {
  if (!DEBUGMODE || !_foundrtc) { return false; };
  _now = _rtc.getNow();
  if (_now.isDST()) { _now.addTime(1,0,0); };
  Serial.print(_now.getDate() + " - " + _now.getTime() + "  |  ");
  return true;
}


bool PorchLightSystem::setThreadPriority(uint8_t threadpriority) { 
  if (DEBUGMODE) { serialPrintDateTime(); Serial.print(F("Changed thread priority to:= ")); Serial.println(threadpriority); };
  TThread_Priority = threadpriority;
  return true;
}
uint8_t PorchLightSystem::getThreadPriority() { return TThread_Priority; }


bool PorchLightSystem::updateLEDColor() { 
  if (getCurrentHoliday() == HOLIDAY_NONE) { return ledstrip.setColor(_dcolor); };

  uint8_t pi; for (uint8_t ci = 0 ; ci < 3; ci++) { for (pi = 0; pi < 4; pi++) { _hcolor[ci][pi] = 0; }; };
    
  if      (_holiday == HOLIDAY_NEWYEAR)      { _hcolor[0][2] = 255;                                           _hcolor[1][0] = 222; _hcolor[1][1] = 193; _hcolor[1][2] = 4; }
  else if (_holiday == HOLIDAY_MLK)          { _hcolor[0][0] = 146; _hcolor[0][1] = 3;   _hcolor[0][2] = 255; _hcolor[1][0] = 222; _hcolor[1][1] = 193; _hcolor[1][2] = 4; }
  else if (_holiday == HOLIDAY_VALENTINE)    { _hcolor[0][0] = 255;                                           _hcolor[1][3] = 255; }
  else if (_holiday == HOLIDAY_TAXDAY)       { _hcolor[0][1] = 255; }
  else if (_holiday == HOLIDAY_EARTHDAY)     { _hcolor[0][2] = 255;                                           _hcolor[1][1] = 255; }
  else if (_holiday == HOLIDAY_BDAY)         { _hcolor[0][0] = 146; _hcolor[0][1] = 3;   _hcolor[0][2] = 255; }
  else if (_holiday == HOLIDAY_PRESIDENTS
        || _holiday == HOLIDAY_MEMORIAL
        || _holiday == HOLIDAY_JULY4TH
        || _holiday == HOLIDAY_LABOR
        || _holiday == HOLIDAY_911
        || _holiday == HOLIDAY_COLOMBUS
        || _holiday == HOLIDAY_VETERANS)     { _hcolor[0][0] = 255;                                           _hcolor[1][3] = 255;                                         _hcolor[2][2] = 255; }
  else if (_holiday == HOLIDAY_HALLOWEEN)    { _hcolor[0][0] = 146; _hcolor[0][1] = 3;   _hcolor[0][2] = 255; _hcolor[1][0] = 247; _hcolor[1][1] = 125; _hcolor[1][2] = 2; }
  else if (_holiday == HOLIDAY_USMC)         { _hcolor[0][0] = 255;                                           _hcolor[1][0] = 222; _hcolor[1][1] = 193; _hcolor[1][2] = 4; }
  else if (_holiday == HOLIDAY_THANKSGIVING) { _hcolor[0][0] = 247; _hcolor[0][1] = 125; _hcolor[0][2] = 2; }
  else if (_holiday == HOLIDAY_CHRISTMAS)    { _hcolor[0][0] = 255;                                           _hcolor[1][1] = 255; }
  else                                       { _hcolor[0][3] = 255; };

  return ledstrip.setColor(_hcolor); 
}


bool    PorchLightSystem::getFoundRTC()        { return _foundrtc; }
uint8_t PorchLightSystem::getCurrentHoliday()  { return (!_setupcomplete || _holiday == HOLIDAY_NOTSETUP) ? HOLIDAY_NONE : _holiday; }