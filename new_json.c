#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#include "new_json.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//static 函数在.c源文件里声明
static const char *__parse_JsonNumber(OBJECT_JSON_S *pstItem, const char *pcStrVal);
static const char *__parse_JsonString(OBJECT_JSON_S *pstItem, const char *pcStrVal);
static const char *__parse_JsonArray(OBJECT_JSON_S *pstItem, const char *pcStrVal);
static const char *__parse_JsonObject(OBJECT_JSON_S *pstItem, const char *pcStrVal);
static const char *__parse_JsonValue(OBJECT_JSON_S *pstJson, const char *pcString);
static const char *__print_strdup(const char * str);
static const char *__print_JsonArray(OBJECT_JSON_S *pstJsonObject, int fmt, int depth);
static const char *__print_JsonObject(OBJECT_JSON_S *pstJsonObject, int fmt, int depth);
static const char *__print_FmtJsonValue(OBJECT_JSON_S *pstJsonObject, int fmt, int depth);
static const char *__print_StringObject(OBJECT_JSON_S *pstJsonObject);

#define STRNCMP(str1, R, str2, num) (strncmp(str1, str2, num) R 0)

//////////////////////////////////////////////////////////////////////////////////////////////

static const char *gpcErrorInfo = NULL;

extern const char *Json_GetErrorInfo(void)
{
    return gpcErrorInfo;
}

//跳过不可见字符
static const char* __skip_CannotSeeCharacter(const char *pcValue)
{
    while (pcValue && *pcValue && (unsigned char)*pcValue <= 32 && pcValue++);
    //如果传入'\0'就返回'\0'
    return pcValue;
}

static OBJECT_JSON_S *__GetJsonItem(void)
{
     OBJECT_JSON_S *pstBuf = (OBJECT_JSON_S *)calloc(1, sizeof(OBJECT_JSON_S));
     return pstBuf;
}

//json core
//返回处理完剩余的字串
static const char *__parse_JsonValue(OBJECT_JSON_S *pstJson, const char *pcString)
{
//    char *pvRetVal = NULL;  //不把返回值赋值给局部变量:因为现在只会return掉，增加赋值操作，该接口还需要判断字符串是否解析结束
    if (!pcString) return 0;
    if (STRNCMP("true", ==, pcString, 4)) {
         pstJson->iJsonType = NJSON_TRUE;
         return pcString + 4;
    }
    if (STRNCMP("false", ==, pcString, 4)) {
         pstJson->iJsonType = NJSON_FALSE;
         return pcString + 5;
    }
    if (STRNCMP("null", ==, pcString, 4)) {
         pstJson->iJsonType = NJSON_NULL;
         return pcString + 4;
    }
    if ('{' == *pcString) {
        return __parse_JsonObject(pstJson, __skip_CannotSeeCharacter(pcString));
    }
    if ('[' == *pcString) {
        return __parse_JsonArray(pstJson, __skip_CannotSeeCharacter(pcString));
    }
    if ('\"' == *pcString) {
        return __parse_JsonString(pstJson, pcString);
    }
    if ('-' == *pcString || (*pcString >= '0' && *pcString <= '9')) {
        return __parse_JsonNumber(pstJson, pcString);
    }

    gpcErrorInfo = pcString;
    return 0;
}

static const char *__parse_JsonString(OBJECT_JSON_S *pstItem, const char *pcStrVal)
{
    const char *ptr; char *pcTmp;
    char *out;
    int iStringLen = 0;
    if (!pstItem || !pcStrVal) return 0;

    if ('\"' != *pcStrVal++)
    {
        gpcErrorInfo = pcStrVal;
        return 0;
    }

    pstItem->iJsonType = NJSON_STRING;
    if ('\"' == (*pcStrVal)) return pcStrVal + 1;

    ptr = pcStrVal;
    while (*ptr != '\"' && *ptr && (++iStringLen))   //iStringLen++问题在于先iStringLen = 0;后++
    {
        if ('\\' == (*ptr++)) ptr++;
    }
    pcTmp = out = (char *)malloc(iStringLen + 1);
    if (!out) return 0;

    ptr = pcStrVal;
    while (*ptr != '\"' && *ptr)
    {
        if ('\\' != *ptr) *pcTmp++ = *ptr++;
        else
        {
             ptr++;
             switch (*ptr)    //异常处理
             {
                case 'b': *pcTmp++ = '\b'; ptr++; break;
                case 't': *pcTmp++ = '\t'; ptr++; break;
                case 'f': *pcTmp++ = '\f'; ptr++; break;
                case 'n': *pcTmp++ = '\n'; ptr++; break;
                case 'r': *pcTmp++ = '\r'; ptr++; break;
                default:  *pcTmp++ = *ptr++; break;
             }
        }
    }

    *pcTmp = '\0';
    pstItem->pcValueString = out;
    return ptr + 1;
}

