#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>

#define __USE_XOPEN

#include <time.h>

#include <curl/curl.h>
#include <jansson.h>
#include "download.h"
#include "sort.h"

void print_usage(void) {
    printf(
        "Usage: cams-download [-h|--help] [-v|--version] [-i|--purpose] [-c|--coordinates] "
        "[-o|--output_directory] [-t|--daily_tables] [-s|--climatology] <--start> <--end> <-a|--authentication>\n\n"
        "[-h|--help]\t\tprint this help page and exit\n"
        "[-v|--version]\t\tprint version\n"
        "[-i|--purpose]\t\tshow program's purpose\n\n"
        "[-c|--coordinates]\t...\n"
        "[-o|--output_directory]\t...\n"
        "[-t|--daily_tables]\tbuild daily tables? Default: false\n"
        "[-s|--climatology]\tbuild climatology? Default: false\n\n"
        "Optional arguments:\n"
        "<--start>\t\tStart date. Default: 1970-01-01.\n"
        "<--end>\t\tStart date. Default: 1970-01-01.\n"
        "<-a|--authentication>\toptional...\n");
}

void print_version(void) {
    printf("FORCE version: %s\n", FORCE_VERSION);
}

void print_purpose(void) {
    printf("Download ECMWF CAMS data from the Atmosphere Data Store\n");
}


void parse_authentication(FILE *api_authentication_file, struct API_AUTHENTICATION *api_authentication) {
    char line[NPOW16];
    char *separator;
    char *needle_p;

    while (!feof(api_authentication_file)) {
        fgets(line, NPOW16, api_authentication_file);
        line[strlen(line) - 1] = '\0';
        if ((needle_p = strstr(line, "url")) != NULL) {
            while (*needle_p != ' ') needle_p++;

            needle_p++;

            api_authentication->base_url = calloc(NPOW16, sizeof(char));

            strncpy(api_authentication->base_url, needle_p, NPOW16); // automatically zero terminated because of calloc
        } else if ((needle_p = strstr(line, "key")) != NULL) {
            while (*needle_p != ' ') needle_p++;

            needle_p++;

            api_authentication->user = calloc(NPOW8, sizeof(char));

            api_authentication->secret = calloc(NPOW12, sizeof(char));

            separator = index(needle_p, ':');

            assert(separator - needle_p < (NPOW8 - 1));

            strncpy(api_authentication->user, needle_p,
                    separator - needle_p); // automatically zero terminated because of calloc

            separator++;

            strncpy(api_authentication->secret, separator, NPOW12); // automatically zero terminated because of calloc
        } else if ((needle_p = strstr(line, "verify")) != NULL) {
            while (*needle_p != ' ') needle_p++;

            needle_p++;

            api_authentication->verify = (int) strtol(needle_p, NULL, 10);
        } else {
            fprintf(stderr, "Error: Failed to parse line: %s\n", line);
            exit(EXIT_FAILURE);
        }
    }
}

struct BOUNDING_BOX parse_coordinate_file(char *coordinate_file, double **lon, double **lat) {
    // TODO I'd like to try implementing some sorting algorithm, which ISN'T the Bubblesort!
    // TODO currently, this returns the bbox of center coordinates; However, I want the bbox of the WRS-2 tiles whose
    //  center coordinates were given.
    static char line[NPOW8];

    char *line_p = (char *) &line;

    size_t n_longitude = 0, n_latitude = 0;

    double *longitude = malloc(sizeof(double));
    double *latitude = malloc(sizeof(double));

    FILE *f = fopen(coordinate_file, "rt");

    if (f == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", coordinate_file);
        abort();
    }

