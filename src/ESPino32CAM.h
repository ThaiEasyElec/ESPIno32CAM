#ifndef _ESPino32Cam_H_
#define _ESPino32Cam_H_


#include "Arduino.h"
#include "esp_camera.h" // camera
#include "fd_forward.h" // face Detect
#include "fr_forward.h" // face Recognition
#include "fb_gfx.h" 
#include "image_util.h"

#include "FS.h"
#include "SPIFFS.h"


// Config Camera Pin 
#define PWDN_GPIO_NUM     -1 
#define RESET_GPIO_NUM     4
#define XCLK_GPIO_NUM      13
#define SIOD_GPIO_NUM     21
#define SIOC_GPIO_NUM     22
#define Y9_GPIO_NUM       34
#define Y8_GPIO_NUM       35
#define Y7_GPIO_NUM       32
#define Y6_GPIO_NUM       25
#define Y5_GPIO_NUM       27
#define Y4_GPIO_NUM       12
#define Y3_GPIO_NUM       14
#define Y2_GPIO_NUM       26
#define VSYNC_GPIO_NUM    36
#define HREF_GPIO_NUM     39
#define PCLK_GPIO_NUM     33

// recognition  define
#define FORMAT_SPIFFS_IF_FAILED true
#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7
#define FACE_ID_PATH "/faceid.dat"

#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)




class ESPino32CAM
{
public:
	ESPino32CAM();
	esp_err_t init();
	camera_config_t config;
	camera_fb_t *capture();
	sensor_t * sensor();
	bool jpg2rgb(camera_fb_t *fb, dl_matrix3du_t **image_matrix);
	bool rgb2jpg(dl_matrix3du_t *rgb888, uint8_t **jpg,size_t *len);
	dl_matrix3du_t *rgb565(dl_matrix3du_t *image);
	size_t grayScale(dl_matrix3du_t *image_888,dl_matrix3du_t **gray);
	box_array_t *faceDetect(dl_matrix3du_t *image_matrix,mtmn_config_t *config);
	int8_t alignFace(dl_matrix3du_t *image_matrix, box_array_t *net_boxes, dl_matrix3du_t **aligned_face);
	void faceIDInit(face_id_list *l, uint8_t size, uint8_t confirm_times);
	bool faceIDInitFlash();
	bool readFaceIDFromFlash(face_id_list *l);
	bool writeFaceIDToFlash(face_id_list *l);
	bool deleteFaceIDinFlash();
	int8_t recognizeFace(face_id_list *l,dl_matrix3du_t *algined_face);
	void drawFaceBoxes(dl_matrix3du_t *image_matrix, box_array_t *boxes,bool landmask);
	void rbgDrawBox(fb_data_t *fb,uint32_t color,int x,int y,int w,int h);
	void rgbGoto(int32_t x, int32_t y);
	void rgbPrint(dl_matrix3du_t *image_matrix, uint32_t color, const char * str);
	int  rgbPrintf(dl_matrix3du_t *image_matrix, uint32_t color, const char *format, ...);
	void clearMemory(dl_matrix3du_t *memory);
	void clearMemory(uint8_t *memory);
	void clearMemory(box_array_t *memory);
	void clearMemory(camera_fb_t *memory);
	void printDebug(String data);
	void printfDebug(const char *format, ...);
	uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
private:
	int rgb_x;
	int rgb_y;
};

size_t grayScale_(dl_matrix3du_t *image_888,dl_matrix3du_t **gray);
size_t gray2gray888(dl_matrix3du_t *gray,dl_matrix3du_t **image_888);







































#endif