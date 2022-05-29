#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "new_json.h"

void do_it(const char *str)
{
    OBJECT_JSON_S *pstJson = NEWJSON_ParseJsonFormat(str);
    if (!pstJson) {Json_GetErrorInfo(); return;}
    char *ptr = NEWJSON_PrintJsonStr(pstJson, 1);
    if (!ptr) return;
    printf("\r\njson content-> \r\n\n%s\r\n", ptr);
}

void do_Infile(const char *filepath)
{
    FILE *pf = fopen(filepath, "rb");
    char *out;
    if (!pf) return;
    fseek(pf, 0, SEEK_END);
    int len = ftell(pf);
    printf("testT file len = %d\r\n", len);
    fseek(pf, 0, SEEK_SET);
    out = (char *)malloc(len + 1);
    fread(out, sizeof(char), len, pf);
    out[len] = 0;
    printf("src context : %s\r\n\n", out);

    OBJECT_JSON_S *pstJson = NEWJSON_ParseJsonFormat(out);
    if (!pstJson) {Json_GetErrorInfo(); return;}
    char *ptr = NEWJSON_PrintJsonStr(pstJson, 1);

    printf("\r\njson content-> \r\n\n%s\r\n", ptr);
    free(ptr); ptr = 0;
}
struct record {const char *precision;double lat,lon;const char *address,*city,*state,*zip,*country; };

void create_jsonObj()
{
    int i;
    int num[3][3] = {{1, 0, 1}, {0, 0, -1}, {5, 5, 6}};
    int ids[4]={116, 943, 234, 38793};
    struct record fields[2]={
        {"zip",37.7668,-1.223959e+2,"","SAN FRANCISCO","CA","94107","US"},
        {"zip",37.371991,-1.22026e+2,"","SUNNYVALE","CA","94085","US"}};

    OBJECT_JSON_S* json, *fmt, *thm;
    char *ptr;

    json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(json, "format", fmt = cJSON_CreateObject());
    cJSON_AddItemToObject(fmt, "type", cJSON_CreateString("Rect"));

    cJSON_AddStringToObject(fmt, "type", "rect");
    cJSON_AddNumberToObject(fmt, "width", 1080);
    cJSON_AddNumberToObject(fmt, "height", 1920);
    cJSON_AddFalseToObject(fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);

    ptr = NEWJSON_PrintJsonStr(json, 1);
    printf("%s\r\n", ptr);
    free(ptr); ptr = 0;


    OBJECT_JSON_S *tmpArr;
    OBJECT_JSON_S *jsonArrayRoot = cJSON_CreateArray();
    for (i = 0; i < 3; i++)
    {
        cJSON_AddItemToArray(jsonArrayRoot, tmpArr = cJSON_CreateIntArray(num[i], 3));
    }

    ptr = NEWJSON_PrintJsonStr(jsonArrayRoot, 1);
    printf("%s\r\n", ptr);
    free(ptr); ptr = 0;

    /* Our "gallery" item: */
    json=cJSON_CreateObject();
    cJSON_AddItemToObject(json, "\n\tImage", fmt=cJSON_CreateObject());
    cJSON_AddNumberToObject(fmt,"Width",800);
    cJSON_AddNumberToObject(fmt,"Height",600);
    cJSON_AddStringToObject(fmt,"Title","View from 15th Floor");
    cJSON_AddItemToObject(fmt, "Thumbnail", thm=cJSON_CreateObject());
    cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
    cJSON_AddNumberToObject(thm,"Height",125);
    cJSON_AddStringToObject(thm,"Width","100");
    cJSON_AddItemToObject(fmt, "IDs", cJSON_CreateIntArray(ids,4));

    ptr = NEWJSON_PrintJsonStr(json, 1);
    printf("%s\r\n", ptr);
    free(ptr); ptr = 0;

    json=cJSON_CreateArray();
    for (i=0;i<2;i++)
    {
        cJSON_AddItemToArray(json, fmt=cJSON_CreateObject());
        cJSON_AddStringToObject(fmt, "precision", fields[i].precision);
        cJSON_AddNumberToObject(fmt, "Latitude", fields[i].lat);
        cJSON_AddNumberToObject(fmt, "Longitude", fields[i].lon);
        cJSON_AddStringToObject(fmt, "Address", fields[i].address);
        cJSON_AddStringToObject(fmt, "City", fields[i].city);
        cJSON_AddStringToObject(fmt, "State", fields[i].state);
        cJSON_AddStringToObject(fmt, "Zip", fields[i].zip);
        cJSON_AddStringToObject(fmt, "Country", fields[i].country);
    }

    ptr = NEWJSON_PrintJsonStr(json, 1);
    printf("%s\r\n", ptr);
    free(ptr); ptr = 0;
}

int main()
{
    //char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";
    char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";
    char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
    char text3[]="[\n    [0, -1, 0],\n\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
    char text4[]="{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n			\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
    char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";
    char mhx[] = "{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080}}";
   // do_it(mhx);
   // do_Infile("tests/test1");
    create_jsonObj();
}


// \"Jack (\\\"Bee\\\") Nimble\"
// "Jack (\"Bee\") Nimble"
