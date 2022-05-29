#ifndef __NEW_JSON_H__
#define __NEW_JSON_H__

#pragma once
#define JSON_SUCCESS 0
#define JSON_FAILED  1

typedef enum NewJsonType_tag
{
    NJSON_FALSE = 0,
    NJSON_TRUE  = 1,
    NJSON_NULL  = 2,
    NJSON_OBJECT= 3,
    NJSON_ARRAY = 4,
    NJSON_STRING= 5,
    NJSON_NUMBER= 6
} NEW_JSON_E;

typedef struct json_tag
{
    int iJsonType;

    int iNumber;           //整型数字值
    double dNumberDouble;  //浮点型数字值

    char *pcKeyString;
    char *pcValueString;
    struct json_tag *pstPrev, *pstNext;
    struct json_tag *pstChild;
} OBJECT_JSON_S;


extern const char *Json_GetErrorInfo(void);
extern OBJECT_JSON_S *NEWJSON_ParseJsonFormat(const char *str);
extern char *NEWJSON_PrintJsonStr(OBJECT_JSON_S *pstJsonObject, int fmt);
#endif


