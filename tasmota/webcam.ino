
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


uint8_t wc_up;
uint16_t wc_width;
uint16_t wc_height;
uint8_t wc_stream_active;

uint32_t webcam_setup(void) {
bool psram;
camera_fb_t *wc_fb;

  wc_stream_active=0;

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

  config.pin_d0 = Pin(GPIO_WEBCAM_Y2_GPIO_NUM);  //Y2_GPIO_NUM;
  config.pin_d1 = Pin(GPIO_WEBCAM_Y3_GPIO_NUM);  //Y3_GPIO_NUM;
  config.pin_d2 = Pin(GPIO_WEBCAM_Y4_GPIO_NUM);  //Y4_GPIO_NUM;
  config.pin_d3 = Pin(GPIO_WEBCAM_Y5_GPIO_NUM);  //Y5_GPIO_NUM;
  config.pin_d4 = Pin(GPIO_WEBCAM_Y6_GPIO_NUM);  //Y6_GPIO_NUM;
  config.pin_d5 = Pin(GPIO_WEBCAM_Y7_GPIO_NUM);  //Y7_GPIO_NUM;
  config.pin_d6 = Pin(GPIO_WEBCAM_Y8_GPIO_NUM);  //Y8_GPIO_NUM;
  config.pin_d7 = Pin(GPIO_WEBCAM_Y9_GPIO_NUM);  //Y9_GPIO_NUM;
  config.pin_xclk = Pin(GPIO_WEBCAM_XCLK_GPIO_NUM);  //XCLK_GPIO_NUM;
  config.pin_pclk = Pin(GPIO_WEBCAM_PCLK_GPIO_NUM);  //PCLK_GPIO_NUM;
  config.pin_vsync = Pin(GPIO_WEBCAM_VSYNC_GPIO_NUM);  //VSYNC_GPIO_NUM;
  config.pin_href = Pin(GPIO_WEBCAM_HREF_GPIO_NUM);  //HREF_GPIO_NUM;
  config.pin_sscb_sda = Pin(GPIO_WEBCAM_SIOD_GPIO_NUM);  //SIOD_GPIO_NUM;
  config.pin_sscb_scl = Pin(GPIO_WEBCAM_SIOC_GPIO_NUM);  //SIOC_GPIO_NUM;

  int16_t xpin;
  xpin=Pin(GPIO_WEBCAM_PWDN_GPIO_NUM);
  if (xpin==99) xpin=-1;
  config.pin_pwdn = xpin; //PWDN_GPIO_NUM;
  xpin=Pin(GPIO_WEBCAM_RESET_GPIO_NUM);
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
};

struct PICSTORE picstore[MAX_PICSTORE];
struct PICSTORE tmp_picstore;

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
  camera_fb_t *wc_fb=0;

  if (bnum<0) {
    if (bnum<-MAX_PICSTORE) bnum=-1;
    bnum=-bnum;
    bnum--;
    if (picstore[bnum].buff) free(picstore[bnum].buff);
    picstore[bnum].len=0;
    return 0;
  }

  if (bnum&0x10) {
    bnum&=0xf;
    _jpg_buf=tmp_picstore.buff;
    _jpg_buf_len=tmp_picstore.len;
    if (!_jpg_buf_len) return 0;
    goto pcopy;
  }

  wc_fb = esp_camera_fb_get();
  if (!wc_fb) return 0;

  if (!bnum) {
    wc_width = wc_fb->width;
    wc_height = wc_fb->height;
    esp_camera_fb_return(wc_fb);
    return 0;
  }
  bool jpeg_converted;
  if (wc_fb->format!=PIXFORMAT_JPEG) {
    jpeg_converted = frame2jpg(wc_fb, 80, &_jpg_buf, &_jpg_buf_len);
    if (!jpeg_converted){
        //Serial.println("JPEG compression failed");
        _jpg_buf_len = wc_fb->len;
        _jpg_buf = wc_fb->buf;
    }
  } else {
    _jpg_buf_len = wc_fb->len;
    _jpg_buf = wc_fb->buf;
  }

