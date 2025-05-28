#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include <DHT.h>

#define DHTPIN 4      // Pin where DHT11 is connected
#define DHTTYPE DHT11 // Define the DHT type
#define LDR_PIN 34    // LDR Pin
#define MOISTURE_PIN 35 // Moisture sensor pin
#define FAN_PIN 12    // Pin connected to MOSFET gate for fan speed control
#define LED1_PIN 13   // LED 1 control pin
#define LED2_PIN 14   // LED 2 control pin
#define PUMP_PIN 15   // Relay control pin for water pump

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

unsigned long pumpOnTime = 0;  // Store the time when pump was turned on
bool pumpActive = false;       // To track if the pump is already on

void setup(){
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_PAGEADDR, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  
  // Display the title at the start
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Smart Greenhouse");
  display.setCursor(0, 30);
  display.print("Automation System");
  display.display();
  
  // Wait for 3 seconds before proceeding
  delay(3000);

  // Set pin modes
  pinMode(LDR_PIN, INPUT);
  pinMode(MOISTURE_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);    // Set fan pin as output for PWM control
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  
 
  digitalWrite(PUMP_PIN, LOW);
}

void loop(){
  // Read temperature and humidity from DHT11
  float temp = dht.readTemperature();
  //Serial.println(temp);
  float humidity = dht.readHumidity();
  //Serial.println(humidity);

  // Read light intensity from LDR
  int lightLevel = analogRead(LDR_PIN);
  int lightPercent = map(lightLevel, 4095, 0, 0, 100);  // Convert to percentage

  // Read moisture level
  int moistureLevel = analogRead(MOISTURE_PIN);
  //Serial.println(moistureLevel);
  int moisturePercent = map(moistureLevel, 4095, 0, 0, 100);  // Convert to percentage

  // Display the readings on the OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temp);
  display.print(" C");
  display.setCursor(0, 10);
  display.print("Humidity: ");
  display.print(humidity);
  display.print(" %");
  display.setCursor(0, 20);
  display.print("Light: ");
  display.print(lightPercent);
  display.print(" %");
  display.setCursor(0, 30);
  display.print("Moisture: ");
  display.print(moisturePercent);
  display.print(" %");
  display.display();
  
  // Control the fan speed using PWM
  if (temp > 20) {
    // Map the temperature to a PWM value (0-255)
    int fanSpeed = map(temp, 20, 40, 0, 255); // You can adjust the temperature range as needed
    fanSpeed = constrain(fanSpeed, 0, 255);   // Ensure the speed is within valid PWM range
    analogWrite(FAN_PIN, fanSpeed);           // Apply PWM signal to the fan
    Serial.println(fanSpeed);
  } else {
    analogWrite(FAN_PIN, 0);                  // Turn off fan if temp <= 30
  }

  // Control LED brightness based on LDR reading
  int ledBrightness = map(lightLevel, 0, 4095, 0, 255);  // Map LDR value to brightness
  analogWrite(LED1_PIN, ledBrightness);
  analogWrite(LED2_PIN, ledBrightness);

  // Control pump based on moisture level (low moisture triggers the pump)
  if (moistureLevel > 1000 && !pumpActive) {
    digitalWrite(PUMP_PIN, HIGH);  // Turn on relay (active low)
    pumpOnTime = millis();       // Start timing for 10 seconds
    pumpActive = true;           // Mark pump as active
    Serial.println("Pump ON");
  }

  // If the pump is active, check if 10 seconds have passed
  if (pumpActive) {
    // Turn off pump after 10 seconds or if moisture level goes below threshold
    if (moistureLevel <= 1000) {
      digitalWrite(PUMP_PIN, LOW); // Turn off relay (active low)
      pumpActive = false;          // Mark pump as inactive
      Serial.println("Pump OFF");
    }
  }

  delay(1000); // Update every second
}
