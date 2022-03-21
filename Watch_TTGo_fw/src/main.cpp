#include "config.h"
#include "BluetoothSerial.h"

#include <stdio.h>
#include <stdlib.h>

// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Bluetooth Serial object
BluetoothSerial SerialBT;

// Watch objects
TTGOClass *watch;
TFT_eSPI *tft;
BMA *sensor;
TinyGPSPlus *gps=nullptr;

/************ Might not need ************/
// 
uint32_t last = 0;
uint32_t updateTimeout = 0;

/***************************************/


volatile uint8_t state;
volatile bool irqBMA = false;
volatile bool irqButton = false;

// Distance, step count 
float distance = 0;
uint32_t step_count = 0;

void readFileToBL(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        //Serial.write(file.read());
        SerialBT.println(file.read()); 
    }
    file.close();
}

void init_hike_watch()
{
    // LittleFS
    if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }

    // GPS
    watch->trunOnGPS();
    watch->gps_begin();
    gps = watch->gps;

    // Stepcounter
    // Configure IMU
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    sensor->accelConfig(cfg);
    sensor->enableAccel();
    
    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(BMA423_INT1, [] {
        // Set interrupt to set irq value to 1
        irqBMA = 1;
    }, RISING); //It must be a rising edge

    // Enable BMA423 step count feature
    sensor->enableFeature(BMA423_STEP_CNTR, true);

    // Reset steps
    sensor->resetStepCounter();

    // Turn on step interrupt
    sensor->enableStepCountInterrupt();

    // Side button
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irqButton = true;
    }, FALLING);

    //!Clear IRQ unprocessed first
    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    watch->power->clearIRQ();

    return;
}

void sendSession()
{
    // Read session and send it via SerialBT
}

void checkSessionStored()
{
    // Check if session was stored

}

void calculateDistance()
{
    // Calculate distance btw 2 points
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
    
    init_hike_watch();

    state = 1;

    SerialBT.begin("Hiking Watch");
}