static const char *__parse_JsonNumber(OBJECT_JSON_S *pstItem, const char *pcStrVal)
{
    int signbit = 1, num = 0, subexponent = 0, exponent = 1, exVal = 0;
    if (!pstItem || !pcStrVal) return 0;

    pstItem->iJsonType = NJSON_NUMBER;

    if ('-' == *pcStrVal) signbit = -1, pcStrVal++;
    if ('0' <= *pcStrVal && (*pcStrVal) <= '9')
    {
        do {
            num = num * 10 + (*pcStrVal - '0'), pcStrVal++;
        } while('0' <= *pcStrVal && (*pcStrVal) <= '9');
    }
    if ('.' == *pcStrVal)
    {
        pcStrVal++;
        do {
            num = num * 10 + (*pcStrVal - '0'), pcStrVal++, subexponent--;
        } while('0' <= *pcStrVal && (*pcStrVal) <= '9');
    }
    if ('E' == *pcStrVal || 'e' == *pcStrVal)
    {
        pcStrVal++;
        do {
            if ('-' == (*pcStrVal)) exponent = -1, pcStrVal++;
            else if ('+' == (*pcStrVal)) pcStrVal++;
            else exVal = exVal * 10 + (*pcStrVal - '0'), ++pcStrVal;
        } while('0' <= *pcStrVal && (*pcStrVal) <= '9');
    }

    pstItem->dNumberDouble = signbit * num * pow(10, (subexponent + exponent * exVal));
    pstItem->iNumber = (int)pstItem->dNumberDouble;
    return pcStrVal;
}

static const char* __parse_JsonArray(OBJECT_JSON_S *pstItem, const char *pcStrVal)
{
    OBJECT_JSON_S *pstChild = NULL;
    OBJECT_JSON_S *pstBrother = NULL;

    if (!pstItem || !pcStrVal || ('[' != *pcStrVal)) return 0;

    pstItem->iJsonType = NJSON_ARRAY;

    pcStrVal = __skip_CannotSeeCharacter(pcStrVal + 1);
    if (']' == *pcStrVal) return pcStrVal + 1;

    pstItem->pstChild = pstChild = __GetJsonItem();
    if (!pstChild) return 0;

    pcStrVal = __parse_JsonValue(pstChild, __skip_CannotSeeCharacter(pcStrVal));  //递归入口 [[
    if (!pcStrVal) return 0;

    while (',' == *pcStrVal)
    {
        pstChild->pstNext = pstBrother = __GetJsonItem();
        if (!pstBrother) return 0;
        pstBrother->pstPrev = pstChild; pstChild = pstBrother;

        pcStrVal = __parse_JsonValue(pstBrother, __skip_CannotSeeCharacter(pcStrVal + 1));
        if (!pcStrVal) return 0;
    }

    pcStrVal = __skip_CannotSeeCharacter(pcStrVal);
    if (']' == *pcStrVal) return pcStrVal + 1;

    gpcErrorInfo = pcStrVal;
    return 0;
}

