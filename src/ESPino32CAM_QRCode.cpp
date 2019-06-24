#include "ESPino32CAM_QRCode.h"
#include "esp_task_wdt.h"

SemaphoreHandle_t ready_;
dl_matrix3du_t *image_gray; 
TaskHandle_t xHandle;
qrResoult qr_resoult;

ESPino32QRCode::ESPino32QRCode()
{
	xHandle = NULL;
}

void ESPino32QRCode::init(ESPino32CAM *cam)
{
	cam_ = *cam;
	ready_ = xSemaphoreCreateBinary();
}

qrResoult ESPino32QRCode::recognition(dl_matrix3du_t *image888)
{	
disableCore0WDT();
	qrResoult res;
	res.status = false;
	qr_resoult.status = false;	
	if(image_gray)
		cam_.clearMemory(image_gray);
	cam_.grayScale(image888,&image_gray);
	//xTaskCreate(qr_recoginze, "qr_recoginze",111500,image_gray, 5, &xHandle);
	xTaskCreatePinnedToCore(qr_recoginze,"qr_recoginze",111500,image_gray,5,NULL,0);
	xSemaphoreTake(ready_, portMAX_DELAY);
	if(qr_resoult.status)
	{
		res = qr_resoult;
	}		
	enableCore0WDT();
	return(res);
}
String ESPino32QRCode::dataType(uint8_t type)
{
	 switch (type) {
  case QUIRC_DATA_TYPE_NUMERIC:
    return "NUMERIC";
  case QUIRC_DATA_TYPE_ALPHA:
    return "ALPHA";
  case QUIRC_DATA_TYPE_BYTE:
    return "BYTE";
  case QUIRC_DATA_TYPE_KANJI:
    return "KANJI";
  }
  return "unknown";
}
void qr_recoginze(void *pdata) 
{ 
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		dl_matrix3du_t *image_gray = ( dl_matrix3du_t*)pdata;
	//Serial.print("begin to qr_recoginze\r\n");
	struct quirc *q;
	struct quirc_data qd;
	uint8_t *image;
	int w,h;
	w = image_gray->w;
	h = image_gray->h;
	q = quirc_new();
	//Serial.println(w);
	//Serial.println(h);
	
	if (!q) 
	{
		Serial.print("Error: can't create quirc object\r\n");
		qr_resoult.status = false;
		xSemaphoreGive(ready_);
		vTaskDelete(xHandle) ;
	}
	int ret = quirc_resize(q,w,h);
	if (ret< 0)
	{
		Serial.print("Error: quirc_resize\r\n");
		qr_resoult.status = false;
		quirc_destroy(q);
		xSemaphoreGive(ready_);
		vTaskDelete(xHandle) ;
	}
    image = quirc_begin(q, &w,&h);
	memcpy(image, image_gray->item, w*h);
    quirc_end(q);
    int id_count = quirc_count(q);
	if (id_count == 0) 
	{
		Serial.print( "Error: not a valid qrcode\r\n");
		qr_resoult.status = false;
		quirc_destroy(q);
		xSemaphoreGive(ready_);
		vTaskDelete(xHandle);
	}
	qrOutput(q);
	quirc_destroy(q);
	xSemaphoreGive(ready_);
	vTaskDelete(xHandle) ;
}

static void qrOutput(struct quirc *q)
{
	 int count = quirc_count(q);
	 Serial.printf("%d QR-codes found:\r\n", count);
	 qr_resoult.payload="";
	 for (uint8_t i = 0; i < count; i++) 
	{
		
		struct quirc_code code;
		struct quirc_data data;
		quirc_decode_error_t err;
		quirc_extract(q,0, &code);
		err = quirc_decode(&code, &data);
		if (err)
		{			
			qr_resoult.status = false;
		}
		qr_resoult.status 		= true;
		qr_resoult.version 		= data.version;
		qr_resoult.eccLevel 	= "MLHQ"[data.ecc_level];
		qr_resoult.mask 		= data.mask;
		qr_resoult.dataType		= data.data_type;
		qr_resoult.length		= data.payload_len;
		qr_resoult.eci			= data.eci;
		
		for(int cnt =0;cnt<data.payload_len;cnt++)
		{
			char c = data.payload[cnt];
			qr_resoult.payload += c;
		}
		if(data.payload_len==0)
		{
			qr_resoult.status 		= false;
		}
		else
			return;
	}
	
}

static void dump_info(struct quirc *q) {
  int count = quirc_count(q);
  int i;

  Serial.printf("%d QR-codes found:\n\n", count);
  for (i = 0; i < count; i++) {
    struct quirc_code code;
    struct quirc_data data;
    quirc_decode_error_t err;
    quirc_extract(q, i, &code);
    err = quirc_decode(&code, &data);
    Serial.print("\n");

    if (err) 
   
      Serial.printf("  Decoding FAILED: %s\r\n", quirc_strerror(err));
    else 
    {
      Serial.println("Decoding successful:");
      dump_data(&data);
    }

    Serial.println();
  }
}
void dump_data(const struct quirc_data *data) {
  Serial.printf("    Version: %d\n", data->version);
  Serial.printf("    ECC level: %c\n", "MLHQ"[data->ecc_level]);
  Serial.printf("    Mask: %d\n", data->mask);
  Serial.printf("    Data type: %d (%s)\n", data->data_type,
      data_type_str(data->data_type));
  Serial.printf("    Length: %d\n", data->payload_len);
  Serial.printf("    Payload: %s\n", data->payload);

  if (data->eci)
    Serial.printf("    ECI: %d\n", data->eci);
}

static const char *data_type_str(int dt) {
  switch (dt) {
  case QUIRC_DATA_TYPE_NUMERIC:
    return "NUMERIC";
  case QUIRC_DATA_TYPE_ALPHA:
    return "ALPHA";
  case QUIRC_DATA_TYPE_BYTE:
    return "BYTE";
  case QUIRC_DATA_TYPE_KANJI:
    return "KANJI";
  }

  return "unknown";
}