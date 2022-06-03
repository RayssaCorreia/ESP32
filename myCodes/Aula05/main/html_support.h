#ifndef WEB_SUPPORT_H_
#define WEB_SUPPORT_H_

void SPIFFS_Directory(char * path);
esp_err_t SPIFFS_Mount(char * path, char * label, int max_files);

esp_err_t Text2Html(httpd_req_t *req, char * filename);
int32_t calcBase64EncodedSize(int origDataSize);
esp_err_t Image2Base64(char * imageFileName, char * base64FileName);
esp_err_t Image2Html(httpd_req_t *req, char * filename, char * type);

#endif