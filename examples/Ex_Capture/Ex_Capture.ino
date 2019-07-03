#include "ESPino32CAM.h"
ESPino32CAM cam;
void setup() {
  Serial.begin(2000000);
  Serial.println("\r\nESPino32CAM");
  if (cam.init() != ESP_OK)
  {
    cam.printDebug(F("Init Fail"));
    while (1);
  }
  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_QVGA);
}

void loop()
{
  unsigned long pv_time  = millis();
  camera_fb_t *fb = cam.capture();
  if (!fb)
  {
    cam.printDebug("Camera capture failed");
    return;
  }
  Serial.write(fb->buf, fb->len);
  cam.printfDebug("Frame %d Per Second",1000/(millis()-pv_time));
  cam.clearMemory(fb);
}
