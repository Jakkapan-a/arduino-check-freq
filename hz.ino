#include <TcBUTTON.h>
#include <TcPINOUT.h>

const int signalPin = 9;
const int totalBuffer = 5;
float buffer[totalBuffer];
int bufferIndex = 0;
int countNoSignal = 0;
const int maxCountNoSignal = 4;

#define BUZZER_ACTIVE 4
void buzzerActiveOnEventChange(boolean state);
TcPINOUT buzzerActive(BUZZER_ACTIVE, buzzerActiveOnEventChange, false);

#define BUZZER_PASSIVE 5
#define START_PIN 7

void startOnEventChange(boolean state);
TcBUTTON start(START_PIN, startOnEventChange, false);

#define LED_RED_PIN 10
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 12
TcPINOUT ledRed(LED_RED_PIN, false);
TcPINOUT ledGreen(LED_GREEN_PIN, false);
TcPINOUT ledBlue(LED_BLUE_PIN, false);

bool isStart = false;
enum STATE_MACHINE
{
  STATE_WAIT,
  STATE_RUNNING,
  STATE_OK,
  STATE_NG,
};

STATE_MACHINE stateMachine = STATE_WAIT;
void manageLed(uint8_t);
void setup()
{
  Serial.begin(9600);

  pinMode(signalPin, INPUT);
  pinMode(BUZZER_PASSIVE, OUTPUT);

  buzzerActive.onToggle(1, 200);
}

uint32_t lastTime500ms = 0;
int countCheck = 0;
const int maxCountCheck = 2;
const float frequencyMin = 350.0; // 350Hz
const float frequencyMax = 415.0; // 415Hz
int totalTone = 0;
void loop()
{
  start.update();
  buzzerActive.update();

  if (!isStart)
  {
    return;
  }

  uint32_t currentTime = millis();
  if (currentTime - lastTime500ms >= 100)
  {

    lastTime500ms = currentTime;

    if (stateMachine == STATE_OK || stateMachine == STATE_NG)
    {
      if (stateMachine == STATE_NG)
      {
        tone(BUZZER_PASSIVE, 2000, 400);
        buzzerActive.on();
      }else
      if(stateMachine == STATE_OK && totalTone > 0){
        totalTone--;
        tone(BUZZER_PASSIVE, 2000, 100);
        delay(200);
      }
      return;
    }
    unsigned long highTime = pulseIn(signalPin, HIGH, 500000); // Timeout 1 second
    unsigned long lowTime = pulseIn(signalPin, LOW, 500000);   // Timeout 1 second
    float averageFrequency = 0;

    if (highTime > 3 && lowTime > 3)
    {
      unsigned long period = highTime + lowTime;
      // calculate frequency in Hz =[ f = 1 / T โดย T = period / 1,000,000 (microsecond) ]
      float frequency = 1000000.0 / period;

      if (frequency > 0 && frequency < 10000)
      { // Check normal frequency range (0 - 10,000 Hz)

        // Add frequency to buffer
        buffer[bufferIndex] = frequency;
        bufferIndex++;
        if (bufferIndex >= totalBuffer)
        {
          bufferIndex = 0;
        }

        if (bufferIndex == 0)
        {
          // Calculate average frequency
          float sum = 0;
          for (int i = 0; i < totalBuffer; i++)
          {
            sum += buffer[i];
          }
          averageFrequency = sum / totalBuffer;
          Serial.print("Average Frequency: ");
          Serial.print(averageFrequency);
          Serial.println(" Hz");
          countCheck++;
        }
      }
      else
      {
        Serial.println("Frequency out of range");
      }
      countNoSignal = 0;
    }
    else
    {
      countNoSignal++;
      if (countNoSignal >= maxCountNoSignal)
      {
        countCheck++;
        Serial.println("No signal detected");
        Serial.println("Reset buffer");
        bufferIndex = 0;
        for (int i = 0; i < totalBuffer; i++)
        {
          buffer[i] = 0;
        }
        countNoSignal = maxCountNoSignal;
      }
    }

    if (countCheck >= maxCountCheck)
    {
      countCheck = 0;
      if (averageFrequency >= frequencyMin && averageFrequency <= frequencyMax)
      {
        Serial.println("OK");
        stateMachine = STATE_OK;
        // LED GREEN
        manageLed(2);
        buzzerActive.onToggle(2, 200);
        totalTone = 2;
      }
      else
      {
        Serial.println("NG");
        stateMachine = STATE_NG;
        // LED RED
        manageLed(1);
      }
    }
  }
  else if (currentTime < lastTime500ms)
  {
    lastTime500ms = currentTime;
  }
}

void buzzerActiveOnEventChange(boolean state)
{
}

void startOnEventChange(boolean state)
{
  isStart = !state;
  Serial.print("Start: ");
  Serial.println(isStart);
  if (isStart)
  {
    // buzzerActive.onToggle(1, 200);
    // tone(BUZZER_PASSIVE, 2000, 200);
    stateMachine = STATE_RUNNING;
    // LED BLUE
    manageLed(3);

    // Reset buffer
    bufferIndex = 0;
    for (int i = 0; i < totalBuffer; i++)
    {
      buffer[i] = 0;
    }

    // Reset count check
    countCheck = 0;

    // Reset count no signal
    countNoSignal = 0;
  }
  else
  {
    noTone(BUZZER_PASSIVE);
    buzzerActive.noToggle();
    stateMachine = STATE_WAIT;

    // LED OFF
    manageLed(0);
  }
}

void manageLed(uint8_t led = 0)
{
  switch (led)
  {
  case 1:
    ledRed.on();
    ledGreen.off();
    ledBlue.off();
    break;
  case 2:
    ledRed.off();
    ledGreen.on();
    ledBlue.off();
    break;
  case 3:
    ledRed.off();
    ledGreen.off();
    ledBlue.on();
    break;
  default:
    ledRed.off();
    ledGreen.off();
    ledBlue.off();
    break;
  }
}