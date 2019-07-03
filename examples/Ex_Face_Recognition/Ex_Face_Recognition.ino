#include "ESPino32CAM.h"
ESPino32CAM cam;
#define BUTTON 0
static mtmn_config_t mtmn_config = {0};
bool is_enrolling = false;
static face_id_list id_list = {0};
void setup() {
  Serial.begin(2000000);
  cam.printDebug("\r\nESPino32CAM");  
  if (cam.init() != ESP_OK)
  {
    cam.printDebug(F("Fail"));
    return;
  }
  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_QVGA);
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.7;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 4;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.4;
  mtmn_config.o_threshold.candidate_number = 1;
  cam.faceIDInit(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
}
void loop()
{
  if (digitalRead(BUTTON) == 0)
  {
    is_enrolling = true;
  }
  camera_fb_t *fb = cam.capture();
  if (!fb)
  {
    cam.printDebug("Camera capture failed\r\n");
    return;
  }
  dl_matrix3du_t *rgb888;
  if (cam.jpg2rgb(fb,&rgb888))
  {
    cam.clearMemory(fb);
    box_array_t *net_boxes = cam.faceDetect(rgb888, &mtmn_config);
    if (net_boxes)
    {
      cam.drawFaceBoxes(rgb888, net_boxes, true);
      dl_matrix3du_t *aligned_face;
      if ( cam.alignFace(rgb888, net_boxes, &aligned_face) == ESP_OK)
      {
        if (is_enrolling)
        {
            int8_t left_sample_face = enroll_face(&id_list, aligned_face);
            cam.rgbPrintf(rgb888, FACE_COLOR_YELLOW, "ID[%u] Sample[%u]", id_list.tail, ENROLL_CONFIRM_TIMES - left_sample_face);
            cam.printDebug("ID "+String(id_list.tail)+"Sample "+String(ENROLL_CONFIRM_TIMES - left_sample_face));
            if (left_sample_face == 0){
                is_enrolling = false;
                cam.rgbGoto(10,50);
                cam.rgbPrintf(rgb888, FACE_COLOR_YELLOW,"Enrolled Face ID: %d\n", id_list.tail);
            }
            if (left_sample_face == -2){
                is_enrolling = false;
                cam.rgbGoto(10,50);
                cam.rgbPrintf(rgb888, FACE_COLOR_YELLOW,"Enrolled Fail\n", id_list.tail);
            }
             cam.rgbGoto(0,0);
        }
        else
        {
          int matched_id = cam.recognizeFace(&id_list, aligned_face);
          if (matched_id >= 0) 
          {
            cam.printDebug("Match Face ID: "+String(matched_id));
            cam.rgbPrintf(rgb888, FACE_COLOR_GREEN, "Hello Subject %u", matched_id);
          } 
          else 
          {
            cam.rgbPrintf(rgb888, FACE_COLOR_RED, "Intruder Alert!");
            cam.printDebug("Intruder Alert!");
          }
        }
      }
      cam.clearMemory(aligned_face);
      cam.clearMemory(net_boxes);
    }
  }
  uint8_t *jpgout;
  size_t  jpglen;
  if (cam.rgb2jpg(rgb888,&jpgout, &jpglen))
    Serial.write(jpgout, jpglen);
  cam.clearMemory(jpgout);
  cam.clearMemory(rgb888);
}
