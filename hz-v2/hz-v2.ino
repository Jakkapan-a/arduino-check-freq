#include <FreqMeasure.h>

void setup() {
  Serial.begin(9600);
  pinMode(13, INPUT_PULLUP);
  FreqMeasure.begin();

  Serial.println("frequency START");
}

double sum=0;
int count=0;

void loop() {
      //  Serial.println("frequency START");


  if (FreqMeasure.available()) {
    // average several reading together
    sum = sum + FreqMeasure.read();
    count = count + 1;
    if (count > 30) {
      float frequency = FreqMeasure.countToFrequency(sum / count);
      
      Serial.print("Frequency: ");
      Serial.print(frequency);
      Serial.println(" Hz");
      sum = 0;
      count = 0;
    }
  }
}


// #include <TimerOne.h>

// volatile unsigned long pulseCount = 0;

// void setup() {
//   pinMode(13, INPUT_PULLUP);
//   Serial.begin(9600);
  
//   Timer1.initialize(1000000); // ตั้งเวลา 1 วินาที
//   Timer1.attachInterrupt(countFrequency); // เรียกฟังก์ชันทุก 1 วินาที
//   attachInterrupt(digitalPinToInterrupt(13), countPulses, RISING);
// }

// void loop() {
//   // รอ Timer1 เรียกใช้ countFrequency ทุก 1 วินาที
// }

// void countPulses() {
//   pulseCount++;
// }

// void countFrequency() {
//   Serial.print("Frequency: ");
//   Serial.print(pulseCount);
//   Serial.println(" Hz");
//   pulseCount = 0; // เริ่มนับใหม่
// }

