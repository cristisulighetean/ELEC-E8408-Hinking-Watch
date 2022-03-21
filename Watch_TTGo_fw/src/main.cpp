#include "config.h"
#include "BluetoothSerial.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

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

bool sessionStored = false;
bool sessionSent = false;

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

void initHikeWatch()
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

void send_data(fs::FS &fs, const char * path)
{
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        SerialBT.write(file.read());
    }
    file.close();
}

void sendSession()
{
    // Read session and send it via SerialBT
    // Sending the session to the Hub
    watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
    watch->tft->drawString("Sending session", 45, 80);
    watch->tft->drawString("to Hub", 55, 110);

    // Sending session id
    send_data(LITTLEFS, "/id.txt");
    // Sending steps
    send_data(LITTLEFS, "/steps.txt");
    // Sending distance
    send_data(LITTLEFS, "/distance.txt");
    // Sending coordinates
    send_data(LITTLEFS, "/coord.txt");
}

void checkSessionStored()
{
    // Check if session was stored 
    // Check for file

}

float calculateDistance(gpsData current, gpsData past)
{
    // Calculate distance btw 2 points
    double p1 = pow((current.lat - past.lat), 2);
    double p2 = pow((current.lng - past.lng), 2);
    return (float) sqrt(p1+p2);
}

void openCoordinatesFile(void)
{
    writeFile(LITTLEFS, "/coord.txt", "");
}


int saveCoordinates(gpsData current)
{   
    appendFile(LITTLEFS, "/steps.txt", "64.83458747762428,24.83458747762428;");
    appendFile(LITTLEFS, "/steps.txt", "64.83458747762428,24.83458747762428;");
    appendFile(LITTLEFS, "/steps.txt", "64.83458747762428,24.83458747762428;");
    appendFile(LITTLEFS, "/steps.txt", "64.83458747762428,24.83458747762428;");

    return 0;
}

void saveIdToFile(uint16_t id)
{
    char buffer[10];
    itoa(id, buffer, 10);
    writeFile(LITTLEFS, "/id.txt", buffer);
}

void saveStepsToFile(uint32_t step_count)
{
    char buffer[10];
    itoa(step_count, buffer, 10);
    writeFile(LITTLEFS, "/steps.txt", buffer);
}

void saveDistanceToFile(float distance)
{
    char buffer[10];
    itoa(distance, buffer, 10);
    writeFile(LITTLEFS, "/distance.txt", buffer);
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
    
    initHikeWatch();

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
                // Timeout to check received 
                if (sessionSent && incomingChar == 'r')
                {
                    Serial.println("Session successfuly send via BT");
                    // TODO
                    // Delete session
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
        gpsData gps_current;
        gpsData gps_past;
        float distance = 0;
        float past_distance = 0;
        uint32_t step_count = 0;

        // Open coordinates file
        //openCoordinatesFile();

        while (1)
        {   
            /* GPS */
            // Get position
            if (gps->location.isUpdated()) {
                updateTimeout = millis();
                gps_current.lat = gps->location.lat();
                gps_current.lng = gps->location.lng();
            } else {
                if (millis() - updateTimeout > 3000) {
                    updateTimeout = millis();
                    watch->tft->setCursor(45, 130);
                    watch->tft->print("No GPS data");
                }
            }

            // Update past position if different and save to file
            if (gps_current.lat != gps_past.lat && gps_current.lng != gps_past.lng)
            {
                Serial.print("LAT: ");Serial.print(gps_current.lat);
                Serial.print(" LNG: ");Serial.print(gps_current.lng);
                Serial.println();

                distance += calculateDistance(gps_current, gps_past); 

                // Save coordinates to file
                saveCoordinates(gps_current); 

                gps_past.lat = gps_current.lat;
                gps_past.lng = gps_current.lng;
            }
            
            // Display distance
            if (distance != past_distance)
            {
                watch->tft->setCursor(45, 100);
                watch->tft->print("Distance: ");
                watch->tft->print("%2.2f");
                watch->tft->print(" km");

                past_distance = distance;
            }
            
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
                // Save steps and distance
                saveIdToFile(69);
                saveStepsToFile(step_count);
                saveDistanceToFile(distance);
                sessionStored = true;

                break;
            }
        }
        break;
    }
    case 4:
    {
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Ending hike", 45, 100);
        delay(1000);
        state = 1;  
        break;
    }
    case 5:
    {
        // Sending the session to the Hub
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Sending session", 45, 80);
        watch->tft->drawString("to Hub", 55, 110);

        state = 1;
        break;

    }
    default:
        // Restart watch
        ESP.restart();
        break;
    }
}