    while ((line_p = fgets(line, NPOW8, f)) != NULL) {

        line_p[strlen(line_p) - 1] = '\0';

        longitude = realloc(longitude, (n_longitude + 1) * sizeof(double));

        latitude = realloc(latitude, (n_latitude + 1) * sizeof(double));

        if (longitude == NULL || latitude == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory while parsing line %s\n", line);
            abort();
        }

        if (sscanf(line, "%lf %lf %*s", longitude + n_longitude, latitude + n_latitude) != 2)
            break;

        n_longitude++;

        n_latitude++;
    }

    fclose(f);

    sort_double(longitude, n_longitude, bubble);

    sort_double(latitude, n_latitude, bubble);

    *lon = longitude;

    *lat = latitude;

    return (struct BOUNDING_BOX) {
        .north = (int) ceil(latitude[n_latitude - 1]), .east = (int) ceil(longitude[n_longitude - 1]),
        .south = (int) floor(latitude[0]), .west = (int) floor(longitude[0])
    };
}

int init_api_authentication(struct API_AUTHENTICATION *api_authentication, const struct OPTIONS *options) {
    FILE *fp;
    char *ads_env;

    if (options->use_custom_authentication)
        fp = fopen(options->authentication, "rt");
    else {
        if ((ads_env = getenv("ADSAUTH")) == NULL) {
            fprintf(stderr, "Error: Environment variable 'ADSAUTH' not set\n");
            exit(EXIT_FAILURE);
        }

        fp = fopen(ads_env, "rt");
    }

    if (fp == NULL)
        return 1;

    parse_authentication(fp, api_authentication);

    fclose(fp);

    return 0;
}

void free_api_authentication(const struct API_AUTHENTICATION *api_authentication) {
    free(api_authentication->base_url);
    free(api_authentication->user);
    free(api_authentication->secret);
}

const char *reverse_code_optopt(const char *dest, int optopt) {
    switch (optopt) {
        case 'h':
            dest = "help";
            break;
        case 'v':
            dest = "version";
            break;
        case 'p':
            dest = "purpose";
            break;
        case 'a':
            dest = "authentication";
            break;
        case 'c':
            dest = "coordinates";
            break;
        case 'o':
            dest = "output_directory";
            break;
        case '0':
            dest = "start";
            break;
        case '1':
            dest = "end";
            break;
        default:
            exit(129);
    }

    return dest;
}

bool validate_file(const char *fname, int flags) {
    if (access(fname, flags) != 0)
        return false;

    return true;
}

bool validate_directory(const char *dname) {
    struct stat sb = {0};

    if (stat(dname, &sb) == 0 && !S_ISDIR(sb.st_mode))
        return false;

    return true;
}

void interpret_curl_result(CURLcode res, int stop_execution) {
    // TODO implement more error checks, and maybe split function to different use cases?
    switch (res) {
        case CURLE_OK:
            break;
        case CURLE_URL_MALFORMAT:
            fprintf(stderr, "The URL you passed to libcurl was not properly formatted.\n");
            break;
        case CURLE_COULDNT_RESOLVE_HOST:
            fprintf(stderr, "Could not resolve host.\n");
            break;
        case CURLE_COULDNT_CONNECT:
            fprintf(stderr, "Could not connect to host or proxy.\n");
            break;
        case CURLE_REMOTE_ACCESS_DENIED:
            fprintf(stderr, "Remote Access to resource denied by server.\n");
            break;
        case CURLE_HTTP_RETURNED_ERROR:
            fprintf(stderr, "Server returned HTTP code >= 400\n");
            break;
        case CURLE_OPERATION_TIMEDOUT:
            fprintf(stderr, "Connection timed out. Retry with higher timeout limit\n");
            break;
        default:
            fprintf(stderr, "Unknown CURL error: %d\n", res);
            break;
    }
    stop_execution ? exit(EXIT_FAILURE) : (void) 0;
}

