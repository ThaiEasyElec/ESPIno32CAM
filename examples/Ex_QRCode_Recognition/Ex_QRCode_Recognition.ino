#include "ESPino32CAM.h"
#include "ESPino32CAM_QRCode.h"
ESPino32CAM cam;
ESPino32QRCode qr;
#define BUTTON_QR 0
void setup() {
  Serial.begin(2000000);
  Serial.println("\r\nESPino32CAM");
  if (cam.init() != ESP_OK)
  {
    Serial.println(F("Fail"));
    while (1);
  }
  qr.init(&cam);
  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_VGA);
  s->set_whitebal(s,true);
}
void loop()
{
  camera_fb_t *fb = cam.capture();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }
  Serial.write(fb->buf, fb->len);
  dl_matrix3du_t *image_rgb;
  if(cam.jpg2rgb(fb,&image_rgb))
  {
     cam.clearMemory(fb);
     if(!digitalRead(BUTTON_QR))
     {
        cam.printDebug("\r\nQR Read:");
        qrResoult res = qr.recognition(image_rgb);
       if(res.status)
       {
          cam.printDebug("");
          cam.printfDebug("Version: %d", res.version);
          cam.printfDebug("ECC level: %c",res.eccLevel);
          cam.printfDebug("Mask: %d", res.mask);
          cam.printDebug("Data type: "+ qr.dataType(res.dataType));
          cam.printfDebug("Length: %d",res.length);
          cam.printDebug("Payload: "+res.payload);
       }
       else
          cam.printDebug("FAIL");
     }
  }
  cam.clearMemory(image_rgb);  
}
