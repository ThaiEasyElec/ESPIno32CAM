#ifndef _ESPINO_QRCODE_H_
#define _ESPINO_QRCODE_H_

#include "ESPino32CAM.h"
#include "quirc/quirc_internal.h"

enum qrState {
	qr_process = 0, 
	qr_fail= 1, 
	qr_success = 2,
	qr_idle = 3
};


typedef struct{
   bool status;
   uint8_t version;
   char eccLevel;
   uint8_t mask;
   uint8_t dataType;
   uint8_t length;
   uint8_t eci;
   String  payload;
   
}qrResoult;



class ESPino32QRCode
{
public:
	ESPino32QRCode();
	void init(ESPino32CAM *cam);
	qrResoult recognition(dl_matrix3du_t *image888);
	String dataType(uint8_t type);
	
private:
	ESPino32CAM cam_;
};
static void qrOutput(struct quirc *q);
void qr_recoginze(void *pdata);
void dump_data(const struct quirc_data *data);
static void dump_info(struct quirc *q);
static const char *data_type_str(int dt);





















#endif