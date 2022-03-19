#include <Arduino.h>
#include "config.h"

#define USER_BUTTON 36

volatile uint8_t state;

TTGOClass *watch;
TFT_eSPI *tft;
BMA *sensor;
bool irq = false;



void IRAM_ATTR button_handle() {
    if (state == 0)
    {
        state = 1;
    }
    else if(state == 1)
    {
        state = 2;
    }
    else if (state == 2)
    {
        state = 3;
    }
    else if (state == 3)
    {
        state = 4;
    }
    else if (state == 4)
    {
        state = 5;
    }
    else if (state == 5)
    {
        state = 1;
    }
}


void init_step_counter(void)
{
    // Configure IMU
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;
    sensor->accelConfig(cfg);
    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(BMA423_INT1, [] {
        // Set interrupt to set irq value to 1
        irq = 1;
    }, RISING); //It must be a rising edge

    // Enable BMA423 step count feature
    sensor->enableFeature(BMA423_STEP_CNTR, true);

    // Reset steps
    sensor->resetStepCounter();

    // Turn on step interrupt
    sensor->enableStepCountInterrupt();
}

void hiking_session()
{

}

void setup()
{
    Serial.begin(115200);
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();

    //Receive objects for easy writing
    tft = watch->tft;
    sensor = watch->bma;

    init_step_counter();

    pinMode(USER_BUTTON, INPUT_PULLUP);
    attachInterrupt(USER_BUTTON, button_handle, CHANGE);
    state = 1;
}



void loop()
{

    switch (state)
    {
    case 1:
    {
        /* Stand-by mode*/

        // Some display settings
        tft->setTextColor(random(0xFFFF));
        tft->drawString("BMA423 StepCount", 3, 50, 4);
        tft->setTextFont(4);
        tft->setTextColor(TFT_WHITE, TFT_BLACK);

        if (irq) {
            irq = 0;
            bool  rlst;
            do {
                // Read the BMA423 interrupt status,
                // need to wait for it to return to true before continuing
                rlst =  sensor->readInterrupt();
            } while (!rlst);

            // Check if it is a step interrupt
            if (sensor->isStepCounter()) {
                // Get step data from register
                uint32_t step = sensor->getCounter();
                tft->setTextColor(random(0xFFFF), TFT_BLACK);
                tft->setCursor(45, 118);
                tft->print("StepCount:");
                tft->print(step);
                Serial.println(step);
            }
        }
        delay(20);

        Serial.println("State 1");

        break;
    }
    case 2:
    {
        /* Hiking session initalisation */
        // Open file for saving data
        Serial.println("State 2");


        // Reset steps count
        tft->setCursor(45, 118);
        tft->print("State 2");

        // Initalize distance variable
        float distance = 0;

        break;
    }
    case 3:
    {
        /* Hiking session ongoing */
        tft->setCursor(45, 118);
        tft->print("State 3");

        Serial.println("State 3");

       
        // Step counter code
        // Save step count
        
        // Get gps location
        // Save gps location

        // Calculate current distance
        
        // Show them to the user
        
        
        
        break;
    }
    case 4:
    {
        // Saving step, distance, coordinates 
        // Close file
        tft->setCursor(45, 118);
        tft->print("State 4");
        Serial.println("State 4");


        break;
    }

    case 5:
    {
        /* Sending session via bluetooth */
        tft->setCursor(45, 118);
        tft->print("State 5");
        Serial.println("State 5");

        break;
    }
    default:
        break;
    }
}