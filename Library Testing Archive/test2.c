/*
#include<stdio.h>
#include<json-c/json.h>

int main(int argc, char **argv) {
    FILE *fp;
    char buffer[1024];

    struct json_object *parsed_json;
    
    struct json_object *coord;
    struct json_object *weather;
    struct json_object *base;
    struct json_object *main;
    struct json_object *visibility;
    struct json_object *wind;
    struct json_object *clouds;
    struct json_object *dt;
    struct json_object *sys;
    struct json_object *timezone;
    struct json_object *id;
    struct json_object *name;
    struct json_object *cod;

    size_t n_friends;
    size_t i;
    fp = fopen("test2.json", "r");
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    parsed_json = json_tokener_parse(buffer);

    json_object_object_get_ex(parsed_json, "main.temp", &main);

    printf("Value %d\n", json_object_get_int(main));
}
*/


// chat GPT code
#include <stdio.h>
#include <json-c/json.h>

int main(int argc, char **argv) {
    FILE *fp;
    char buffer[1024];

    struct json_object *parsed_json;
    struct json_object *main_obj;
    struct json_object *temp;

    fp = fopen("test2.json", "r");
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    printf("JSON file content:\n%s\n", buffer);

    parsed_json = json_tokener_parse(buffer);

    // First, get the "main" object
    json_object_object_get_ex(parsed_json, "main", &main_obj);
    // Then, get the "temp" field inside the "main" object
    json_object_object_get_ex(main_obj, "temp", &temp);

    printf("Temperature: %.2f\n", json_object_get_double(temp));

    // Free the parsed JSON object
    json_object_put(parsed_json);

    return 0;
}
