#include "ESPino32Cam.h"
ESPino32Cam cam;
void setup() {
  Serial.begin(1000000);
  Serial.println(F("Initial Camera :"));
  if(cam.init()!=ESP_OK)
  {
    Serial.println(F("Fail"));
    while(1);
  }
   Serial.println(F("OK"));
   sensor_t *s = cam.sensor();
   s->set_framesize(s, FRAMESIZE_VGA);
   s->set_whitebal(s,true);
   s->set_brightness(s,200);
}

void loop() 
{
 capture();
}
esp_err_t capture()
{
  camera_fb_t * fb = NULL;
  fb = cam.capture();
  if (!fb) 
  {
        Serial.println("Camera capture failed");
        return ESP_FAIL;
  }
  Serial.write(fb->buf,fb->len);
  esp_camera_fb_return(fb);
}
