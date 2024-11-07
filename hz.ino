#include <TcBUTTON.h>
#include <TcPINOUT.h>

const int signalPin = 9;
const int totalBuffer = 1;
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
const float frequencyMax = 470.0; // 470Hz
int totalTone = 0;

float previousAverage = 0;
const float maxDeviationPercentage = 20.0; // Max deviation from stable frequency in %

void loop()
{
  start.update();
  buzzerActive.update();

  if (!isStart) return;

  uint32_t currentTime = millis();
  if (currentTime - lastTime500ms >= 100)
  {
    lastTime500ms = currentTime;

    if (stateMachine == STATE_OK || stateMachine == STATE_NG)
    {
      handleBuzzerState();
      return;
    }

    float frequency = readFrequency();

    if (frequency > 0 && isValidFrequency(frequency))
    {
      buffer[bufferIndex++] = frequency;
      if (bufferIndex >= totalBuffer) bufferIndex = 0;

      if (bufferIndex == 0)
      {
        float averageFrequency = calculateAverageFrequency();
        evaluateFrequency(averageFrequency);
        previousAverage = averageFrequency;
      }
    }

    else
    {
      handleNoSignal();
    }
  }
}

void handleBuzzerState()
{
  if (stateMachine == STATE_NG)
  {
    tone(BUZZER_PASSIVE, 2000, 400);
    buzzerActive.on();
  }
  else if (stateMachine == STATE_OK && totalTone > 0)
  {
    totalTone--;
    tone(BUZZER_PASSIVE, 2000, 50);
    delay(100);
  }
}

float readFrequency()
{
  unsigned long highTime = pulseIn(signalPin, HIGH, 500000); // Timeout 500ms
  unsigned long lowTime = pulseIn(signalPin, LOW, 500000);   // Timeout 500ms

  Serial.print("High: ");
  Serial.print(highTime);
  Serial.print(" Low: ");
  Serial.print(lowTime);
  Serial.print(" ");

  if (highTime > 0 && lowTime > 0)
  {
    unsigned long period = highTime + lowTime;
    float frequency = 1000000.0 / period;

    Serial.print("Period: ");
    Serial.print(period);
    Serial.print(" ");
    // Frequency range 0 - 1000 Hz
    if (frequency > 0 && frequency < 1000)
    {
      Serial.print("Frequency: ");
      Serial.print(frequency);
      Serial.println(" Hz");
      countCheck++;
      countNoSignal = 0;
      return frequency;
    }
  }
   Serial.println(" ");
  return -1;
}

bool isValidFrequency(float frequency)
{
  if (previousAverage == 0) return true;

  float deviation = abs(frequency - previousAverage) / previousAverage * 100;
  return deviation <= maxDeviationPercentage;
}

void handleNoSignal()
{
  countNoSignal++;
  if (countNoSignal >= maxCountNoSignal)
  {
    countCheck++;
    Serial.println("No signal detected");
    Serial.println("Reset buffer");
    resetBuffer();
    countNoSignal = maxCountNoSignal;
  }
}

void evaluateFrequency(float averageFrequency)
{
  if (averageFrequency >= frequencyMin && averageFrequency <= frequencyMax)
  {
    Serial.println("OK");
    stateMachine = STATE_OK;
    manageLed(2); // LED GREEN
    buzzerActive.onToggle(2, 200);
    totalTone = 2;
  }
  else
  {
    Serial.println("NG");
    stateMachine = STATE_NG;
    manageLed(1); // LED RED
  }
}

float calculateAverageFrequency()
{
  float sum = 0;
  for (int i = 0; i < totalBuffer; i++) sum += buffer[i];
  float averageFrequency = sum / totalBuffer;
  Serial.print("Average Frequency: ");
  Serial.print(averageFrequency);
  Serial.println(" Hz");
  return averageFrequency;
}

void resetBuffer()
{
  bufferIndex = 0;
  for (int i = 0; i < totalBuffer; i++) buffer[i] = 0;
}

void buzzerActiveOnEventChange(boolean state) {}

void startOnEventChange(boolean state)
{
  isStart = !state;
  Serial.print("Start: ");
  Serial.println(isStart);
  if (isStart)
  {
    stateMachine = STATE_RUNNING;
    manageLed(3); // LED BLUE
    resetBuffer();
    countCheck = 0;
    countNoSignal = 0;
    previousAverage = 0; 
  }
  else
  {
    noTone(BUZZER_PASSIVE);
    buzzerActive.noToggle();
    stateMachine = STATE_WAIT;
    manageLed(0); // LED OFF
  }
}

void manageLed(uint8_t led = 0)
{
  ledRed.off();
  ledGreen.off();
  ledBlue.off();

  switch (led)
  {
    case 1: ledRed.on(); break;
    case 2: ledGreen.on(); break;
    case 3: ledBlue.on(); break;
  }
}
