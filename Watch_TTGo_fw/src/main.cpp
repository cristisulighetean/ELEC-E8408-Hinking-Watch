#include "config.h"

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
TinyGPSPlus *gps = nullptr;

// GPS var
uint32_t last = 0;
uint32_t updateTimeout = 0;

uint32_t sessionId = 10;

volatile uint8_t state;
volatile bool irqBMA = false;
volatile bool irqButton = false;

bool sessionStored = false;
bool sessionSent = false;

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

void sendDataBT(fs::FS &fs, const char * path)
{
    /* Sends data via SerialBT */
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

void sendSessionBT()
{
    // Read session and send it via SerialBT
    watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
    watch->tft->drawString("Sending session", 20, 80);
    watch->tft->drawString("to Hub", 80, 110);

    // Sending session id
    sendDataBT(LITTLEFS, "/id.txt");
    SerialBT.write(';');
    // Sending steps
    sendDataBT(LITTLEFS, "/steps.txt");
    SerialBT.write(';');
    // Sending distance
    sendDataBT(LITTLEFS, "/distance.txt");
    SerialBT.write(';');
    // Sending coordinates
    sendDataBT(LITTLEFS, "/coord.txt");
}

float calculateDistance(gpsData current, gpsData past)
{
    // Calculate distance btw 2 points
    double p1 = pow((current.lat - past.lat), 2);
    double p2 = pow((current.lng - past.lng), 2);
    return (float) sqrt(p1+p2);
}

void saveCoordinatesDummy(void)
{   
    appendFile(LITTLEFS, "/coord.txt", "64.83458747762428,24.83458747762428;");
    appendFile(LITTLEFS, "/coord.txt", "64.83458747762428,24.83458747762428;");
    appendFile(LITTLEFS, "/coord.txt", "64.83458747762428,24.83458747762428;");
    appendFile(LITTLEFS, "/coord.txt", "64.83458747762428,24.83458747762428;");
}

void saveCoordinates(gpsData current)
{
    char buf [100];
    sprintf(buf, "%3.14f,%3.14f;", current.lat, current.lng);
    writeFile(LITTLEFS, "/coord.txt", buf);
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

void deleteSession()
{
    deleteFile(LITTLEFS, "/id.txt");
    deleteFile(LITTLEFS, "/distance.txt");
    deleteFile(LITTLEFS, "/steps.txt");
    deleteFile(LITTLEFS, "/coord.txt");
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

        while (1)
        {
            /* code */
            /* Bluetooth sync function */
            if (SerialBT.available())
            {
                char incomingChar = SerialBT.read();
                if (incomingChar == 'c' and sessionStored and not sessionSent){
                    // Send session via bluetooth
                    sendSessionBT();
                    // Send connection termination char
                    SerialBT.write('\n');
                    sessionSent = true;

                    // Dummy delay
                    delay(1000);
                    // Print new message to user
                    watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
                    watch->tft->drawString("Hiking Watch",  45, 25, 4);
                    watch->tft->drawString("Press button", 50, 80);
                    watch->tft->drawString("to start session", 40, 110);
                }

                if (sessionSent && incomingChar == 'r')
                {
                    // Delete session
                    deleteSession();
                    sessionStored = false;
                    sessionSent = false;
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
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Initializing watch", 25, 100);
        delay(1000); 
        state = 3;
        break;
    }
    case 3:
    {
        /* Hiking session ongoing */

        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Starting hike", 45, 100);
        delay(1000);
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);

        watch->tft->setCursor(45, 70);
        watch->tft->print("Steps: 0");

        watch->tft->setCursor(45, 100);
        watch->tft->print("Distance: 0 km");

        last = millis();

        // Lat long
        gpsData gps_current;
        gpsData gps_past;
        float distance = 0;
        float past_distance = 0;
        uint32_t step_count = 0;
        sessionId += 1;
        sensor->resetStepCounter();

        while (1)
        {   
            watch->gpsHandler();
            /* GPS */
            if (gps->location.isUpdated()) {
                updateTimeout = millis();
                gps_current.lat = gps->location.lat();
                gps_current.lng = gps->location.lng();

                watch->tft->setCursor(45, 130);
                char lat[30]; sprintf(lat, "LAT: %2.6f", gps_current.lat);
                watch->tft->print(lat);

                watch->tft->setCursor(45, 160);
                char lng[30]; sprintf(lng, "LNG: %2.6f", gps_current.lng);
                watch->tft->print(lng);
            } else {
                if (millis() - updateTimeout > 3000) {
                    updateTimeout = millis();
                    watch->tft->setCursor(45, 130);
                    watch->tft->print("No GPS data");
                }
            }

            // Update covered distance & save to file
            if (gps_current.lat != gps_past.lat && gps_current.lng != gps_past.lng)
            {
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
                char buf[20];
                sprintf(buf, "%2.2f", distance);
                watch->tft->print(buf);
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
                    // Get step data from register
                    step_count = sensor->getCounter();
                    watch->tft->setCursor(45, 70);
                    watch->tft->print("Steps: ");
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
                saveIdToFile(sessionId);
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
    default:
        // Restart watch
        ESP.restart();
        break;
    }
}