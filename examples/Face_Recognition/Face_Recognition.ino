#include "ESPino32Cam.h"
ESPino32Cam cam;
static mtmn_config_t mtmn_config = {0};
bool is_enrolling = false;
static face_id_list id_list = {0};
#define BUTTON 0
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
  mtmn_config.min_face = 160;
  mtmn_config.pyramid = 0.7;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 4;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.4;
  mtmn_config.o_threshold.candidate_number = 1;
  face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
}

void loop()
{
  capture();
  if(digitalRead(BUTTON)==0)
     is_enrolling = true;
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
    run_face_recognition(image_matrix, net_boxes);
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
static int run_face_recognition(dl_matrix3du_t *image_matrix, box_array_t *net_boxes){
    dl_matrix3du_t *aligned_face = NULL;
    int matched_id = 0;
    aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
    if(!aligned_face){
        Serial.println("Could not allocate face recognition buffer");
        return matched_id;
    }
    if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK){
        if (is_enrolling)
        {
            int8_t left_sample_face = enroll_face(&id_list, aligned_face);
            cam.rgb_printf(image_matrix, FACE_COLOR_CYAN, "ID[%u] Sample[%u]", id_list.tail, ENROLL_CONFIRM_TIMES - left_sample_face);
            if (left_sample_face == 0){
                is_enrolling = false;
                Serial.printf("Enrolled Face ID: %d\n", id_list.tail);
            }
        } else {
            matched_id = recognize_face(&id_list, aligned_face);
            if (matched_id >= 0) {
                Serial.printf("Match Face ID: %u\n", matched_id);
                cam.rgb_printf(image_matrix, FACE_COLOR_GREEN, "Hello Subject %u", matched_id);
            } else {
                Serial.println("No Match Found");
                cam.rgb_print(image_matrix, FACE_COLOR_RED, "Intruder Alert!");
                matched_id = -1;
            }
        }
    } else {
        Serial.println("Face Not Aligned");
        cam.rgb_print(image_matrix, FACE_COLOR_YELLOW, "Human Detected");
    }

    dl_matrix3du_free(aligned_face);
    return matched_id;
}
