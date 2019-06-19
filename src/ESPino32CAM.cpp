
#include "ESPino32CAM.h"

ESPino32CAM::ESPino32CAM()
{
	rgb_x = 0;
	rgb_y = 0;
}
esp_err_t ESPino32CAM::init()
{
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk = XCLK_GPIO_NUM;
	config.pin_pclk = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;
	  if(psramFound()){
		config.frame_size = FRAMESIZE_UXGA;
		config.jpeg_quality = 10;
		config.fb_count = 2;
	  } 
	  else {
		config.frame_size = FRAMESIZE_SVGA;
		config.jpeg_quality = 12;
		config.fb_count = 1;
	  }
		esp_err_t err = esp_camera_init(&config);
	return err; 
}
camera_fb_t * ESPino32CAM::capture()
{
	return esp_camera_fb_get();
}
sensor_t * ESPino32CAM::sensor()
{
	return  esp_camera_sensor_get();
}
bool ESPino32CAM::jpg2rgb(camera_fb_t *fb, dl_matrix3du_t **image_matrix)
{
	*image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
	if (!*image_matrix)
	{
		printDebug("dl_matrix3du_alloc failed");
		return false;
	}
	 bool s = fmt2rgb888(fb->buf, fb->len, fb->format, (*image_matrix)->item);
	if (!s)
	{
		dl_matrix3du_free(*image_matrix);
		printDebug("to rgb888 failed");
		return false;
	}
	return true;
}
bool ESPino32CAM::rgb2jpg(dl_matrix3du_t *rgb888, uint8_t **jpg,size_t *len)
{
	 bool s = fmt2jpg(rgb888->item, rgb888->w * rgb888->h * 3, rgb888->w, rgb888->h, PIXFORMAT_RGB888, 90, jpg, len);
	 return s;
}
dl_matrix3du_t * ESPino32CAM::rgb565(dl_matrix3du_t *image)
{
	dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, image->w, image->h, 2);
	if (!image_matrix)
	{
		printDebug("dl_matrix3du_alloc failed");
		return false;
	}
	size_t n=0;
	for(size_t i=0;i<(image->w*image->h*3);i+=3)
	{
		
		uint16_t color = color565(image->item[i+2],image->item[i+1],image->item[i]);
	    uint8_t hi = color >> 8, lo = color;
		image_matrix->item[n++] = hi;
		image_matrix->item[n++] = lo;
	}
	return image_matrix;
}
size_t ESPino32CAM::grayScale(dl_matrix3du_t *image_888,dl_matrix3du_t **gray)
{
	return (grayScale_(image_888,gray));
}

void ESPino32CAM::clearMemory(camera_fb_t *memory)
{
	esp_camera_fb_return(memory);
}
void ESPino32CAM::clearMemory(dl_matrix3du_t *memory)
{
	dl_matrix3du_free(memory);
}
void ESPino32CAM::clearMemory(uint8_t *memory)
{
	free(memory);
}
void ESPino32CAM::clearMemory(box_array_t *memory)
{
	free(memory->box);
    free(memory->landmark);
    free(memory);
}

