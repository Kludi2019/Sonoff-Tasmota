
#if defined(ESP32) && defined(USE_WEBCAM)

#define CAMERA_MODEL_AI_THINKER

//#define USE_TEMPLATE


#ifndef USE_TEMPLATE

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#endif


#include "esp_camera.h"
#include "sensor.h"

//#include "SimStreamer.h"
//#include "OV2640Streamer.h"
//#include "OV2640.h"
//#include "CRtspSession.h"



uint8_t wc_up;
uint16_t wc_width;
uint16_t wc_height;

uint32_t webcam_setup(void) {
bool psram;
camera_fb_t *wc_fb;

  if (wc_up) {
    return wc_up;
  }

//esp_log_level_set("*", ESP_LOG_VERBOSE);

camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

#ifndef USE_TEMPLATE
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

#else
/*
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
*/

  config.pin_d0 = pin[GPIO_WEBCAM_Y2_GPIO_NUM];  //Y2_GPIO_NUM;
  config.pin_d1 = pin[GPIO_WEBCAM_Y3_GPIO_NUM];  //Y3_GPIO_NUM;
  config.pin_d2 = pin[GPIO_WEBCAM_Y4_GPIO_NUM];  //Y4_GPIO_NUM;
  config.pin_d3 = pin[GPIO_WEBCAM_Y5_GPIO_NUM];  //Y5_GPIO_NUM;
  config.pin_d4 = pin[GPIO_WEBCAM_Y6_GPIO_NUM];  //Y6_GPIO_NUM;
  config.pin_d5 = pin[GPIO_WEBCAM_Y7_GPIO_NUM];  //Y7_GPIO_NUM;
  config.pin_d6 = pin[GPIO_WEBCAM_Y8_GPIO_NUM];  //Y8_GPIO_NUM;
  config.pin_d7 = pin[GPIO_WEBCAM_Y9_GPIO_NUM];  //Y9_GPIO_NUM;
  config.pin_xclk = pin[GPIO_WEBCAM_XCLK_GPIO_NUM];  //XCLK_GPIO_NUM;
  config.pin_pclk = pin[GPIO_WEBCAM_PCLK_GPIO_NUM];  //PCLK_GPIO_NUM;
  config.pin_vsync = pin[GPIO_WEBCAM_VSYNC_GPIO_NUM];  //VSYNC_GPIO_NUM;
  config.pin_href = pin[GPIO_WEBCAM_HREF_GPIO_NUM];  //HREF_GPIO_NUM;
  config.pin_sscb_sda = pin[GPIO_WEBCAM_SIOD_GPIO_NUM];  //SIOD_GPIO_NUM;
  config.pin_sscb_scl = pin[GPIO_WEBCAM_SIOC_GPIO_NUM];  //SIOC_GPIO_NUM;

  int16_t xpin;
  xpin=pin[GPIO_WEBCAM_PWDN_GPIO_NUM];
  if (xpin==99) xpin=-1;
  config.pin_pwdn = xpin; //PWDN_GPIO_NUM;
  xpin=pin[GPIO_WEBCAM_RESET_GPIO_NUM];
  if (xpin==99) xpin=-1;
  config.pin_reset = xpin; //RESET_GPIO_NUM;
#endif

  //ESP.getPsramSize()

  //esp_log_level_set("*", ESP_LOG_INFO);


  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  psram=psramFound();
  if (psram) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    AddLog_P(LOG_LEVEL_INFO,"PSRAM found!");
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    AddLog_P(LOG_LEVEL_INFO,"PSRAM not found!");
  }

  // stupid workaround camera diver eats up static ram should prefer SPIRAM
  // so we steal static ram to force driver to alloc SPIRAM
  //ESP.getMaxAllocHeap()