void init_curl_handle(CURL **handle, const struct CLIENT *client) {
    curl_easy_setopt(*handle, CURLOPT_USERNAME, client->auth.user);

    curl_easy_setopt(*handle, CURLOPT_PASSWORD, client->auth.secret);

    curl_easy_setopt(*handle, CURLOPT_SSL_VERIFYHOST, client->auth.verify);

    curl_easy_setopt(*handle, CURLOPT_FAILONERROR, 1L);

    curl_easy_setopt(*handle, CURLOPT_TIMEOUT, (long) client->timeout);
}

ADS_STATUS check_ads_status(CURL **handle, const struct CLIENT *client) {
    ADS_STATUS return_val;
    // TODO reduce NPOW12 to NPOW6
    char url[NPOW12];
    int url_status;

    struct ADS_STATUS_RESPONSE ads_status_response = {0};

    json_t *root;
    json_error_t error;
    json_t *warning;

    if ((url_status = snprintf(url, NPOW12, "%s/%s", client->auth.base_url, "status.json")) < 0 ||
        url_status >= NPOW12) {
        fprintf(stderr, "Error: Failed to create url for ADS status check.\n"
                        "Return value of snprintf: %d\n", url_status);
        exit(EXIT_FAILURE);
    }

    init_curl_handle(handle, client);

    curl_easy_setopt(*handle, CURLOPT_URL, url);

    curl_easy_setopt(*handle, CURLOPT_WRITEFUNCTION, write_curl_string);

    curl_easy_setopt(*handle, CURLOPT_WRITEDATA, (void *) &ads_status_response);

    CURLcode res = curl_easy_perform(*handle);
    interpret_curl_result(res, 0);

    // TODO some memory corruption happening? Writing further than needed/allowed?
    //ads_status_response.curl_string.data[ads_status_response.curl_string.length - 1] = '\0';

    root = json_loads(ads_status_response.curl_string.data, 0, &error);

    if (!root) {
        fprintf(stderr, "Error: Failed to parse JSON response on line %d: %s.\n", error.line, error.text);
        exit(EXIT_FAILURE);
    }

    if (json_is_object(root)) {
        warning = json_object_get(root, "warning");
        // TODO RTFM!
        if (!json_is_array(warning)) {
            fprintf(stderr, "Error: Expected a JSON array. Got passed other data structure.\n");
            json_decref(root);
            exit(EXIT_FAILURE);
        }

        if (warning != NULL && json_array_size(warning) != 0)
            return_val = ADS_STATUS_WARNING;
    }

    json_decref(root);

    free(ads_status_response.curl_string.data);

    curl_easy_reset(*handle);

    return return_val ? return_val : ADS_STATUS_OK;
}

size_t write_curl_string(char *message, size_t size, size_t nmemb, void *data_container_p) {
    size_t message_length = size * nmemb;

    struct CURL_DATA *in_memory_string = (struct CURL_DATA *) data_container_p;

    char *message_data_p = realloc(in_memory_string->data, in_memory_string->length + message_length + 1);

    if (message_data_p == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for string.\n");
        exit(EXIT_FAILURE);
    }

    in_memory_string->data = message_data_p;

    memcpy(&(in_memory_string->data[in_memory_string->length]), message, message_length);

    in_memory_string->length += message_length;

    in_memory_string->data[in_memory_string->length] = '\0';

    return message_length;
}

size_t write_curl_generic(char *message, size_t size, size_t n, void *data_container_p) {
    size_t message_length = size * n;

    struct CURL_DATA *data = (struct CURL_DATA *) data_container_p;

    char *new_data_p = realloc(data->data, data->length + message_length);

    if (new_data_p == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for CURL write-back.\n");
        exit(EXIT_FAILURE);
    }

    data->data = new_data_p;

    memcpy(&(data->data[data->length]), message, message_length);

    data->length += message_length;

    return message_length;
}