box_array_t  * ESPino32CAM::faceDetect(dl_matrix3du_t *image_matrix,mtmn_config_t *config)
{
	return face_detect(image_matrix,config);
}
int8_t  ESPino32CAM::alignFace(dl_matrix3du_t *image_matrix, box_array_t *net_boxes, dl_matrix3du_t **aligned_face)
{
	*aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
	if(!aligned_face)
	{
       printDebug("Could not allocate face recognition buffer");
        return ESP_FAIL;
    }
	return align_face(net_boxes, image_matrix, *aligned_face);
}
int8_t ESPino32CAM::recognizeFace(face_id_list *l,dl_matrix3du_t *algined_face)
{
	return recognize_face(l,algined_face);
}
void  ESPino32CAM::faceIDInit(face_id_list *l, uint8_t size, uint8_t confirm_times)
{
	face_id_init(l, size, confirm_times);
}
bool ESPino32CAM::faceIDInitFlash()
{
	if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
       printDebug("SPIFFS Mount Failed");
        return false;
    }
	return true;
}
bool ESPino32CAM::readFaceIDFromFlash(face_id_list *l)
{
	
	File file = SPIFFS.open(FACE_ID_PATH);
	if(!file || file.isDirectory()){
       printDebug("failed to open file for reading");
       return false;
    }
	l->head = 	file.read();
	l->tail = 	file.read();
    l->count =  file.read();
	l->size = 	file.read();
	l->confirm_times = file.read();
	dl_matrix3d_t **list = l->id_list;
	const int block_len = FACE_ID_SIZE * sizeof(float);
	 for(uint8_t i =0;i<l->count;i++)
	{
		list[i] = dl_matrix3d_alloc(1, 1, 1, FACE_ID_SIZE);
		file.read((uint8_t*)list[i]->item,block_len);           
	}
	file.close();
	return true;
}
bool ESPino32CAM::writeFaceIDToFlash(face_id_list *l)
{
	File file = SPIFFS.open(FACE_ID_PATH,FILE_WRITE);
	if(!file || file.isDirectory()){
       printDebug("failed to open file for reading");
       return false;
    }
	file.write(l->head); 
	file.write(l->tail); 
	file.write(l->count); 
	file.write(l->size); 
	file.write(l->confirm_times);
	const int block_len = FACE_ID_SIZE * sizeof(float);
	for(uint8_t i =0;i<l->count;i++)
   {
     file.write((uint8_t*)l->id_list[i]->item,block_len);           
   }
	 file.close();
	 return true;
}
bool ESPino32CAM::deleteFaceIDinFlash()
{
	 if(SPIFFS.remove(FACE_ID_PATH)){
        printDebug("file deleted");
		return true;
    } else {
       printDebug("delete failed");
    }
	return false;
}
void ESPino32CAM:: drawFaceBoxes(dl_matrix3du_t *image_matrix, box_array_t *boxes,bool landmask)
{
    int x, y, w, h, i;
    uint32_t color = FACE_COLOR_YELLOW;
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    for (i = 0; i < boxes->len; i++){
        // rectangle box
        x = (int)boxes->box[i].box_p[0];
        y = (int)boxes->box[i].box_p[1];
        w = (int)boxes->box[i].box_p[2] - x + 1;
        h = (int)boxes->box[i].box_p[3] - y + 1;
		rbgDrawBox(&fb,color,x,y,w,h);
		if(landmask)
		{
			int x0, y0, j;
			for (j = 0; j < 10; j+=2) {
				x0 = (int)boxes->landmark[i].landmark_p[j];
				y0 = (int)boxes->landmark[i].landmark_p[j+1];
				fb_gfx_fillRect(&fb, x0, y0, 3, 3, color);
			}
		}
	}
}
void ESPino32CAM:: rbgDrawBox(fb_data_t *fb,uint32_t color,int x,int y,int w,int h)
{
	 fb_gfx_drawFastHLine(fb, x, y, w, color);
     fb_gfx_drawFastHLine(fb, x, y+h-1, w, color);
     fb_gfx_drawFastVLine(fb, x, y, h, color);
     fb_gfx_drawFastVLine(fb, x+w-1, y, h, color);
}
void ESPino32CAM:: rgbGoto(int32_t x, int32_t y)
{
	rgb_x = x;
	rgb_y = y;
}
void ESPino32CAM:: rgbPrint(dl_matrix3du_t *image_matrix, uint32_t color, const char * str){
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    //fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, 10, color, str);
	fb_gfx_print(&fb,rgb_x ,rgb_y,color, str);
}
int ESPino32CAM:: rgbPrintf(dl_matrix3du_t *image_matrix, uint32_t color, const char *format, ...){
    char loc_buf[64];
    char * temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(copy);
    if(len >= sizeof(loc_buf)){
        temp = (char*)malloc(len+1);
        if(temp == NULL) {
            return 0;
        }
    }
    vsnprintf(temp, len+1, format, arg);
    va_end(arg);
    rgbPrint(image_matrix, color, temp);
    if(len > 64){
        free(temp);
    }
    return len;
}
void ESPino32CAM:: printfDebug(const char *format, ...){
    char loc_buf[64];
    char * temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(copy);
    if(len >= sizeof(loc_buf)){
        temp = (char*)malloc(len+1);
        if(temp == NULL) {
            return ;
        }
    }
    vsnprintf(temp, len+1, format, arg);
    va_end(arg);
    //rgbPrint(image_matrix, color, temp);
	printDebug(temp);
    if(len > 64){
        free(temp);
    }
   
}

void ESPino32CAM::printDebug(String data)
{
	Serial.print("<DBG>");
	Serial.print(data);
	Serial.print("</DBG>\r\n");
}
uint16_t ESPino32CAM::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
size_t grayScale_(dl_matrix3du_t *image_888,dl_matrix3du_t **gray)
{
	*gray = dl_matrix3du_alloc(1,image_888->w,image_888->h,1);
	if (!*gray)
		return(0);
	(*gray)->w = image_888->w;
	(*gray)->h = image_888->h;
	(*gray)->c = 1;
	size_t len = image_888->w * image_888->h * image_888->c;
	size_t lengray=0;
	for(size_t i=0;i<len;i+=3)
	{
		uint8_t r = image_888->item[i];
		uint8_t g = image_888->item[i+1]; 
		uint8_t b = image_888->item[i+2];
		uint8_t m = max(r, g);
		m = max(m,b);
		uint8_t n = min(r, g);
		n = min(n,b);
		uint8_t gr = (m+n)/2;
		
		//uint8_t gr = (r*0.3)+(g*0.59)+(b*0.11);
		(*gray)->item[lengray]=gr;
		lengray++;	
	}
	return(lengray);
}
size_t gray2gray888(dl_matrix3du_t *gray,dl_matrix3du_t **image_888)
{
	if (*image_888)
			dl_matrix3du_free(*image_888);
	*image_888 = dl_matrix3du_alloc(1,gray->w,gray->h,3);
	if (!*image_888)
		return(0);
	size_t len = gray->w*gray->h*gray->c;
	size_t lenout=0;
	for(size_t i=0;i<len;i++)
	{
		uint8_t g = gray->item[i];
		(*image_888)->item[lenout++] = g;
		(*image_888)->item[lenout++] = g;
		(*image_888)->item[lenout++] = g;
	}
	return(lenout);
}