#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <cJSON.h>

typedef struct JsonNameValuePairT {
    const char* _name;
    const char* _value;
}JsonNameValuePair;

char *loadJsonFromFile(const char *const json_file);
void storeJsonToFile(char* json, const char* file);
int intFromJsonValue(const cJSON *json_value, unsigned int max_range);
int intFromJsonArray(const char *json_array, const char *token_name, int default_value, unsigned int max_range);
double doubleFromJsonValue(const cJSON *json_value, double max_range);
int colorFromJsonValue(const cJSON *json_value, int default_value);
const char *stringFromJsonArray(const char *json_array, const char *token_name);
JsonNameValuePair *nameValuePairsFromJsonArray(const char *json_array);
void freeNameValuePairs(JsonNameValuePair *pairs);
int countNameValuePairs(JsonNameValuePair *pairs);
cJSON* readJson(const char* data_json_file);

#ifdef __cplusplus
} //extern "C"
#endif