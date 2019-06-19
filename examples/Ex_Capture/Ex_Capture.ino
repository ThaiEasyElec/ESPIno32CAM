#include "ESPino32CAM.h"
ESPino32CAM cam;
unsigned long pv_time = 0;
void setup() {
  Serial.begin(1000000);
  Serial.println("\r\nESPino32CAM");
  if (cam.init() != ESP_OK)
  {
    Serial.println(F("Fail"));
    while (1);
  }
  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_QVGA);
}

void loop()
{
  pv_time = millis();
  camera_fb_t *fb = cam.capture();
  if (!fb)
  {
    cam.printDebug("Camera capture failed");
    return;
  }
  Serial.write(fb->buf, fb->len);
  cam.printfDebug("Frame %d Per Second",1000/(millis()-pv_time));
  esp_camera_fb_return(fb);
}