static const char *__parse_JsonObject(OBJECT_JSON_S *pstItem, const char *pcStrVal)
{
    OBJECT_JSON_S *pstChild, *pstBrother;
    if (!pstItem || !pcStrVal || ('{' != *pcStrVal)) return 0;
    pcStrVal = __skip_CannotSeeCharacter(pcStrVal + 1);
    //printf("%s\r\n", pcStrVal);
    pstItem->iJsonType = NJSON_OBJECT;
    if ('}' == *pcStrVal) return (pcStrVal + 1);
    pstItem->pstChild = pstChild = __GetJsonItem();
    if (!pstChild) return 0;

    pcStrVal = __parse_JsonString(pstChild, __skip_CannotSeeCharacter(pcStrVal));
    pcStrVal = __skip_CannotSeeCharacter(pcStrVal);  //冒号之前可能存在空格之类，不需要的字符
    if (!pcStrVal)
    {
        return 0;
    }
    pstChild->pcKeyString = pstChild->pcValueString, pstChild->pcValueString = 0;

    if (':' == *pcStrVal)
    {
        pcStrVal = __parse_JsonValue(pstChild, __skip_CannotSeeCharacter(pcStrVal + 1));
        if (!pcStrVal) return 0;
    }

    while (',' == *pcStrVal)
    {
        pstChild->pstNext = pstBrother = __GetJsonItem();
        if (!pstBrother) return 0;
        pstBrother->pstPrev = pstChild; pstChild = pstBrother;
        pcStrVal = __parse_JsonString(pstBrother, __skip_CannotSeeCharacter(pcStrVal + 1));
        if (!pcStrVal) return 0;

        pstBrother->pcKeyString = pstBrother->pcValueString, pstBrother->pcValueString = 0;

        if (':' == *pcStrVal) pcStrVal = __parse_JsonValue(pstBrother, __skip_CannotSeeCharacter(pcStrVal + 1));
        else gpcErrorInfo = pcStrVal;

        if (!pcStrVal) return 0;
    }

    pcStrVal = __skip_CannotSeeCharacter(pcStrVal);
    if ('}' == *pcStrVal) return pcStrVal + 1;

    gpcErrorInfo = pcStrVal;
    return 0;
}

extern OBJECT_JSON_S *NEWJSON_ParseJsonFormat(const char *str)
{
    OBJECT_JSON_S *pstJson = __GetJsonItem();
    if (!pstJson) return 0;

    str = __parse_JsonValue(pstJson, __skip_CannotSeeCharacter(str));
    //printf("Json_GetErrorInfo = %s\r\n", str);
    if (!str) return 0;
   // printf("Json_GetErrorInfo = %s\r\n", Json_GetErrorInfo());
    return pstJson;
}


static const char *__print_JsonString(char *src, int fmt)
{
    //if (!src) return 0;
    int stringlen = 3, flag = 0;
    char *ptr = src;
    char *outString = 0, *out;

    if (!ptr)
    {
        char cTmp[3] = "\"\"";
        if (!(outString = (char *)malloc(stringlen))) return 0;
        strcpy(outString, cTmp);
        return outString;
    }

    for (; ptr && *ptr; ptr++)
    {
        if ('\\' == *ptr || (*ptr > 0 && *ptr < 32) || '\"' == *ptr)
        {
            flag = 1;
        }
    }

    if (!flag)
    {
        //while (ptr && *ptr && stringlen++ && ptr++);
        int len = ptr - src;
        char *out = (char *)malloc(stringlen + len);
        if (!out) return 0;
        outString = out;
        *out++ = '\"';
        strcpy(out, src);
        out[len] = '\"';
        out[len + 1] = '\0';
        return outString;
    }

    ptr = src;

    while (*ptr)
    {
        stringlen++;
        if (strchr("\"\\\r\n\t\f\b", *ptr))
        {
            stringlen++;
        }
        else if (*ptr < 32)
        {
            stringlen += 5;
        }

        ptr++;
    }

    ptr = src;
    out = outString = (char *)malloc(stringlen);
    if (!outString) return 0;
    *outString++ = '\"';

    // n\tame
    while (*ptr)
    {
        if (*ptr >= 32 && '\"' != *ptr && '\\' != *ptr) *outString++ = *ptr++;
        else
        {
            *outString++ = '\\';
            switch (*ptr)
            {
            case '\b': *outString++ = 'b'; break;
            case '\r': *outString++ = 'r'; break;
            case '\n': *outString++ = 'n'; break;
            case '\t': *outString++ = 't'; break;
            case '\f': *outString++ = 'f'; break;
            case '\"': *outString++ = '"'; break;
            case '\\': *outString++ = '\\'; break;
            default:  *outString++ = *ptr; break;
            }
            ptr++;
        }
    }

    *outString++ = '\"';
    *outString = '\0';
    return out;
}

