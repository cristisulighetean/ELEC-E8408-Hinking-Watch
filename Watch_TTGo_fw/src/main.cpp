#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "BluetoothSerial.h"

// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Bluetooth Serial object
BluetoothSerial SerialBT;

volatile uint8_t state;
volatile bool irqBMA = false;
volatile bool irqButton = false;

TTGOClass *watch;
TFT_eSPI *tft;
BMA *sensor;

// Distance, step count 
float distance = 0;
uint32_t step_count = 0;

void init_step_counter(void)
{
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
}

void init_little_fs()
{
    if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }
}

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

void setup()
{
    Serial.begin(115200);
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();

    //Receive objects for easy writing
    tft = watch->tft;
    sensor = watch->bma;

    watch->tft->fillScreen(TFT_BLACK);
    watch->tft->drawString("Hiking Watch",  25, 50, 4);
    watch->tft->setTextFont(4);
    watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Initalize objects
    init_step_counter();
    init_little_fs();

    state = 1;

    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irqButton = true;
    }, FALLING);

    //!Clear IRQ unprocessed first
    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    watch->power->clearIRQ();

    SerialBT.begin("ESP32");
}


void loop()
{

    switch (state)
    {
    case 1:
    {
        /* Stand-by mode*/
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Start button", 25, 100);
        watch->tft->drawString("to start session", 10, 125);
        /* Initialisations */

        while (1)
        {
            /* code */


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

        // Clear screen
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Initializing watch", 25, 100);

        // Do the initializations
        // Dummy delay
        delay(1000);

        // Reset steps count
        sensor->resetStepCounter(); 

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

        // Setup
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Starting hike", 25, 100);
        delay(1000);
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);

        while (1)
        {   
            /* A non-blocking delay. */

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
                    watch->tft->setCursor(45, 118);
                    watch->tft->print("StepCount:");
                    watch->tft->print(step_count);
                }
            }

            /* GPS */

            /* Distance calculation */

            /* Saving to file */

            delay(20);

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

        // Step counter code
        // Save step count
        
        // Get gps location
        // Save gps location

        // Calculate current distance
        
        break;
    }
    case 4:
    {
        // Saving step, distance, coordinates 
        // Close file
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Save state", 25, 100);

        // Save step counter to file
        listDir(LITTLEFS, "/", 3);
        
        char buffer[30];
        itoa(step_count, buffer, 10);
        Serial.print("Buffer: "); Serial.println(buffer);
        writeFile(LITTLEFS, "/steps.txt", buffer);
        
        while (1)
        {
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
        /* Sending session via bluetooth */
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        watch->tft->drawString("Sync", 25, 100);

        readFileToBL(LITTLEFS, "/steps.txt");
        SerialBT.println(step_count); 
        

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
        /* Bluetooth sync */
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