#define TFT_Class GxTFT
#include <GxTFT.h>
#include <GxIO/STM32MICRO/GxIO_STM32F407ZET6_FSMC/GxIO_STM32F407ZET6_FSMC.h>
#include <GxCTRL/GxCTRL_ILI9341/GxCTRL_ILI9341.h>

GxIO_STM32F407ZET6_FSMC io;
GxCTRL_ILI9341 controller(io);
TFT_Class tft(io, controller, 320, 240);

#define HALL_PIN PA1
#define CENTER_X 160
#define CENTER_Y 140
#define RADIUS 90
#define NEEDLE_LENGTH 75
#include <math.h>

volatile uint16_t pulseCount = 0;
unsigned long lastUpdate = 0;
float startAngle = 180.0f;
float lastAngle = -1;
int rpm = 0;

void onHallPulse() {
  pulseCount++;
}


void drawGaugeBackground() {
  tft.fillScreen(TFT_BLACK);

  // Skalenstriche alle 2000 rpm (10 Schritte)
  for (int i = 0; i <= 10; i++) {
   /* float angle = 180.0f - i * 27.0f;  // 270° Bereich in 10 Schritte
    float angleRad = angle * M_PI / 180.0f;
*/
     float angle = startAngle - i * 27.0f;
    float angleRad = angle * M_PI / 180.0f;
    
    int x0 = CENTER_X + (RADIUS - 10) * cos(angleRad);
    int y0 = CENTER_Y - (RADIUS - 10) * sin(angleRad);
    int x1 = CENTER_X + RADIUS * cos(angleRad);
    int y1 = CENTER_Y - RADIUS * sin(angleRad);

    tft.drawLine(x0, y0, x1, y1, TFT_WHITE);

    // Beschriftung
    char buf[6];
    sprintf(buf, "%d", i * 2000);
    int xt = CENTER_X + (RADIUS - 30) * cos(angleRad) - 10;
    int yt = CENTER_Y - (RADIUS - 30) * sin(angleRad) - 7;
    tft.setCursor(xt, yt);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.print(buf);
  }

  // Optional: Kreisrand
  tft.drawCircle(CENTER_X, CENTER_Y, RADIUS, TFT_WHITE);
}

void drawNeedle(float angleDeg) {
  // Vorherigen Zeiger löschen
  if (lastAngle >= 0) {
    float radLast = lastAngle * M_PI / 180.0f;
    int xOld = CENTER_X + NEEDLE_LENGTH * cos(radLast);
    int yOld = CENTER_Y - NEEDLE_LENGTH * sin(radLast);
    tft.drawLine(CENTER_X, CENTER_Y, xOld, yOld, TFT_BLACK);
  }

  float angleRad = angleDeg * M_PI / 180.0f;
  int xNew = CENTER_X + NEEDLE_LENGTH * cos(angleRad);
  int yNew = CENTER_Y - NEEDLE_LENGTH * sin(angleRad);
  tft.drawLine(CENTER_X, CENTER_Y, xNew, yNew, TFT_RED);

  lastAngle = angleDeg;
}

void drawRPM(int rpm) {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(100, 10);
  tft.print("RPM: ");
  tft.print(rpm);
  tft.print("    ");  // zum Löschen alter Werte
}

void setup() {
  tft.init();
  tft.fillScreen(TFT_BLACK);
  drawGaugeBackground();
   
  pinMode(HALL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), onHallPulse, RISING);

  lastUpdate = millis();
}

void loop() {
 /* // Test: RPM simulieren von 0 bis 20000
  rpm += 500;
  if (rpm > 20000) rpm = 0;

  // Winkel berechnen (Start bei 135°, dann 270° gegen Uhrzeigersinn)
  float angle = 135.0f - ((float)rpm / 20000.0f) * 270.0f;
*/
 unsigned long now = millis();
  if (now - lastUpdate >= 1000) {
    noInterrupts();
    uint16_t pulses = pulseCount;
    pulseCount = 0;
    interrupts();

    rpm = pulses * 60;  // 1 Impuls = 1 Umdrehung
    //rpm = (pulseCount * 60); // impulseProUmdrehung;

    /*float angle = 135.0f - ((float)rpm / 20000.0f) * 270.0f;
    if (angle < -135.0f) angle = -135.0f;
    if (angle > 135.0f) angle = 135.0f;
*/
  float angle = startAngle - ((float)rpm / 20000.0f) * 270.0f;
    if (angle < (startAngle - 270.0f)) angle = startAngle - 270.0f;
    if (angle > startAngle) angle = startAngle;

  drawNeedle(angle);
  drawRPM(rpm);
   lastUpdate = now;
  }
  delay(100);
}