static const char *__print_JsonNumber(OBJECT_JSON_S *pstJsonObject, int fmt)
{
    if (!pstJsonObject) return 0;
    double doublenum = pstJsonObject->dNumberDouble;
    int intNum = pstJsonObject->iNumber;
    char *ptr;
    char *out = 0;

    if (0 == doublenum)
    {
        if (!(out = (char *)calloc(1, 2 * sizeof(char)))) return 0;
        strcpy(out, "0");
    }
    else if (fabs(doublenum - intNum) < DBL_EPSILON) //整型
    {
        if (!(out = (char *)calloc(1, 12 * sizeof(char)))) return 0;
        snprintf(out, 12, "%d", intNum);
    }
    else
    {
        if (!(out = (char *)calloc(1, 64 * sizeof(char)))) return 0;
        if (fabs(doublenum - intNum) < DBL_EPSILON && fabs(doublenum) < 1.0e60)
        {

            snprintf(out, 64, "%.0f", doublenum);
        }
        else if (fabs(doublenum) < 1.0e-6 || fabs(doublenum) > 1.0e+9)
        {
            snprintf(out, 64, "%e", doublenum);
        }
        else
        {
            snprintf(out, 64, "%f", doublenum);
        }
    }

    return out;
}

static const char *__print_strdup(const char * str)
{
    char *pstSrc;
    if (!str) return;

    pstSrc = (char *)malloc(strlen(str) + 1);
    if (!pstSrc) return 0;
    memcpy(pstSrc, str, strlen(str) + 1);
    return pstSrc;
}


static const char *__print_JsonArray(OBJECT_JSON_S *pstJsonObject, int fmt, int depth)
{
    int len = 3, FullArrayLen=0, i, memcpylen;
    char *out, *ret, *ptr;
    char **ppcHoldFullArray;
    OBJECT_JSON_S *pstChild = pstJsonObject->pstChild;

    if (!pstJsonObject) return 0;
    if (!pstJsonObject->pstChild)
    {
        if (!(out = (char *)malloc(sizeof(char) * 3))) return 0;
        strcpy(out, "[]");
        out[2] = 0;
        return out;
    }

    while (pstChild)
    {
        FullArrayLen++;
        pstChild = pstChild->pstNext;
    }

    ppcHoldFullArray = (char **)malloc(sizeof(char**) * FullArrayLen);
    if (!ppcHoldFullArray) return 0;
    pstChild = pstJsonObject->pstChild;

    for (i = 0; i < FullArrayLen; i++)
    {
        ret = __print_FmtJsonValue(pstChild, fmt, depth);
        if (!ret) return 0;
        ppcHoldFullArray[i] = ret;

        len += strlen(ret) + (fmt?1:0) + 2;
        pstChild = pstChild->pstNext;
    }

    out = (char *)malloc(len);
    if (!out) return 0;

    *out = '['; ptr = out+1; *ptr = '\0';
    for (i = 0; i < FullArrayLen; i++)
    {
        memcpylen = strlen(ppcHoldFullArray[i]);
        memcpy(ptr, ppcHoldFullArray[i], memcpylen);
        ptr += memcpylen;
        if (i != FullArrayLen - 1)
        {
            *ptr++ = ',';
            if (fmt) *ptr++ = ' ';
        }
        free((void *)ppcHoldFullArray[i]);
        ppcHoldFullArray[i] = 0;
    }

    free((void **)ppcHoldFullArray);
    ppcHoldFullArray = 0;

    *ptr++ = ']';
    *ptr = '\0';
    return out;
}

static const char *__print_StringObject(OBJECT_JSON_S *pstJsonObject)
{
    if (!pstJsonObject) return 0;

    return __print_JsonString(pstJsonObject->pcValueString, 1);
}

