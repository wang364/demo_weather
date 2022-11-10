#include <stdio.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "jsmn.h"

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}

static int displayContent(const char *json, jsmntok_t *t, int count)
{
    int i = 0;
    /* Loop over all keys of the root object */
    for (i = 1; i < count; i++)
    {
        if (jsoneq(json, &t[i], "city") == 0)
        {
            /* We may use strndup() to fetch string value */
            printf("- City: %.*s\n", t[i + 1].end - t[i + 1].start,
                   json + t[i + 1].start);
            i++;
        }
        else if (jsoneq(json, &t[i], "tem") == 0)
        {
            /* We may additionally check if the value is either "true" or "false" */
            printf("- Temperature: %.*s\n", t[i + 1].end - t[i + 1].start,
                   json + t[i + 1].start);
            i++;
        }
        else if (jsoneq(json, &t[i], "humidity") == 0)
        {
            printf("- Humidity: %.*s\n", t[i + 1].end - t[i + 1].start,
                   json + t[i + 1].start);
            i++;
        }
        else if (jsoneq(json, &t[i], "win") == 0)
        {
            /* We may want to do strtol() here to get numeric value */
            printf("- Wind: %.*s\n", t[i + 1].end - t[i + 1].start,
                   json + t[i + 1].start);
            i++;
        }
        else if (jsoneq(json, &t[i], "win_speed") == 0)
        {
            int j;
            printf("- Wind speed: %.*s\n", t[i + 1].end - t[i + 1].start,
                   json + t[i + 1].start);
            i++;
        }
        else if (jsoneq(json, &t[i], "air") == 0)
        {
            int j;
            printf("- Air: %.*s\n", t[i + 1].end - t[i + 1].start,
                   json + t[i + 1].start);
            i++;
        }
        else
        {
            continue;
        }
    }
}

/*
 * The output looks like YAML.
 */
static int dump(const char *js, jsmntok_t *t, int count, int indent) {
  int i, j, k;
  jsmntok_t *key;
  if (count == 0) {
    return 0;
  }
  if (t->type == JSMN_PRIMITIVE) {
    printf("%.*s", t->end - t->start, js + t->start);
    return 1;
  } else if (t->type == JSMN_STRING) {
    printf("'%.*s'", t->end - t->start, js + t->start);
    return 1;
  } else if (t->type == JSMN_OBJECT) {
    printf("\n");
    j = 0;
    for (i = 0; i < t->size; i++) {
      for (k = 0; k < indent; k++) {
        printf("  ");
      }
      key = t + 1 + j;
      j += dump(js, key, count - j, indent + 1);
      if (key->size > 0) {
        printf(": ");
        j += dump(js, t + 1 + j, count - j, indent + 1);
      }
      printf("\n");
    }
    return j + 1;
  } else if (t->type == JSMN_ARRAY) {
    j = 0;
    printf("\n");
    for (i = 0; i < t->size; i++) {
      for (k = 0; k < indent - 1; k++) {
        printf("  ");
      }
      printf("   - ");
      j += dump(js, t + 1 + j, count - j, indent + 1);
      printf("\n");
    }
    return j + 1;
  }
  return 0;
}

// 解析天气数据
void ParseWeatherInnfo(char* wInfo){
    jsmn_parser parser;
    jsmntok_t tokens[128];
    int ret;
    int i;

    jsmn_init(&parser);

    ret = jsmn_parse(&parser, wInfo, strlen(wInfo), tokens, 128);
    if (ret < 0)
    {
       printf("A error has happened, errno is:%d.\n", ret);
       //return;
    }

    dump(wInfo, tokens, ret, 0);
    displayContent(wInfo, tokens, 29);
}

// 接收数据回调函数
static size_t receive_data(void *data, size_t size, size_t nmemb, void *writer)
{
    char* test  = (char* )writer;
    char weatherInfo[1024];

    memset(weatherInfo,0x0,sizeof(1024));
    strncpy(weatherInfo,(char *)data,strlen((char *)data));
    ParseWeatherInnfo(weatherInfo);
    return size*nmemb; 
}


// 获取天气数据
void getWeatherInfo(){
    char tt[64] = "12345";
    int ret;
    CURL *curlhandle = NULL;

    curlhandle = curl_easy_init();
    if (NULL == curlhandle) {
        printf("[curl test]: curl_easy_init failed.\n");
        goto ERROR_RESULT;
    }
 
    ret = curl_easy_setopt(curlhandle, CURLOPT_URL, "https://www.yiketianqi.com/free/day?appid=51259866&appsecret=h1HlEGoR&unescape=1&cityid=101010100");  //这里是获取固定城市的天气
    if (ret != CURLE_OK) {
        printf("[curl test]: set curl url failed.\n");
        goto ERROR_RESULT;
    }
   
    curl_easy_setopt(curlhandle, CURLOPT_SSL_VERIFYPEER, false);   // 不验证数字证书
    
    ret = curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, tt); // 这里是CURLOPT_WRITEFUNCTION的第四个参数
    ret = curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, receive_data); // 回调函数
    if (ret != CURLE_OK) {
        printf("[curl test]: write curl data failed.\n");
        goto ERROR_RESULT;
    }

    ret = curl_easy_perform(curlhandle);
    if (ret != CURLE_OK) {
        printf("curl_easy_perform error---ret is %d.\n",ret);
        goto ERROR_RESULT;
    }

    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();
    return;


    ERROR_RESULT:
        curl_easy_cleanup(curlhandle);
        curl_global_cleanup();
        return;    
}

int main()
{
    getWeatherInfo();
    return 0;
}