//  void *x=malloc(70000);
void *x=0;

  esp_err_t err = esp_camera_init(&config);

  if (x) free(x);

  if (err != ESP_OK) {
    AddLog_P2(LOG_LEVEL_INFO,"Camera init failed with error 0x%x", err);
    return 0;
  }

  sensor_t * wc_s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (wc_s->id.PID == OV3660_PID) {
    wc_s->set_vflip(wc_s, 1); // flip it back
    wc_s->set_brightness(wc_s, 1); // up the brightness just a bit
    wc_s->set_saturation(wc_s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  wc_s->set_framesize(wc_s, FRAMESIZE_CIF);


  AddLog_P(LOG_LEVEL_INFO,"Camera successfully initialized!");


  //if (s_state)
  //wc_width=s_state->width;

  wc_up=1;

  if (psram) {
    wc_up=2;
  }
  return wc_up;
}

uint32_t wc_set_framesize(uint32_t size) {
  sensor_t *s = esp_camera_sensor_get();
  return s->set_framesize(s,(framesize_t)size);
}

uint32_t wc_get_width(void) {
  return wc_width;
}

uint32_t wc_get_height(void) {
  return wc_height;
}

#define MAX_PICSTORE 4
struct PICSTORE {
  uint8_t *buff;
  uint32_t len;
} picstore[MAX_PICSTORE];


uint32_t get_picstore(int32_t num, uint8_t **buff) {
  if (num<0) return MAX_PICSTORE;
  *buff=picstore[num].buff;
  return picstore[num].len;
}

uint32_t wc_get_jpeg(uint8_t **buff) {
size_t _jpg_buf_len = 0;
uint8_t * _jpg_buf = NULL;
camera_fb_t *wc_fb;
  wc_fb = esp_camera_fb_get();
  if (!wc_fb) return 0;
  if (wc_fb->format!=PIXFORMAT_JPEG) {
    bool jpeg_converted = frame2jpg(wc_fb, 80, &_jpg_buf, &_jpg_buf_len);
    if (!jpeg_converted){
        _jpg_buf_len = wc_fb->len;
        _jpg_buf = wc_fb->buf;
    }
  } else {
    _jpg_buf_len = wc_fb->len;
    _jpg_buf = wc_fb->buf;
  }
  esp_camera_fb_return(wc_fb);
  *buff=_jpg_buf;
  return _jpg_buf_len;
}


uint32_t wc_get_frame(int32_t bnum) {
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  camera_fb_t *wc_fb;

  if (bnum<0) {
    if (bnum<-MAX_PICSTORE) bnum=-1;
    bnum=-bnum;
    bnum--;
    if (picstore[bnum].buff) free(picstore[bnum].buff);
    picstore[bnum].len=0;
    return 0;
  }

  wc_fb = esp_camera_fb_get();
  if (!wc_fb) return 0;
  uint32_t len=wc_fb->len;

  if (!bnum) {
    wc_width = wc_fb->width;
    wc_height = wc_fb->height;
    esp_camera_fb_return(wc_fb);
    return 0;
  }

  if (bnum<1 || bnum>MAX_PICSTORE) bnum=1;
  bnum--;
  if (picstore[bnum].buff) free(picstore[bnum].buff);
  picstore[bnum].buff = (uint8_t *)heap_caps_malloc(len+4,MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (picstore[bnum].buff) {
    if (wc_fb->format!=PIXFORMAT_JPEG) {
      bool jpeg_converted = frame2jpg(wc_fb, 80, &_jpg_buf, &_jpg_buf_len);
      if (!jpeg_converted){
          //Serial.println("JPEG compression failed");
          _jpg_buf_len = wc_fb->len;
          _jpg_buf = wc_fb->buf;
      }
    } else {
      _jpg_buf_len = wc_fb->len;
      _jpg_buf = wc_fb->buf;
    }
    memcpy(picstore[bnum].buff,_jpg_buf,_jpg_buf_len);
    picstore[bnum].len=_jpg_buf_len;
  } else {
    picstore[bnum].len=0;
  }
  esp_camera_fb_return(wc_fb);
  if (!picstore[bnum].buff) return 0;
  return  len;
}

bool HttpCheckPriviledgedAccess(bool);
extern ESP8266WebServer *Webserver;

void HandleImage(void) {
  if (!HttpCheckPriviledgedAccess(true)) { return; }

  uint32_t bnum = Webserver->arg(F("p")).toInt();
  if (bnum<0 || bnum>MAX_PICSTORE) bnum=1;
  WiFiClient client = Webserver->client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-disposition: inline; filename=capture.jpg\r\n";
  response += "Content-type: image/jpeg\r\n\r\n";
  Webserver->sendContent(response);

  if (!bnum) {
    uint8_t *buff;
    uint32_t len;
    len=wc_get_jpeg(&buff);
    if (len) {
      client.write(buff,len);
    }
  } else {
    bnum--;
    if (!picstore[bnum].len) {
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR("no image #: %d"), bnum);
      return;
    }
    client.write((char *)picstore[bnum].buff, picstore[bnum].len);
  }

  AddLog_P2(LOG_LEVEL_DEBUG, PSTR("sending image #: %d"), bnum+1);

}

ESP8266WebServer *CamServer;
#define BOUNDARY "e8b8c539-047d-4777-a985-fbba6edff11e"
void handleMjpeg(void) {
  camera_fb_t *wc_fb;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;

  AddLog_P(LOG_LEVEL_INFO, "stream");
  WiFiClient client = CamServer->client();
  client.print("HTTP/1.1 200 OK\r\n"
  			"Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n"
  				"\r\n");

  int nFrames;
  for (nFrames = 0; nFrames < 25; ++nFrames) {

    wc_fb = esp_camera_fb_get();
    if (!wc_fb) return;

    if (wc_fb->format!=PIXFORMAT_JPEG) {
      bool jpeg_converted = frame2jpg(wc_fb, 80, &_jpg_buf, &_jpg_buf_len);
      if (!jpeg_converted){
        AddLog_P(LOG_LEVEL_INFO, "JPEG compression failed");
        _jpg_buf_len = wc_fb->len;
        _jpg_buf = wc_fb->buf;
      }
    } else {
      _jpg_buf_len = wc_fb->len;
      _jpg_buf = wc_fb->buf;
    }

    client.printf("Content-Type: image/jpeg\r\n"
      "Content-Length: %d\r\n"
        "\r\n", static_cast<int>(_jpg_buf_len));
    client.write(_jpg_buf, _jpg_buf_len);
    client.print("\r\n--" BOUNDARY "\r\n");
    esp_camera_fb_return(wc_fb);
    delay(20);
  }
  //client.write(wc_fb->buf, wc_fb->len);

  /*
  Serial.println("STREAM BEGIN");
  WiFiClient client = server.client();
  auto startTime = millis();

  int res = esp32cam::Camera.streamMjpeg(client);
  if (res <= 0) {
    Serial.printf("STREAM ERROR %d\n", res);
    return;
  }
  auto duration = millis() - startTime;
  Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
*/
AddLog_P(LOG_LEVEL_INFO, "stream exit");
}

/*
int
		CameraClass::streamMjpeg(Client& client, const StreamMjpegConfig& cfg)
	{
#define BOUNDARY "e8b8c539-047d-4777-a985-fbba6edff11e"
		client.print("HTTP/1.1 200 OK\r\n"
			"Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n"
				"\r\n");
		auto lastCapture = millis();
		int nFrames;
		for (nFrames = 0; cfg.maxFrames < 0 || nFrames < cfg.maxFrames; ++nFrames) {
			auto now = millis();
			auto sinceLastCapture = now - lastCapture;
			if (static_cast<int>(sinceLastCapture) < cfg.minInterval) {
				delay(cfg.minInterval - sinceLastCapture);
			}
			lastCapture = millis();

			auto frame = capture();
			if (frame == nullptr) {
				break;
			}

			client.printf("Content-Type: image/jpeg\r\n"
				"Content-Length: %d\r\n"
					"\r\n", static_cast<int>(frame->size()));
			if (!frame->writeTo(client, cfg.frameTimeout)) {
				break;
			}
			client.print("\r\n--" BOUNDARY "\r\n");
		}
		return nFrames;
#undef BOUNDARY
*/

void CamHandleRoot(void) {
  //CamServer->redirect("http://" + String(ip) + ":81/cam.mjpeg");
  CamServer->sendHeader("Location", WiFi.localIP().toString() + ":81/cam.mjpeg");
  CamServer->send(302, "", "");
}



uint32_t wc_set_streamserver(uint32_t flag) {

  if (global_state.wifi_down) return 0;

  if (flag) {
    if (!CamServer) {
      CamServer = new ESP8266WebServer(81);
      CamServer->on("/", CamHandleRoot);
      CamServer->on("/cam.mjpeg", handleMjpeg);
      AddLog_P(LOG_LEVEL_INFO, "cam stream init");
      CamServer->begin();
    }
  } else {
    if (CamServer) {
      CamServer->stop();
      delete CamServer;
      CamServer=NULL;
      AddLog_P(LOG_LEVEL_INFO, "cam stream exit");
    }
  }
  return 0;
}

void wc_loop(void) {
  if (CamServer) CamServer->handleClient();
}

void wc_pic_setup(void) {
  Webserver->on("/wc.jpg", HandleImage);
  Webserver->on("/wc.mjpeg", HandleImage);
}

/*
typedef enum {
    FRAMESIZE_96x96,    // 96x96
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QQVGA2,   // 128x160
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 240x176
    FRAMESIZE_240x240,  // 240x240
    FRAMESIZE_QVGA,     // 320x240 6
    FRAMESIZE_CIF,      // 400x296 7
    FRAMESIZE_VGA,      // 640x480 8
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    FRAMESIZE_QXGA,     // 2048*1536
    FRAMESIZE_INVALID
} framesize_t;

flash led = gpio4
red led = gpio 33
*/


#endif
