#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SVineeth182-project-1_inferencing.h>  // Edge Impulse model

// OLED parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Built-in LED and Built-in Buzzer
#define LED_PIN 2         // Built-in LED (standard ESP32)
#define BUZZER_PIN 4      // Built-in Buzzer (try GPIO4 first)

float features[3];        // ML model input features

unsigned long last_read_time = 0;
const unsigned long READ_INTERVAL = 30000; // 30 seconds

// Signal getter for Edge Impulse
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(); // Default SDA/SCL

  // Setup OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Health Monitor Ready");
  display.display();
  delay(2000);

  // Setup LED and Buzzer pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  if (millis() - last_read_time >= READ_INTERVAL) {
    last_read_time = millis();

    // Simulate Sensor Readings
    float hr = random(60, 110);
    float spo2 = random(88, 100);
    float temp = random(360, 380) / 10.0;

    Serial.print("HR: "); Serial.print(hr);
    Serial.print(" | SpO2: "); Serial.print(spo2);
    Serial.print(" | Temp: "); Serial.println(temp);

    features[0] = hr;
    features[1] = spo2;
    features[2] = temp;

    signal_t signal;
    signal.total_length = 3;
    signal.get_data = &raw_feature_get_data;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res != EI_IMPULSE_OK) {
      Serial.println("Error running classifier");
      return;
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);

    if (result.classification[0].value < 0.8) {
      Serial.println("Prediction: Normal");
      display.println("Normal");
      digitalWrite(LED_PIN, LOW);      // LED OFF
      digitalWrite(BUZZER_PIN, LOW);   // Buzzer OFF
    }
    else if (result.classification[1].value < 0.8) {
      Serial.println("Prediction: Abnormal!");
      display.println("Abnormal");

      // Activate LED and Buzzer
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);  // Short beep
      digitalWrite(BUZZER_PIN, LOW);
      delay(300);  // Pause
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);  // Short beep
      digitalWrite(BUZZER_PIN, LOW);
    }

    display.display();
  }
}
