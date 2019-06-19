#include "ESPino32Cam.h"
ESPino32Cam cam;
static mtmn_config_t mtmn_config = {0};
void setup() {
  Serial.begin(1000000);
  Serial.println(F("Initial Camera :"));
  if (cam.init() != ESP_OK)
  {
    Serial.println(F("Fail"));
    while (1);
  }
  Serial.println(F("OK"));
  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_whitebal(s, true);
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
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  if (!image_matrix)
  {
    esp_camera_fb_return(fb);
    Serial.println("dl_matrix3du_alloc failed");
    return ESP_FAIL;
  }

  size_t out_len;
  out_len = fb->width * fb->height * 3;
 
  bool s = fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
  if (!s)
  {
    dl_matrix3du_free(image_matrix);
    Serial.println("to rgb888 failed");
    return ESP_FAIL;
  }
  esp_camera_fb_return(fb);
  box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);
  if (net_boxes)
  {
    cam.draw_face_boxes(image_matrix, net_boxes);
    free(net_boxes->box);
    free(net_boxes->landmark);
    free(net_boxes);
  }
  uint8_t *jpgout;
  size_t  jpglen;
  s = fmt2jpg(image_matrix->item, out_len, fb->width, fb->height, PIXFORMAT_RGB888, 90, &jpgout, &jpglen);
  if (s)
  {
    Serial.write(jpgout, jpglen);
  }
  free(jpgout);
  dl_matrix3du_free(image_matrix);
}
