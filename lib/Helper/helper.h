#ifndef PORCHLIGHTSHELPER_H
#define PORCHLIGHTSHELPER_H
  #include <Arduino.h>
  #include "settings.h"
  #include "led_positions.h"
  #include <EEPROM.h>
    #define EPP_DATA_START 1000
    #define EPP_DATA_SIZE  2
    #define EPP_DATA_MAX_ENTRIES 366
  #include <avr/wdt.h>
    #define  WDTLen WDTO_2S
  #include <LowPower.h>
  #include <Wire.h>
  #include <Adafruit_NeoPixel.h>
  #include <Adafruit_MAX1704X.h>
  #include <NTC_Thermistor.h>
  #include <TDS3231.h>
  #include <TThread.h>
  
  // Threads
    #define t_battery  0
    #define t_sensors  1
    #define t_time     2
    #define t_system   3
    #define t_led      4
    #define thrd_count 5  

  // Holidays
    #define HOLIDAY_NOTSETUP     255
    #define HOLIDAY_NONE         0
    #define HOLIDAY_NEWYEAR      1
    #define HOLIDAY_MLK          2
    #define HOLIDAY_VALENTINE    3
    #define HOLIDAY_PRESIDENTS   4
    #define HOLIDAY_TAXDAY       5
    #define HOLIDAY_EARTHDAY     6
    #define HOLIDAY_MEMORIAL     7
    #define HOLIDAY_BDAY         8
    #define HOLIDAY_JULY4TH      9
    #define HOLIDAY_LABOR        10
    #define HOLIDAY_911          11
    #define HOLIDAY_COLOMBUS     12
    #define HOLIDAY_HALLOWEEN    13
    #define HOLIDAY_USMC         14
    #define HOLIDAY_VETERANS     15
    #define HOLIDAY_THANKSGIVING 16
    #define HOLIDAY_CHRISTMAS    17

  // Input/Output Pins
    #define PIN_OLITE    A0 // Ambient light sensor
    #define PIN_BTEMP1H  A1 // Battery temp sensor 1 5v
    #define PIN_BTEMP1   A2 // Battery temp sensor 1 Input
    #define PIN_BTEMP2H  A3 // Battery temp sensor 2 5v
    #define PIN_BTEMP2   A4 // Battery temp sensor 2 Input
    #define PIN_BHEAT    2  // Battery heater enable pin
    #define PIN_CENABLE  3  // CE (charge enable) on both of the solar chips (allowing for control over charging)
    #define PIN_PGOOD    4  // PGOOD from one of the solar chargers (indicating charge is available)
    #define PIN_LEDSTRIP 6  // Data out pin for the LED strip
  
    



  uint32_t period_t_millis();
  
  
  
  
  class PBattery {
    private:
    // Statuses
      bool _setupcomplete;
      bool _foundmax;
    
    // Battery info
      double _voltage;

    // Temperature control
      double _temperature[2];
      bool   _heateron;

    // Charge control
      bool   _chargeavailable;
      bool   _chargingenabled;

    public:
      PBattery();
      bool reset();
      bool begin();

      bool foundMax();

      bool getNewReadings();

      bool isHeatingAllowed();
      bool isHeating();
      bool enableHeater();
      bool disableHeater();

      bool   isChargingAvailable();
      bool   isChargingAllowed();
      bool   isChargingEnabled();
      bool   isCharging();
      bool   enableCharging();
      bool   disableCharging();

      double getVoltage();
      double getTemperature();
  };
  






  
  struct PLEDPixelStruct {
    bool     isallowed;
    uint16_t stripindex;
    uint8_t  section;
  };

  class PLEDStrip {
    private:
      bool _setupcomplete;
      
      uint8_t _color[3][4];
      uint8_t _colorcount;

      uint8_t _brightness;
      
      struct PLEDPixelStruct pled[LEDSTRIP_UCOUNT];

    public:
      PLEDStrip();
      bool reset();
      bool begin();
      bool demo(uint16_t delayval);

      bool setColor(uint8_t color[3][4]);
          
      bool    isLit();
      uint8_t getBrightness();
      bool    updatePixels();
      bool    setBrightness(uint8_t brightness);
      bool    setBrightness(uint8_t brightness, uint32_t fadedelay);

      bool     getLEDIsAllowed(uint8_t led);
      bool     setLEDIsAllowed(uint8_t led, bool isallowed);
      uint16_t getLEDStripIndex(uint8_t led);
      uint8_t  getLEDSection(uint8_t led);
  };
  







  class PorchLightSystem {
    private:
    // Current statuses
      bool _setupcomplete;
      bool _batterysaver;
    
    // Colors
      uint8_t  _dcolor[3][4]; // Default
      uint8_t  _hcolor[3][4]; // Holiday

    // Time data
      TTime_Full _now; // This is used just for printing date/time to the serial monitor (this way we aren't changing the public 'now' variable unexpectedly, but still get fresh date/time in the serial monitor)
      bool       _foundrtc;
      uint8_t    _holiday;
      bool       _latenight;
      bool       _recalcHoliday();

    // Sensor data
      uint8_t  _ambientlight;

    // Data logging
      bool _morningdatastored;
      bool _eveningdatastored;
      bool _storeBatteryDataToEEPROM(uint16_t sadr);


    public:
      PorchLightSystem();
      bool begin();

      bool setDefaultColors(); // defualt user colors
      bool setDefaultColors(uint8_t dcolor[3][4]);

      bool sleep(period_t period);
      bool sleep(period_t period, uint8_t multiplier);

      bool getNewSensorReadings();
      bool getNewBatteryReadings();
      bool getNewTimeData();

      uint8_t getAmbientLight();
      bool    inBatterySaverMode();

      #if DEBUGMODE == true
        bool serialPrintDateTime();
      #endif

      TThread thrd[thrd_count];

      bool    setThreadPriority(uint8_t threadpriority);
      uint8_t getThreadPriority();

      PBattery  battery;

      PLEDStrip ledstrip;
      bool updateLEDColor();

      TTime_Full now;
      bool    getFoundRTC() const;
      bool    isLateNight() const;
      uint8_t getCurrentHoliday();
  };
#endif