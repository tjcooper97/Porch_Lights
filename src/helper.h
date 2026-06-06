#ifndef PORCHLIGHTSHELPER_H
#define PORCHLIGHTSHELPER_H
  #include <Arduino.h>
  #include "settings.h"
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
  
  // LED Strip
    #define LEDSTRIP_COUNT  476 // Total number of NeoPixels in the strip
    #define LEDSTRIP_UCOUNT 36  // Number of NeoPixels we are using in the strip (the others are off)

  // Holidays
    #define HOLIDAY_NOTSETUP     255
    #define HOLIDAY_NONE         0
    #define HOLIDAY_NEWYEAR      1
    #define HOLIDAY_VALENTINE    2
    #define HOLIDAY_TAXDAY       3
    #define HOLIDAY_EARTHDAY     4
    #define HOLIDAY_BDAY         5
    #define HOLIDAY_JULY4TH      6
    #define HOLIDAY_HALLOWEEN    7
    #define HOLIDAY_THANKSGIVING 8
    #define HOLIDAY_CHRISTMAS    9

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

  // LED Sections
    #define SP_LEFTF  1
    #define SP_LEFTR  2
    #define SP_FRONT1 3
    #define SP_FRONT2 4
    #define SP_FRONT3 5
    #define SP_STAIR  6
    #define SP_STAIRA 7
    #define SP_RITER  8

  // LED Pixel Positions
    #define SP_LEFTF_1 4
    #define SP_LEFTF_2 21
    #define SP_LEFTF_3 36
    #define SP_LEFTF_4 53

    #define SP_LEFTR_1 61
    #define SP_LEFTR_2 78
    #define SP_LEFTR_3 93
    #define SP_LEFTR_4 111
    
    #define SP_FRONT_1  119
    #define SP_FRONT_2  138
    #define SP_FRONT_3  153
    #define SP_FRONT_4  172
    
    #define SP_FRONT_5  179
    #define SP_FRONT_6  198
    #define SP_FRONT_7  213
    #define SP_FRONT_8  232
    
    #define SP_FRONT_9  239
    #define SP_FRONT_10 258
    #define SP_FRONT_11 273
    #define SP_FRONT_12 292
    
    #define SP_STAIR_1  296
    #define SP_STAIR_2  306
    #define SP_STAIR_A1 310
    #define SP_STAIR_3  315
    #define SP_STAIR_4  325
    #define SP_STAIR_5  326
    #define SP_STAIR_6  327
    #define SP_STAIR_7  328
    #define SP_STAIR_8  338
    #define SP_STAIR_A2 343
    #define SP_STAIR_9  347
    #define SP_STAIR_10 357  
    
    #define SP_RITER_1 362
    #define SP_RITER_2 379
    #define SP_RITER_3 394
    #define SP_RITER_4 411
    



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
      bool demo();

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

      bool serialPrintDateTime();

      TThread thrd[thrd_count];

      bool    setThreadPriority(uint8_t threadpriority);
      uint8_t getThreadPriority();

      PBattery  battery;

      PLEDStrip ledstrip;
      bool    updateLEDColor();

      TTime_Full now;
      bool    getFoundRTC();
      uint8_t getCurrentHoliday();
  };
#endif