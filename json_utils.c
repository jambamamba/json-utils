#include <json_utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug_logger.h>
#include <unistd.h>

LOG_CATEGORY(JSON, "JSON")

JsonNameValuePair *nameValuePairsFromJsonArray(const char *json_array_string)
{
    JsonNameValuePair *pairs = NULL;
    if(json_array_string && *json_array_string) {
        cJSON *json_array = cJSON_Parse(json_array_string);
        cJSON *json_obj = NULL;
        int num_pairs = 0;
        cJSON_ArrayForEach(json_obj, json_array) {
            pairs = (JsonNameValuePair*)realloc((JsonNameValuePair*)pairs, (num_pairs +1) * sizeof(JsonNameValuePair));
            char* x = cJSON_Print(json_obj);
            cJSON *json_child_obj = NULL;
            cJSON_ArrayForEach(json_child_obj, json_obj) {
                pairs[num_pairs]._name = json_child_obj->string ? strdup(json_child_obj->string) : NULL;
                pairs[num_pairs]._value = json_child_obj->valuestring ? strdup(json_child_obj->valuestring) : NULL;
                break;
            }
            num_pairs++;
        }
        pairs = (JsonNameValuePair*)realloc((JsonNameValuePair*)pairs, (num_pairs +1) * sizeof(JsonNameValuePair));
        pairs[num_pairs]._name = NULL;
        pairs[num_pairs]._value = NULL;
    }
    return pairs;
}
void freeNameValuePairs(JsonNameValuePair *pairs)
{
    if(!pairs) {
        return;
    }
    for (int i = 0;; ++i) {
        JsonNameValuePair *pair = &pairs[i];
        if(!pair->_name) {
            break;
        }
        if(pair->_name) {
            free((void *)pair->_name);
        }
        if(pair->_value) {
            free((void *)pair->_value);
        }
    }
    free((void *)pairs);
}
int countNameValuePairs(JsonNameValuePair *pairs)
{
    int num_pairs = 0;
    for(;pairs && pairs[num_pairs]._name; num_pairs++){}
    return num_pairs;
}
int intFromJsonArray(const char *json_array, const char *token_name, int default_value, unsigned int max_range)
{
    int value = default_value;
    if(json_array && *json_array && token_name && *token_name) {
        cJSON *json = cJSON_Parse(json_array);
        cJSON *json_token = NULL;
        cJSON_ArrayForEach(json_token, json) {
            cJSON *json_value = cJSON_GetObjectItemCaseSensitive(json_token, token_name);
            if(json_value) {
                value = intFromJsonValue(json_value, max_range);
                break;
            }
        }
    }
    return value;
}

const char *stringFromJsonArray(const char *json_array, const char *token_name)
{
    if(json_array && *json_array && token_name && *token_name) {
        cJSON *json = cJSON_Parse(json_array);
        cJSON *json_token = NULL;
        cJSON_ArrayForEach(json_token, json) {
            cJSON *json_value = cJSON_GetObjectItemCaseSensitive(json_token, token_name);
            if(json_value && cJSON_IsString(json_value)){
                return json_value->valuestring;
            }
        }
    }
    return NULL;
}

char *loadJsonFromFile(const char *const json_file)
{
    FILE *fp = fopen(json_file, "rt");
    if (!fp){
        LOG(FATAL, JSON,  "Cannot find file to load: '%s'\n", json_file);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int file_sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (file_sz <= 0){
        LOG(WARNING, JSON, "Invalid file size of '%i' for gui json file: '%s'\n", file_sz, json_file);
        return NULL;
    }
    char *json_string = (char *)malloc(file_sz);
    int res = fread(json_string, file_sz, 1, fp);
    (void)res;
    fclose(fp);

    return json_string;
}

void storeJsonToFile(char* json, const char* file)
{
    LOG(DEBUG, JSON,  "Storing data into json file : '%s'\n", file);
    FILE *fp = fopen(file, "wb");
    if(fp)
    {
        LOG(DEBUG, JSON,  "%s\n", json);
        fwrite(json, strlen(json), 1, fp);
        fclose(fp);
    }
    else
    {
        LOG(FATAL, JSON,  "Cannot save json file : '%s'\n", file);
    }
    free(json);
}

int intFromJsonValue(const cJSON *json_value, unsigned int max_range)
{
    if(json_value) {
        if(cJSON_IsNumber(json_value)){
            return json_value->valueint;
        }
        else if(cJSON_IsString(json_value)){
            if(json_value->valuestring[strlen(json_value->valuestring)-1] == '%') {
                char *buffer = alloca(strlen(json_value->valuestring));
                strcpy(buffer, json_value->valuestring);
                buffer[strlen(json_value->valuestring)-1] = 0;
                if(max_range == 0) {
                    LOG(FATAL, JSON, "max_range must not be zero when using percentage values\n");
                }
                return atoi(buffer) * max_range / 100;
            }
            return atoi(json_value->valuestring);
        }
    }
    LOG(WARNING, JSON, "failed to parse json value\n");
    return max_range;
}

double doubleFromJsonValue(const cJSON *json_value, double max_range)
{
    if(json_value) {
        if(cJSON_IsNumber(json_value)){
            return json_value->valuedouble;
        }
        else if(cJSON_IsString(json_value)){
            if(json_value->valuestring[strlen(json_value->valuestring)-1] == '%') {
                char *buffer = alloca(strlen(json_value->valuestring));
                strcpy(buffer, json_value->valuestring);
                buffer[strlen(json_value->valuestring)-1] = 0;
                if(max_range == 0) {
                    LOG(FATAL, JSON, "max_range must not be zero when using percentage values\n");
                }
                return atof(buffer) * max_range / 100;
            }
            return atof(json_value->valuestring);
        }
    }
    LOG(WARNING, JSON, "failed to parse json value\n");
    return max_range;
}

int colorFromJsonValue(const cJSON *json_value, int default_value)
{
    if(json_value && 
        cJSON_IsString(json_value) &&
        json_value->valuestring[0] == '#') {
        char *buffer = &json_value->valuestring[1];
        return strtol(buffer, NULL, 16);
    }

    return default_value;
}

cJSON* readJson(const char* data_json_file)
{
    cJSON* data_json = NULL;
    if (access(data_json_file, F_OK) == 0) {
        char *data_json_string = loadJsonFromFile(data_json_file);
        data_json = cJSON_Parse(data_json_string);
        free(data_json_string);
    } else {
        LOG(WARNING, JSON, "Could not find json file\n");
    }

    if (data_json == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
        
        if (error_ptr != NULL){
            LOG(FATAL, JSON, "Error before: %s\n", error_ptr);
        }
    }

    return data_json;
}