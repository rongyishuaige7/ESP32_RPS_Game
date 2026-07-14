#include "CameraStream.h"
#include "StateMachine.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_http_server.h>
#include <esp_camera.h>
#include <img_converters.h>
#include <esp_log.h>

static const char* TAG = "CameraStream";

#define PART_BOUNDARY "123456789000000000000987654321"
#define STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=" PART_BOUNDARY
#define STREAM_BOUNDARY "\r\n--" PART_BOUNDARY "\r\n"
#define STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"

static httpd_handle_t s_stream_httpd = NULL;
static StateMachine* s_stateMachine = NULL;

static esp_err_t stream_handler(httpd_req_t* req) {
    camera_fb_t* fb = NULL;
    uint8_t* jpg_buf = NULL;
    size_t jpg_len = 0;
    char part_buf[96];
    const unsigned long frame_interval_ms = 1000 / STREAM_FPS;

    if (httpd_resp_set_type(req, STREAM_CONTENT_TYPE) != ESP_OK ||
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*") != ESP_OK) {
        return ESP_FAIL;
    }

    while (true) {
        GameState state = s_stateMachine ? s_stateMachine->getState() : STATE_IDLE;
        bool allow_stream = (state == STATE_IDLE || state == STATE_RESULT);

        if (allow_stream) {
            fb = esp_camera_fb_get();
            if (!fb) {
                ESP_LOGW(TAG, "Camera capture failed");
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }

            if (fb->format == PIXFORMAT_JPEG) {
                jpg_buf = fb->buf;
                jpg_len = fb->len;
            } else {
                bool ok = frame2jpg(fb, 12, &jpg_buf, &jpg_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!ok || !jpg_buf) {
                    ESP_LOGW(TAG, "JPEG convert failed");
                    vTaskDelay(pdMS_TO_TICKS(100));
                    continue;
                }
            }

            if (httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY)) != ESP_OK) break;
            size_t hlen = snprintf(part_buf, sizeof(part_buf), STREAM_PART, (unsigned int)jpg_len);
            if (httpd_resp_send_chunk(req, part_buf, hlen) != ESP_OK) break;
            if (httpd_resp_send_chunk(req, (const char*)jpg_buf, jpg_len) != ESP_OK) break;

            if (fb) {
                esp_camera_fb_return(fb);
                fb = NULL;
            } else if (jpg_buf) {
                free(jpg_buf);
                jpg_buf = NULL;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(frame_interval_ms));
    }

    if (fb) esp_camera_fb_return(fb);
    if (jpg_buf) free(jpg_buf);
    return ESP_OK;
}

static esp_err_t index_handler(httpd_req_t* req) {
    const char* html = "<!DOCTYPE html><html><head><meta charset=utf-8>"
        "<title>RPS Camera</title></head><body><h1>RPS Camera</h1>"
        "<p><a href=\"/stream\">View MJPEG stream</a></p></body></html>";
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, strlen(html));
}

CameraStream::CameraStream() : running(false) {}

void CameraStream::start(StateMachine* stateMachine) {
    s_stateMachine = stateMachine;

#if WIFI_USE_STA
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("[CameraStream] Connecting to WiFi");
    for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[CameraStream] WiFi STA failed, stream disabled");
        return;
    }
    Serial.printf("[CameraStream] STA IP: %s\n", WiFi.localIP().toString().c_str());
#else
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    Serial.printf("[CameraStream] AP SSID: %s, IP: %s\n", WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());
#endif

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 8;

    if (httpd_start(&s_stream_httpd, &config) != ESP_OK) {
        Serial.println("[CameraStream] HTTP server start failed");
        return;
    }

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL
    };
    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(s_stream_httpd, &stream_uri);
    httpd_register_uri_handler(s_stream_httpd, &index_uri);

    running = true;
    ESP_LOGI(TAG, "Stream server started: http://%s/stream", WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString().c_str() : WiFi.localIP().toString().c_str());
}