void loop()
{
    watch->gpsHandler();

    switch (state)
    {
    case 1:
    {
        /* Initial stage */
        watch->tft->fillScreen(TFT_BLACK);
        watch->tft->setTextFont(4);
        watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);
        watch->tft->drawString("Hiking Watch",  45, 25, 4);
        watch->tft->drawString("Press button", 50, 80);
        watch->tft->drawString("to start session", 40, 110);

        bool sessionStored = false;
        bool sessionSent = false;
        // Check if we have a stored session

        // If we have a session
        // Print on the watch that we sync and download session to hub
        
        Serial.println(state);

        while (1)
        {
            /* code */
            /* Bluetooth sync function */
            if (SerialBT.available())
            {
                char incomingChar = SerialBT.read();
                if (incomingChar == 'c' and sessionStored and not sessionSent){
                    // Send session via bluetooth
                    sendSession();
                    // Send connection termination char
                    SerialBT.write('c');
                    sessionSent = true;
                }
                // TODO verify if receive char is r
                if (sessionSent && incomingChar == 'r')
                {
                    Serial.println("Session successfuly send via BT");
                }
            }

            /*      IRQ     */
            if (irqButton) {
                irqButton = false;
                watch->power->readIRQ();
                if (state == 1)
                {
                    state = 2;
                }
                watch->power->clearIRQ();
            }
            if (state == 2) {
                break;
            }
        }
        break;
    }
    case 2:
    {
        /* Hiking session initalisation */
        Serial.println(state);

        // Clear screen
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Initializing watch", 25, 100);

        // Do the initializations
        delay(1000); 

        distance = 0;
        step_count = 0;

        // Clear screen
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);

        state = 3;

        break;
    }
    case 3:
    {
        /* Hiking session ongoing */
        Serial.println(state);

        // Reset steps count
        sensor->resetStepCounter();

        // Setup
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Starting hike", 45, 100);
        delay(1000);
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        
        watch->tft->setCursor(45, 40);
        watch->tft->print("Step Int: 0");

        watch->tft->setCursor(45, 70);
        watch->tft->print("Step API: 0");

        watch->tft->setCursor(45, 100);
        watch->tft->print("Distance: 0 km");

        last = millis();

        // Lat long
        float lat = 0;
        float lng = 0;
        float past_lat = 0;
        float past_lng = 0;
        bool coord_printed = false;

        while (1)
        {   
            /* GPS */
            // Get position
            if (gps->location.isUpdated()) {
                updateTimeout = millis();
                lat = gps->location.lat();
                lng = gps->location.lng();

                watch->tft->setCursor(45, 130);
                watch->tft->print("Valid GPS");
                watch->tft->setCursor(45, 160);
                watch->tft->print(lat);
                watch->tft->setCursor(45, 190);
                watch->tft->print(lng);

            } else {
                if (millis() - updateTimeout > 3000) {
                    updateTimeout = millis();

                    Serial.println("Invalid location");

                    watch->tft->setCursor(45, 130);
                    watch->tft->print("Invalid GPS");

                }
            }

            if (lat != past_lat && lng != past_lng)
            {
                Serial.print("LAT: ");Serial.print(lat);
                Serial.print(" LNG: ");Serial.print(lng);
                Serial.println();

                past_lat = lat;
                past_lng = lng;
            }

            // If the coord are the same drop them 
            // else save to coord.txt

            // Calculate distance

            // Add to current distance and display it to user


            /* Step counter */
            if (irqBMA) {
                irqBMA = false;
                bool  rlst;
                do {
                    // Read the BMA423 interrupt status,
                    // need to wait for it to return to true before continuing
                    rlst =  sensor->readInterrupt();
                } while (!rlst);

                // Check if it is a step interrupt
                if (sensor->isStepCounter()) {
                    // incremement step count
                    step_count++;
                    // TODO
                    // Remove the step count - fill black rect

                    // Get step data from register
                    watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);
                    watch->tft->setCursor(45, 40);
                    watch->tft->print("Step Int: ");
                    watch->tft->print(step_count);

                    watch->tft->setCursor(45, 70);
                    watch->tft->print("Step API: ");
                    watch->tft->print(sensor->getCounter());
                }
            }

            /*      IRQ     */
            if (irqButton) {
                irqButton = false;
                watch->power->readIRQ();
                if (state == 3)
                {
                    state = 4;
                }
                watch->power->clearIRQ();
            }
            if (state == 4) {
                break;
            }
                
        }
        break;
    }
    case 4:
    {
        // Saving id, steps and distance in different files 
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Saving hike", 25, 100);
        
        char buffer[30];
        itoa(step_count, buffer, 10);
        Serial.print("Buffer: "); Serial.println(buffer);
        writeFile(LITTLEFS, "/steps.txt", buffer);
        
        Serial.println(state);

        while (1)
        {
            if (SerialBT.available()){
                char incomingChar = SerialBT.read();
                Serial.println(incomingChar);
            }    
            /*      IRQ     */
            if (irqButton) {
                irqButton = false;
                watch->power->readIRQ();
                if (state == 4)
                {
                    state = 5;
                }
                watch->power->clearIRQ();
            }
            if (state == 5) {
                break;
            }
        }
        
        break;
    }

    case 5:
    {
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("State 5", 25, 100);

        Serial.println(state);


        while (1)
        {
            /*      IRQ     */
            if (irqButton) {
                irqButton = false;
                watch->power->readIRQ();
                if (state == 5)
                {
                    state = 6;
                }
                watch->power->clearIRQ();
            }
            if (state == 6) {
                break;
            }
        }

        break;
    }
    case 6:
    {
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("State 6", 25, 100);

        Serial.println(state);

        while (1)
        {
            // After finish sync or IRQ, go to fist state

            /*      IRQ     */
            if (irqButton) {
                irqButton = false;
                watch->power->readIRQ();
                if (state == 6)
                {
                    state = 1;
                }
                watch->power->clearIRQ();
            }
            if (state == 1) {
                break;
            }
        }

        break;

    }
    default:
        break;
    }
}