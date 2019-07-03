#include "ESPino32CAM.h"
#include "ESPino32CAM_LineNotify.h"
const char* ssid     = ""; 
const char* password = "";
String token = "";
ESPino32CAM cam;
LineNotify line;
#define Button 0
void setup() {
  Serial.begin(115200);
  Serial.println("\r\nESPino32CAM");
  if (cam.init() != ESP_OK)
  {
    cam.printDebug(F("Init Fail"));
    while (1);
  }
  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_VGA);
  s->set_whitebal(s,true);
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\r\nConnected");
  line.authenKey(token);
  line.setUTC(7);
}
void loop()
{
  camera_fb_t *fb;
  String msg;
  if(!digitalRead(Button))
  {
    lineResp ret;
    msg = "ทดสอบ ส่งข้อความ สติ๊กเกอร์ และ รูปภาพจากกล้อง ESPIno32CAM";
    Serial.println("Send Message,Sticker and Image: "+msg);
    Serial.println("Capture");
    fb = cam.capture();
    ret = line.send(msg,4,275,fb->buf, fb->len);
	cam.clearMemory(fb);
    if(ret.status)
    {
      Serial.printf("API Called %d/%d per hour\r\n",ret.remaining,ret.limit);
      Serial.printf("Send Image %d/%d per hour\r\n",ret.imageremaining,ret.imagelimit);
      Serial.printf("The time when the limit is reset %d(UTC epoch seconds) , ",ret.reset);
      Serial.println(ret.reset_time);
    }
    else
      Serial.println(ret.response);
   Serial.println();
  } 
}