struct PRODUCT_RESPONSE
ads_request_product(PRODUCT_TYPE product, __attribute__((unused)) const struct PRODUCT_REQUEST *request,
                    CURL **handle, const struct CLIENT *client) {
    // TODO reduce NPOW12 to NPOW8
    char *product_name;
    char url[NPOW12];
    int url_status;

    struct ADS_STATUS_RESPONSE ads_retrieve_response = {0};
    struct PRODUCT_RESPONSE request_response = {0};
    request_response.state = PRODUCT_STATUS_INVALID;

    json_t *root, *state, *request_id, *location, *content_length;
    json_error_t error;
    json_t *warning;

    switch (product) {
        case PRODUCT_CAMS_REPROCESSED:
            product_name = "cams-global-reanalysis-eac4";
            break;
        default:
            break;
    }

    if ((url_status = snprintf(url, NPOW12, "%s/resources/%s", client->auth.base_url, product_name)) < 0 ||
        url_status >= NPOW12) {
        fprintf(stderr, "Error: Failed to assemble request url\n");
        exit(EXIT_FAILURE);
    }

    init_curl_handle(handle, client);

    curl_easy_setopt(*handle, CURLOPT_POST, 1L);

    curl_easy_setopt(*handle, CURLOPT_URL, url);

    curl_easy_setopt(*handle, CURLOPT_WRITEFUNCTION, write_curl_string);
    curl_easy_setopt(*handle, CURLOPT_WRITEDATA, (void *) &ads_retrieve_response);

    struct curl_slist *list = NULL;
    list = curl_slist_append(list, "Content-Type: application/json");
    curl_easy_setopt(*handle, CURLOPT_HTTPHEADER, list);

    // "{'variable': 'total_aerosol_optical_depth_469nm', 'date': '2003-01-01/2003-01-01', 'time': '00:00', 'format': 'grib'}"
    char *d = "{'variable': 'total_aerosol_optical_depth_469nm', 'date': '2002-01-01/2010-01-09', 'time': '12:00', 'format': 'grib'}";
    curl_easy_setopt(*handle, CURLOPT_POSTFIELDS, d);

    CURLcode res = curl_easy_perform(*handle);
    interpret_curl_result(res, 0);

    // TODO some memory corruption happening? Writing further than needed/allowed?
    //ads_retrieve_response.curl_string.data[ads_retrieve_response.curl_string.length] = '\0';

    /* Example of returned JSON
     * {
     *   "state": "completed",
     *   "request_id": "a3a00819-ee46-470c-9403-bd842b20828a",
     *   "location": "https://download-0000-ads-clone.copernicus-climate.eu/cache-compute-0000/cache/data0/adaptor.mars.internal-1682585725.0681317-9181-12-fe1c1646-57d5-431a-b7cf-913ea09f554f.grib",
     *   "content_length": 2155863600,
     *   "content_type": "application/x-grib",
     *   "result_provided_by": "d447a679-22bf-4a39-95be-39c5280c6772",
     *   "specific_metadata_json": {
     *     "top_request_origin": "api"
     *   }
     * }
     */
    if (!(root = json_loads(ads_retrieve_response.curl_string.data, 0, &error))) {
        fprintf(stderr, "Error: Failed to parse JSON response on line %d: %s.\n", error.line, error.text);
        exit(EXIT_FAILURE);
    }

    if (!json_is_object(root)) {
        fprintf(stderr, "Error: Returned JSON is not an object.\n");
        goto cleanup;
    }

    state = json_object_get(root, "state");
    if (!json_is_string(state)) {
        fprintf(stderr, "Error: Could not get state from JSON message.\n");
        goto cleanup;
    }
    request_response.state = convert_to_product_status(json_string_value(state));

    request_id = json_object_get(root, "request_id");
    if (!json_is_string(request_id)) {
        fprintf(stderr, "Error: Could not get request_id from JSON message.\n");
        goto cleanup;
    }
    // TODO check result of malloc and strcpy!!!
    // TODO is +1 needed? IDK
    request_response.id = calloc(json_string_length(request_id) + 1, sizeof(char));
    strcpy(request_response.id, json_string_value(request_id));

    if (request_response.state != PRODUCT_STATUS_COMPLETED)
        goto cleanup;

    location = json_object_get(root, "location");
    if (!json_is_string(location)) {
        fprintf(stderr, "Error: Could not get file location from JSON message.\n");
        goto cleanup;
    }

    content_length = json_object_get(root, "content_length");
    if (!json_is_integer(content_length)) {
        fprintf(stderr, "Error: Could not get content length from JSON message.\n");
        goto cleanup;
    }
/*
    content_type = json_object_get(root, "content_type");
    if (!json_is_string(content_type)) {
        fprintf(stderr, "Error: Could not get content_type from JSON message.\n");
        goto cleanup;
    }
*/

    // TODO check result of malloc and strcpy!!!
    // TODO is +1 needed? IDK
    request_response.location = calloc(json_string_length(location) + 1, sizeof(char));
    strcpy(request_response.location, json_string_value(location));
    request_response.length = json_integer_value(content_length);

    cleanup:
    json_decref(root);
    free(ads_retrieve_response.curl_string.data);
    curl_easy_reset(*handle);
    curl_slist_free_all(list);

    return request_response;
}

