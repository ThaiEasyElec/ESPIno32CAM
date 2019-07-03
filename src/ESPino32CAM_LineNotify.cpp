#include "ESPino32CAM_LineNotify.h"


LineNotify::LineNotify()
{
}
void LineNotify::authenKey(String authen)
{
	authen_ = authen;
}
String LineNotify::buildHead()
{
  String head_str = F("POST /api/notify HTTP/1.1\r\n");
  head_str += "authorization: Bearer "+authen_+"\r\n";
  head_str += F("Accept: */*\r\n");
  head_str += F("Host: notify-api.line.me\r\n");
  head_str += F("accept-encoding: gzip, deflate\r\n");
  head_str += F("Connection: keep-alive\r\n");
  head_str += F("content-type: multipart/form-data; boundary=");
  head_str += PART_BOUNDARY;
  head_str += F("\r\n");
  head_str += F("content-length: ");
  return head_str;
}
bool LineNotify::connectServer()
{
	return lineClient.connect("notify-api.line.me", 443);	
}
lineResp LineNotify::waitResponse()
{
	lineResp ret;
	ret.status = false;
	ret.response="";
	if(!lineClient.connected())
	{
		ret.response="Disconnect";
		return(ret);
	}
	unsigned long old_t = millis();
	//Serial.println("Wait Resp");
	while(lineClient.connected()) 
	{
		String line = lineClient.readStringUntil('\n');
		if(line.indexOf(F("HTTP/1.1 200"))!=-1)
		{
				ret.status = true;
		}
		if(line.indexOf(F("X-RateLimit"))!=-1)
		{
			int index;
			if(line.indexOf(F("-Limit:"))!=-1)
			{
				index = line.indexOf(F(":"));
				ret.limit = line.substring(index+1).toInt();
			}
			if(line.indexOf(F("-ImageLimit:"))!=-1)
			{
				index = line.indexOf(F(":"));
				ret.imagelimit = line.substring(index+1).toInt();
			}
			if(line.indexOf(F("-Remaining:"))!=-1)
			{
				index = line.indexOf(F(":"));
				ret.remaining = line.substring(index+1).toInt();
			}
			if(line.indexOf(F("-ImageRemaining:"))!=-1)
			{
				index = line.indexOf(F(":"));
				ret.imageremaining = line.substring(index+1).toInt();
			}
			if(line.indexOf(F("-Reset:"))!=-1)
			{
				index = line.indexOf(F(":"));
				ret.reset = line.substring(index+1).toInt();
				uint32_t  epoch = ret.reset+utc_time;
				uint8_t h = (epoch  % 86400L) / 3600;
				uint8_t m = (epoch  % 3600) / 60;
				uint8_t s = (epoch % 60);
				ret.reset_time = String(h)+":"+String(m)+":"+String(s);
			}
		}
		if(line.indexOf(F("}"))!=-1)
		{
			ret.response += line+"\r\n";
			break;
		}
		if((millis()-old_t)>5000)
		{
			//Serial.println("Response Timeout");
			ret.response="Timeout";
			break;
		}
	}
  //Serial.println("Close Socket");
  lineClient.stop();
  return(ret);
}
lineResp LineNotify::message(String message)
{
	return send(message);	
}
lineResp LineNotify::image(String message,uint8_t *image,size_t size)
{
	return send(message,0,0,image,size);	
}
lineResp LineNotify::sticker(String message,uint8_t stickerPacketID,uint16_t stickerID)
{
	return send(message,stickerPacketID,stickerID);		
}
void LineNotify:: splitSendPic(uint8_t *image,size_t size)
{
	size_t add=0;
	if(size>10000)
	{
		uint8_t num = size/10000;
		for(uint8_t i=0;i<num;i++)
		{
			lineClient.write(image+add,10000);
			add+=10000;
		}
		lineClient.write(image+add,size-add);
	}
	else
		lineClient.write(image,size);
}
lineResp LineNotify::send(String message,uint8_t stickerPacketID,uint16_t stickerID,uint8_t *image,size_t size)
{
	lineResp ret;
	ret.status = false;
	ret.response="Disconnect";
	
	if(message.length()==0)
	{
		ret.response= "Message Error";
		return(ret);		
	}
	if (!connectServer())
	{
		ret.response= "Connection failed!";
		return(ret);		
	}
	String req = buildHead();
	String 	bound = "--";
			bound += PART_BOUNDARY;
	String 	startBound = bound+"\r\n";
	String 	stopBound = bound+"--\r\n";
	
	String data = startBound+"Content-Disposition: form-data; name=\"message\"\r\n\r\n";
	data += message;
	
	if((stickerPacketID!=0)&&(stickerID!=0))
	{
		data += "\r\n";
		data += startBound;
		data += "Content-Disposition: form-data; name=\"stickerPackageId\"\r\n\r\n";
		data += String(stickerPacketID);
		data += "\r\n";
		data += startBound;
		data += "Content-Disposition: form-data; name=\"stickerId\"\r\n\r\n";
		data += String(stickerID);
		
	}
	if(image!=NULL)
	{
		data += "\r\n";
		data += startBound;
		data += "Content-Disposition: form-data; name=\"imageFile\"; filename=\"espcampic.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
	}
	
	req += String(data.length()+size+2+stopBound.length())+"\r\n\r\n";
	lineClient.print(req);
	lineClient.print(data);
	if(image!=NULL)
	 splitSendPic(image,size);
	lineClient.print("\r\n"+stopBound);
	return waitResponse();
	
}
void LineNotify::setUTC(int utc_t)
{
	utc_time = (utc_t)*60*60;
}






