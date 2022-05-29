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
}

int main()
{
    //char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";
    char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";
    char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
    char text3[]="[\n    [0, -1, 0],\n\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
    char text4[]="{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n			\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
    char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";
    char mhx[] = "{\"name\" : \"Jack (\\\"Bee\\\") Nimble\"}";
   // do_it(mhx);
    do_Infile("tests/test1");
}


// \"Jack (\\\"Bee\\\") Nimble\"
// "Jack (\"Bee\") Nimble"
