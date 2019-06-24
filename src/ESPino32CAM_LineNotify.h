#ifndef _ESPINO_LINE_NOTIFY_H_
#define _ESPINO_LINE_NOTIFY_H_


#include "Arduino.h"
#include <WiFiClientSecure.h>
#define lineServer = "notify-api.line.me" 
#define PART_BOUNDARY "123456789000000000000987654321"
//#define PART_BOUNDARY "--------------------------736018195177359813587908"

typedef struct Line_response
{
	bool status;
	uint16_t limit;
	uint8_t  imagelimit;
	uint16_t remaining;
	uint8_t imageremaining;
	uint32_t reset;
	String 	 response;
	String reset_time;
}lineResp;


class LineNotify
{
	public:
		LineNotify();
		void authenKey(String authen);
		void setUTC(int utc_t);
		bool connectServer();
		lineResp message(String message);
		lineResp image(String message,uint8_t *image,size_t size);
		lineResp sticker(String message,uint8_t stickerPacketID,uint16_t stickerID);
		lineResp send(String message,uint8_t stickerPacketID=0,uint16_t stickerID=0,uint8_t *image=NULL,size_t size=0);
		lineResp waitResponse();
	private:
		String authen_;
		int utc_time=0;
		String buildHead();
		WiFiClientSecure lineClient;
		void splitSendPic(uint8_t *image,size_t size);
	
};






#endif