static const char *__print_JsonObject(OBJECT_JSON_S *pstJsonObject, int fmt, int depth)
{
    int outlen = 6, brotherNum = 0, i=0, j, entryLen;
    int fail = 0, depthLen = 0;
    char *out, *ptr, *ret;
    char **entries, **enumentries;
    OBJECT_JSON_S *pstChild;

    if (!pstJsonObject) return 0;
    pstChild = pstJsonObject->pstChild;
    if (!pstChild)
    {
        ptr = out = (char *)malloc(outlen);
        if (!out) return 0;
        if (depth > 0 && depth <= 1)
        {
            *out++ = '{';
            if (fmt) *out++ = '\n';
            *out++ = '}';
            *out = '\0';
            return ptr;
        }

        *out++ = '{'; if (fmt) *out++ = '\n';
        for (i = 0; i < depth; i++)
        {
            if (fmt)
            {
                *out++ = '\t';
            }
        }
        *out++ = '}';
        *out = '\0';
        return ptr;
    }

    while (pstChild)
    {
        brotherNum++;
        pstChild = pstChild->pstNext;
    }

    entries = (char **)malloc(sizeof(char **) * brotherNum);
    if (!entries) return 0;
    enumentries = (char **)malloc(sizeof(char **) * brotherNum);
    if (!enumentries) return 0;

    pstChild = pstJsonObject->pstChild;
    for (j = 0; j < depth; j++)
    {
        depthLen++;
    }

    while (pstChild && (!fail))
    {
        ret = __print_JsonString(pstChild->pcKeyString, 1);
        if (ret) entries[i] = ret;
        else
        {
            fail = 1;
        }

        ret = __print_FmtJsonValue(pstChild, 1, depth + 1);
        if (ret) enumentries[i] = ret;
        else
        {
            fail = 1;
        }

        // 3 认为是 两个'\n' 一个'\t'
        outlen += strlen(entries[i]) + strlen(enumentries[i]) + (fmt?3:0) + 2 * depthLen - 1;
        i++;
        pstChild = pstChild->pstNext;
    }

  //  printf("json object len = %d -> %d\r\n", outlen, depth);
    ptr = out = (char *)malloc(outlen);
    if (!out) fail = 1;

    if (fail)
    {
        for (i = 0; i < brotherNum; i++)
        {
            if (entries[i])
            {
                free(enumentries[i]);
            }

            if (enumentries[i])
            {
                free(enumentries[i]);
            }
        }
        free(enumentries);
        free(entries);
        return 0;
    }

    *ptr++ = '{';
    if (fmt) *ptr++ = '\n';
    *ptr  = '\0';

    for (i = 0; i < brotherNum; i++)
    {
        if (fmt)
        {
            for (j = 0; j < depth; j++)
            {
                *ptr++ = '\t';
            }
        }

        entryLen = strlen(entries[i]);
        memcpy(ptr, entries[i], entryLen);
        ptr += entryLen;
        free(entries[i]);

        *ptr++ = ':'; if (fmt) *ptr++ = '\t';

        entryLen = strlen(enumentries[i]);
        memcpy(ptr, enumentries[i], entryLen);
        ptr += entryLen;
        free(enumentries[i]);

        if (i != brotherNum - 1) *ptr++ = ',';
        if (fmt) *ptr++ = '\n';
    }

    free(enumentries);
    free(entries);

    for (i = 0; i < depth - 1; i++)
    {
        *ptr++ = '\t';
    }

    *ptr++ = '}';
    *ptr = '\0';

    return out;
}

static const char *__print_FmtJsonValue(OBJECT_JSON_S *pstJsonObject, int fmt, int depth)
{
    if (!pstJsonObject) return 0;

    switch (pstJsonObject->iJsonType & 0xf)  //低4位数据
    {
    case NJSON_STRING: return __print_StringObject(pstJsonObject);
    case NJSON_NUMBER: return __print_JsonNumber(pstJsonObject, 1);
    case NJSON_FALSE:  return __print_strdup("false");
    case NJSON_TRUE:   return __print_strdup("true");
    case NJSON_NULL:   return __print_strdup("null");
    case NJSON_ARRAY:  return __print_JsonArray(pstJsonObject, fmt, depth);
    case NJSON_OBJECT: return __print_JsonObject(pstJsonObject, fmt, depth);
    default: return NULL;
    }
}


static void __delete_jsonObject(OBJECT_JSON_S *pstJsonObject)
{
    OBJECT_JSON_S *pstNext;
    OBJECT_JSON_S *pstCur = pstJsonObject;

    if (!pstJsonObject) return;

    while (pstCur)
    {
        if (pstCur->pstChild) __delete_jsonObject(pstCur->pstChild);
        pstNext = pstCur;
        pstCur = pstCur->pstNext;
        if (pstNext->pcKeyString) free(pstNext->pcKeyString);
        if (pstNext->pcValueString) free(pstNext->pcValueString);
        if (pstNext) free(pstNext);
        pstNext = 0;
    }
}

extern char *NEWJSON_PrintJsonStr(OBJECT_JSON_S *pstJsonObject, int fmt)
{
    char *ptr = __print_FmtJsonValue(pstJsonObject, fmt, 1);
    __delete_jsonObject(pstJsonObject);
    return ptr;
}