pcopy:
  if (bnum<1 || bnum>MAX_PICSTORE) bnum=1;
  bnum--;
  if (picstore[bnum].buff) free(picstore[bnum].buff);
  picstore[bnum].buff = (uint8_t *)heap_caps_malloc(_jpg_buf_len+4,MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (picstore[bnum].buff) {
    memcpy(picstore[bnum].buff,_jpg_buf,_jpg_buf_len);
    picstore[bnum].len=_jpg_buf_len;
  } else {
    picstore[bnum].len=0;
  }
  if (wc_fb) esp_camera_fb_return(wc_fb);
  //if (jpeg_converted) free(_jpg_buf);
  if (!picstore[bnum].buff) return 0;
  return  _jpg_buf_len;
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
      free(buff);
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

WiFiClient client;
uint32_t wc_timer;

void handleMjpeg(void) {
  AddLog_P(LOG_LEVEL_INFO, "handle camserver");
  //if (!wc_stream_active) {
    wc_stream_active=1;
    client = CamServer->client();
    AddLog_P(LOG_LEVEL_INFO, "create client");
  //}
//  wc_timer=10;
}

void handleMjpeg_task(void) {
  camera_fb_t *wc_fb;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  //WiFiClient client = CamServer->client();
  uint32_t tlen;

/*
  if (wc_timer) {
    wc_timer--;
    return;
  }
*/

  if (!client.connected()) {
    wc_stream_active=0;
    AddLog_P(LOG_LEVEL_INFO,"client fail");
    goto exit;
  }

  if (wc_stream_active==1) {
    client.flush();
    client.setTimeout(3);
    AddLog_P(LOG_LEVEL_INFO, "start stream");
    client.print("HTTP/1.1 200 OK\r\n"
  			"Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n"
  				"\r\n");
    wc_stream_active=2;
  } else {



    wc_fb = esp_camera_fb_get();
    if (!wc_fb) {
      wc_stream_active=0;
      AddLog_P(LOG_LEVEL_INFO, "frame fail");
      goto exit;
    }

    bool jpeg_converted;
    if (wc_fb->format!=PIXFORMAT_JPEG) {
      jpeg_converted = frame2jpg(wc_fb, 80, &_jpg_buf, &_jpg_buf_len);
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
    tlen=client.write(_jpg_buf, _jpg_buf_len);
    /*
    if (tlen!=_jpg_buf_len) {
      esp_camera_fb_return(wc_fb);
      wc_stream_active=0;
      AddLog_P(LOG_LEVEL_INFO, "send fail");
    }*/
    client.print("\r\n--" BOUNDARY "\r\n");

    if (tmp_picstore.buff) free(tmp_picstore.buff);
    tmp_picstore.buff = (uint8_t *)heap_caps_malloc(_jpg_buf_len+4,MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (tmp_picstore.buff) {
      memcpy(tmp_picstore.buff,_jpg_buf,_jpg_buf_len);
      tmp_picstore.len=_jpg_buf_len;
    } else {
      tmp_picstore.len=0;
    }
    //if (jpeg_converted) free(_jpg_buf);
    esp_camera_fb_return(wc_fb);
    //AddLog_P(LOG_LEVEL_INFO, "send frame");

    //Serial.printf("loop %d\n",client.status());
    //if (client.available()) {
    //  char c = client.read();
    //  Serial.print(c);
    //}
exit:
    if (!wc_stream_active) {
      AddLog_P(LOG_LEVEL_INFO, "stream exit");
      client.flush();
      client.stop();
    }
  }
}

void CamHandleRoot(void) {
  //CamServer->redirect("http://" + String(ip) + ":81/cam.mjpeg");
  CamServer->sendHeader("Location", WiFi.localIP().toString() + ":81/cam.mjpeg");
  CamServer->send(302, "", "");
  Serial.printf("WC root called");
}

uint32_t wc_set_streamserver(uint32_t flag) {

  if (global_state.wifi_down) return 0;

  wc_stream_active=0;

  if (flag) {
    if (!CamServer) {
      CamServer = new ESP8266WebServer(81);
      CamServer->on("/", CamHandleRoot);
      CamServer->on("/cam.mjpeg", handleMjpeg);
      CamServer->on("/cam.jpg", handleMjpeg);
      CamServer->on("/stream", handleMjpeg);
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
  if (wc_stream_active) handleMjpeg_task();
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



typedef struct {
        size_t size; //number of values used for filtering
        size_t index; //current value index
        size_t count; //value count
        int sum;
        int * values; //array to be filled with values
} ra_filter_t;

//#include "esp_http_server.h"
//#include "fb_gfx.h"
//#include "esp_timer.h"
//#include "esp_camera.h"
//#include "img_converters.h"

/*
static ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size){
    memset(filter, 0, sizeof(ra_filter_t));

    filter->values = (int *)malloc(sample_size * sizeof(int));
    if(!filter->values){
        return NULL;
    }
    memset(filter->values, 0, sample_size * sizeof(int));

    filter->size = sample_size;
    return filter;
}

static int ra_filter_run(ra_filter_t * filter, int value){
    if(!filter->values){
        return value;
    }
    filter->sum -= filter->values[filter->index];
    filter->values[filter->index] = value;
    filter->sum += filter->values[filter->index];
    filter->index++;
    filter->index = filter->index % filter->size;
    if (filter->count < filter->size) {
        filter->count++;
    }
    return filter->sum / filter->count;
}


static esp_err_t stream_handler(void){
httpd_req_t *req=0;

    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];
#if CONFIG_ESP_FACE_DETECT_ENABLED
    dl_matrix3du_t *image_matrix = NULL;
    bool detected = false;
    int face_id = 0;
    int64_t fr_start = 0;
    int64_t fr_ready = 0;
    int64_t fr_face = 0;
    int64_t fr_recognize = 0;
    int64_t fr_encode = 0;
#endif

    static int64_t last_frame = 0;
    if (!last_frame)
    {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    enable_led(true);
    isStreaming = true;
#endif

    while (true)
    {
#if CONFIG_ESP_FACE_DETECT_ENABLED
        detected = false;
        face_id = 0;
#endif

        fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        }
        else
        {
            _timestamp.tv_sec = fb->timestamp.tv_sec;
            _timestamp.tv_usec = fb->timestamp.tv_usec;
#if CONFIG_ESP_FACE_DETECT_ENABLED
            fr_start = esp_timer_get_time();
            fr_ready = fr_start;
            fr_face = fr_start;
            fr_encode = fr_start;
            fr_recognize = fr_start;
            if (!detection_enabled || fb->width > 400)
            {
#endif
                if (fb->format != PIXFORMAT_JPEG)
                {
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted)
                    {
                        ESP_LOGE(TAG, "JPEG compression failed");
                        res = ESP_FAIL;
                    }
                }
                else
                {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
#if CONFIG_ESP_FACE_DETECT_ENABLED
            }
            else
            {

                image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

                if (!image_matrix)
                {
                    ESP_LOGE(TAG, "dl_matrix3du_alloc failed");
                    res = ESP_FAIL;
                }
                else
                {
                    if (!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item))
                    {
                        ESP_LOGE(TAG, "fmt2rgb888 failed");
                        res = ESP_FAIL;
                    }
                    else
                    {
#if CONFIG_ESP_FACE_DETECT_LSSH
                        // lssh_update_config(&lssh_config, min_face, image_matrix->h, image_matrix->w);
#endif
                        fr_ready = esp_timer_get_time();
                        box_array_t *net_boxes = NULL;
                        if (detection_enabled)
                        {
#if CONFIG_ESP_FACE_DETECT_MTMN
                            net_boxes = face_detect(image_matrix, &mtmn_config);
#endif

#if CONFIG_ESP_FACE_DETECT_LSSH
                            net_boxes = lssh_detect_object(image_matrix, lssh_config);
#endif
                        }
                        fr_face = esp_timer_get_time();
                        fr_recognize = fr_face;
                        if (net_boxes || fb->format != PIXFORMAT_JPEG)
                        {
                            if (net_boxes)
                            {
                                detected = true;
#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
                                if (recognition_enabled)
                                {
                                    face_id = run_face_recognition(image_matrix, net_boxes);
                                }
                                fr_recognize = esp_timer_get_time();
#endif
                                draw_face_boxes(image_matrix, net_boxes, face_id);
                                dl_lib_free(net_boxes->score);
                                dl_lib_free(net_boxes->box);
                                if (net_boxes->landmark != NULL)
                                    dl_lib_free(net_boxes->landmark);
                                dl_lib_free(net_boxes);
                            }
                            if (!fmt2jpg(image_matrix->item, fb->width * fb->height * 3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len))
                            {
                                ESP_LOGE(TAG, "fmt2jpg failed");
                            }
                            esp_camera_fb_return(fb);
                            fb = NULL;
                        }
                        else
                        {
                            _jpg_buf = fb->buf;
                            _jpg_buf_len = fb->len;
                        }
                        fr_encode = esp_timer_get_time();
                    }
                    dl_matrix3du_free(image_matrix);
                }
            }
#endif
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }
        else if (_jpg_buf)
        {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK)
        {
            break;
        }
        int64_t fr_end = esp_timer_get_time();

#if CONFIG_ESP_FACE_DETECT_ENABLED
        int64_t ready_time = (fr_ready - fr_start) / 1000;
        int64_t face_time = (fr_face - fr_ready) / 1000;
        int64_t recognize_time = (fr_recognize - fr_face) / 1000;
        int64_t encode_time = (fr_encode - fr_recognize) / 1000;
        int64_t process_time = (fr_encode - fr_start) / 1000;
#endif

        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        ESP_LOGI(TAG, "MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps)"
#if CONFIG_ESP_FACE_DETECT_ENABLED
                      ", %u+%u+%u+%u=%u %s%d"
#endif
                 ,
                 (uint32_t)(_jpg_buf_len),
                 (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
                 avg_frame_time, 1000.0 / avg_frame_time
#if CONFIG_ESP_FACE_DETECT_ENABLED
                 ,
                 (uint32_t)ready_time, (uint32_t)face_time, (uint32_t)recognize_time, (uint32_t)encode_time, (uint32_t)process_time,
                 (detected) ? "DETECTED " : "", face_id
#endif
        );
    }

#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    isStreaming = false;
    enable_led(false);
#endif

    last_frame = 0;
    return res;
}
*/

#endif