bool constrain_dates(struct DATE_RANGE *dates) {
    return mktime(&dates->start) <= mktime(&dates->end);
}

PRODUCT_STATUS convert_to_product_status(const char *status) {
    if (strcmp(status, "completed") == 0) return PRODUCT_STATUS_COMPLETED;
    if (strcmp(status, "queued") == 0) return PRODUCT_STATUS_QUEUED;
    if (strcmp(status, "running") == 0) return PRODUCT_STATUS_RUNNING;
    if (strcmp(status, "failed") == 0) return PRODUCT_STATUS_FAILED;

    return PRODUCT_STATUS_INVALID;
}

int ads_download_product(struct PRODUCT_RESPONSE *response, CURL **handle, struct CLIENT *client, const char *fp) {
    printf("Downloading %.2lf MB from %s\n", ((double) response->length) * 0.000001, response->location);

    struct CURL_DATA data_product = {0};

    init_curl_handle(handle, client);

    curl_easy_setopt(*handle, CURLOPT_URL, response->location);
    curl_easy_setopt(*handle, CURLOPT_WRITEFUNCTION, &write_curl_generic);
    curl_easy_setopt(*handle, CURLOPT_WRITEDATA, (void *) &data_product);

    CURLcode res = curl_easy_perform(*handle);
    interpret_curl_result(res, 0);

    curl_easy_reset(*handle);

    if (data_product.length != response->length) {
        fprintf(stderr, "Error: Received different amount of bytes from than promised."
                        "Expected %ld, got %ld\n",
                response->length, data_product.length);
        exit(EXIT_FAILURE); // TODO could be made a return with some exit code instead (need to free resources first)
    }

    FILE *f = fopen(fp, "wb");

    if (f == NULL) {
        fprintf(stderr, "Error: Could not open file %s.\n", fp);
        exit(EXIT_FAILURE); // TODO could be made a return with some exit code instead (need to free resources first)
    }

    if (fwrite(data_product.data, sizeof(char), data_product.length, f) != data_product.length * sizeof(char)) {
        fprintf(stderr, "Error: Could not write entire data stream to file.\n");
        exit(EXIT_FAILURE); // TODO could be made a return with some exit code instead (need to free resources first)
    }

    free(data_product.data);
    fclose(f);

    return 0;
}

