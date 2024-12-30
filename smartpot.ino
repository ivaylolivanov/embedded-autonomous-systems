#include <math.h>

#define PIN_PHOTO_RESISTOR 0
#define PIN_LCD_ENABLE 2
#define PIN_LCD_READ_WRITE 11
#define PIN_LCD_DATA_INSTRUCTION 12

#define LCD_STATUS_BUSY 0
#define LCD_STATUS_FREE 0

#define LIGHT_SAMPLES_MAX 48

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

long PHOTO_RESISTOR_SAMPLE_PERIOD = 3600000;

// TODO: Consider smaller type - char?
int PIN_LCD_DATA_BUS[] = { 3, 4, 5, 6, 7, 8, 9, 10 };

int LIGHT_EXPOSURE_AVG = 0;
int LIGHT_SAMPLE_INDEX = 0;
// TODO: Consider smaller type - char?
int LIGHT_SAMPLES[LIGHT_SAMPLES_MAX] = {0};
long TIME_PHOTO_RESISTOR_NEXT_READ = 0;

void setup()
{
    Serial.begin(9600);

    LIGHT_EXPOSURE_AVG = 0;
    TIME_PHOTO_RESISTOR_NEXT_READ = 0;
    LIGHT_SAMPLE_INDEX = 0;

    lcdInitialize();
}

void loop()
{
    long time_current = millis();

    if (TIME_PHOTO_RESISTOR_NEXT_READ <= time_current)
    {
        samplePhotoResistor(time_current);
        Serial.println(LIGHT_EXPOSURE_AVG);
    }

    lcdPrintTest();

    // wait for 0.01 sec
    /* delay(SENSOR_SAMPLE_PERIOD); */
}

void samplePhotoResistor(int time_current)
{
    int raw = analogRead(PIN_PHOTO_RESISTOR);
    int remapped = map(raw, 1, 1000, 100, 1);

    if (LIGHT_SAMPLE_INDEX >= LIGHT_SAMPLES_MAX)
        LIGHT_SAMPLE_INDEX = 0;

    LIGHT_SAMPLES[LIGHT_SAMPLE_INDEX++] = remapped;

    int valid_samples = 0;
    float exposure_sum = 0.0f;
    for (int i = 0; i < LIGHT_SAMPLES_MAX; ++i)
    {
        int sample = LIGHT_SAMPLES[i];
        if (!sample)
            continue;

        exposure_sum += sample;
        ++valid_samples;
    }

    LIGHT_EXPOSURE_AVG = round(exposure_sum / valid_samples);

    TIME_PHOTO_RESISTOR_NEXT_READ = time_current + PHOTO_RESISTOR_SAMPLE_PERIOD;
}

void lcdInitialize()
{
    for (int pin = PIN_LCD_ENABLE; pin <= PIN_LCD_DATA_INSTRUCTION; ++pin)
        pinMode(pin, OUTPUT);

    // NOTE: Wait for LcdStartup;
    delay(100);

    // NOTE: select as 8-bit interface, 2-line display, 5x7 character size
    lcdWrite(0x38, false);
    delay(64);

    // NOTE: select as 8-bit interface, 2-line display, 5x7 character size
    lcdWrite(0x38, false);
    delay(50);

    // NOTE: select as 8-bit interface, 2-line display, 5x7 character size
    lcdWrite(0x38, false);
    delay(20);

    // NOTE: Set input mode: auto-increment, no display of shifting
    lcdWrite(0x06, false);
    delay(20);

    // NOTE: display setup: turn on the monitor, cursor on, no flickering
    lcdWrite(0x0E, false);
    delay(20);

    // NOTE: clear the scree, cursor position returns to 0
    lcdWrite(0x01, false);
    delay(100);

    // NOTE: display setup: turn on the monitor, cursor on, no flickering
    lcdWrite(0x80, false);
    delay(20);
}

void lcdWrite(int value, bool is_data)
{
    int pin_start = PIN_LCD_DATA_BUS[0];
    int pin_end = PIN_LCD_DATA_INSTRUCTION;
    if (is_data)
    {
        digitalWrite(PIN_LCD_DATA_INSTRUCTION, HIGH);
        digitalWrite(PIN_LCD_READ_WRITE,       LOW);

        pin_end = PIN_LCD_DATA_BUS[7];
    }

    for (int pin = pin_start; pin <= pin_end; ++pin)
    {
        digitalWrite(pin, value & 01);
        value >>= 1;
    }

    digitalWrite(PIN_LCD_ENABLE, LOW);
    delayMicroseconds(1);
    digitalWrite(PIN_LCD_ENABLE, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_LCD_ENABLE, LOW);
    delayMicroseconds(1);
}

void lcdPrintMessage(char* message)
{
    char character = message[0];
    while (character != '\0')
    {
        lcdWrite(character, true);
        ++message;
        character = message[0];
    }
}

bool lcdGetStatus()
{
    bool status = LCD_STATUS_BUSY;

    digitalWrite(PIN_LCD_DATA_INSTRUCTION, LOW);
    digitalWrite(PIN_LCD_READ_WRITE,       HIGH);

    digitalWrite(PIN_LCD_ENABLE, LOW);
    // NOTE: Makes sure 'Enable' is properly set.
    delayMicroseconds(1);

    digitalWrite(PIN_LCD_ENABLE, HIGH);
    // NOTE: Waits for data.
    delayMicroseconds(1);

    int d7 = digitalRead(PIN_LCD_DATA_BUS[7]);
    if (!d7)
        status = LCD_STATUS_FREE;

    return status;
}

void lcdPrintTest()
{
    // NOTE: Clear the screen, cursor position returns to 0
    lcdWrite(0x01, false);
    delay(10);
    lcdWrite(0x80, false);
    delay(10);
    lcdPrintMessage("Exo, a3 c5M!");
    delay(10);
    // NOTE: set cursor position at second line, second position
    lcdWrite(0xc0+2, false);
    delay(10);
    lcdPrintMessage("Test teing :)");
    delay(5000);
}