void ads_check_product_state(struct PRODUCT_RESPONSE *response, CURL **handle, struct CLIENT *client) {
    struct CURL_DATA status_response = {0};
    // TODO reduce NPOW12 to NPOW8
    char url[NPOW12];
    int url_status;

    if ((url_status = snprintf(url, NPOW12, "%s/tasks/%s", client->auth.base_url, response->id)) < 0 ||
        url_status >= NPOW12) {
        fprintf(stderr, "Error: Failed to assemble request url\n");
        exit(EXIT_FAILURE);
    }

    init_curl_handle(handle, client);

    curl_easy_setopt(*handle, CURLOPT_URL, url);
    curl_easy_setopt(*handle, CURLOPT_WRITEFUNCTION, &write_curl_string);
    curl_easy_setopt(*handle, CURLOPT_WRITEDATA, (void *) &status_response);

    CURLcode res = curl_easy_perform(*handle);
    interpret_curl_result(res, 0);

    curl_easy_reset(*handle);

    /* Example of returned JSON => identical to initial request!
     * {
     *   "state": "completed",
     *   "request_id": "a3a00819-ee46-470c-9403-bd842b20828a",
     *   "location": "https://download-0000-ads-clone.copernicus-climate.eu/cache-compute-0000/cache/data0/adaptor.mars.internal-1682585725.0681317-9181-12-fe1c1646-57d5-431a-b7cf-913ea09f554f.grib",
     *   "content_length": 2155863600,
     *   "content_type": "application/x-grib",
     *   "result_provided_by": "d447a679-22bf-4a39-95be-39c5280c6772",
     *   "specific_metadata_json": {
     *     "top_request_origin": "api"
     *   }
     * }
     */

    json_t *root, *state, *location, *content_length;
    json_error_t error;
    json_t *warning;

    if (!(root = json_loads(status_response.data, 0, &error))) {
        fprintf(stderr, "Error: Failed to parse JSON response on line %d: %s.\n", error.line, error.text);
        exit(EXIT_FAILURE);
    }

    if (!json_is_object(root)) {
        fprintf(stderr, "Error: Returned JSON is not an object.\n");
        goto cleanup;
    }

    state = json_object_get(root, "state");
    if (!json_is_string(state)) {
        fprintf(stderr, "Error: Could not get state from JSON message.\n");
        goto cleanup;
    }
    response->state = convert_to_product_status(json_string_value(state));

    if (response->state != PRODUCT_STATUS_COMPLETED)
        goto cleanup;

    location = json_object_get(root, "location");
    if (!json_is_string(location)) {
        fprintf(stderr, "Error: Could not get file location from JSON message.\n");
        goto cleanup;
    }

    content_length = json_object_get(root, "content_length");
    if (!json_is_integer(content_length)) {
        fprintf(stderr, "Error: Could not get content length from JSON message.\n");
        goto cleanup;
    }

    response->length = json_integer_value(content_length);

    size_t curr_buff_size = response->location == NULL ? 0 : strlen(response->location);
    size_t file_location_size = strlen(json_string_value(location));

    if (curr_buff_size < file_location_size) {
        char *new_data_p = realloc(response->location, sizeof(char) * (curr_buff_size + file_location_size + 1));

        if (new_data_p == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for download path on server.\n");
            exit(EXIT_FAILURE);
        }

        response->location = new_data_p;
    }

    memset(response->location, 0, strlen(response->location) + 1);
    // TODO check result of strcpy!
    strcpy(response->location, json_string_value(location));

    cleanup:
    json_decref(root);
    free(status_response.data);
}

int ads_delete_product_request(struct PRODUCT_RESPONSE *response, CURL **handle, struct CLIENT *client) {
    // TODO reduce NPOW12 to NPOW8
    char url[NPOW12];
    int url_status;

    if ((url_status = snprintf(url, NPOW12, "%s/tasks/%s", client->auth.base_url, response->id)) < 0 ||
        url_status >= NPOW12) {
        fprintf(stderr, "Error: Failed to assemble URL to delete product from ADS.\n");
        exit(EXIT_FAILURE);
    }

    init_curl_handle(handle, client);

    curl_easy_setopt(*handle, CURLOPT_URL, url);
    curl_easy_setopt(*handle, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(*handle);
    interpret_curl_result(res, 0);

    curl_easy_reset(*handle);

    return 1;
}
