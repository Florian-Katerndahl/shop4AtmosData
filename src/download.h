#ifndef CAMS_DOWNLOAD_H
#define CAMS_DOWNLOAD_H

#include <stdbool.h>

#define __USE_XOPEN

#include <time.h>

#include <curl/curl.h>
#include <jansson.h>

#define FORCE_VERSION "Test, Test!"

typedef enum {
    PRODUCT_STATUS_COMPLETED = 0,
    PRODUCT_STATUS_QUEUED = 1,
    PRODUCT_STATUS_RUNNING = 2,
    PRODUCT_STATUS_FAILED = 3,
    PRODUCT_STATUS_INVALID = 9999,
} PRODUCT_STATUS;

typedef enum {
    ADS_STATUS_OK = 0,
    ADS_STATUS_WARNING = 1
} ADS_STATUS;

typedef enum {
    PRODUCT_CAMS_REPROCESSED = 0,
    PRODUCT_CAMS_COMPOSITION_FORECAST = 1,
} PRODUCT_TYPE;

typedef enum {
    SENSING_TIME_00 = 0,
    SENSING_TIME_03 = 1,
    SENSING_TIME_06 = 2,
    SENSING_TIME_09 = 3,
    SENSING_TIME_12 = 4,
    SENSING_TIME_15 = 5,
    SENSING_TIME_18 = 6,
    SENSING_TIME_21 = 7,
} SENSING_TIME;

enum {
    NPOW2 = 4,
    NPOW4 = 16,
    NPOW6 = 64,
    NPOW8 = 256,
    NPOW10 = 1024,
    NPOW12 = 4096,
    NPOW14 = 16384,
    NPOW16 = 65536,
    NPOW18 = 262144,
    NPOW20 = 1048576,
    NPOW22 = 4194304,
    NPOW24 = 16777216,
    NPOW26 = 67108864,
    NPOW28 = 268435456,
    NPOW30 = 1073741824
};

/**
 * @brief
 * @author Florian Katerndahl
 */
struct DATE_RANGE {
    struct tm start;
    struct tm end;
};

/**
 * @brief
 */
struct OPTIONS {
    int use_custom_authentication;
    char authentication[NPOW16];
    char coordinates[NPOW16];
    char output_directory[NPOW16];
};

/**
 * @struct BOUNDING_BOX
 * @brief
 * @author Florian Katerndahl
 */
struct BOUNDING_BOX {
    int area_subset;
    int north;
    int east;
    int south;
    int west;
};

/**
 * @struct API_AUTHENTICATION
 * @brief struct containing all variables/fields defined in the `.cdsapirc` file
 * @author Florian Katerndahl
 */
struct API_AUTHENTICATION {
    char *base_url;     ///< Base url for API requests
    char *user;         ///< UID
    char *secret;       ///< secret used for authentication
    int verify;        ///< verify the tls-ssl connection?
};

/**
 * @brief
 * @author Florian Katerndahl
 */
struct CLIENT {
    struct API_AUTHENTICATION auth;
    int quiet __attribute__((unused));                          ///< logging not used for now
    int debug __attribute__((unused));                          ///< debugging not used for now
    unsigned int timeout;
    int progress __attribute__((unused));                       ///< display progress bar
    int full_stack __attribute__((unused));
    int delete;
    unsigned int max_retries;
    unsigned int max_sleep;
    unsigned int last_state;
    int wait_until_complete;
    char *metadata __attribute__((unused));
    int forget __attribute__((unused));
    unsigned int retries;
    CURL **curl_handle;
};

/**
 * @brief Simple struct to write responses from server to, combining the data itself as well as its size.
 * @author Florian Katerndahl
 */
struct CURL_DATA {
    char *data;
    size_t length;
};

/**
 * @brief
 * @author Florian Katerndahl
 */
struct ADS_STATUS_RESPONSE {
    struct CURL_DATA curl_string;
};

/**
 * @brief
 * @author Florian Katerndahl
 */
struct PRODUCT_REQUEST {
    PRODUCT_TYPE product;
    struct BOUNDING_BOX bbox;
    char *variable;
    struct DATE_RANGE dates;
    //SENSING_TIME time;
    char *format;
    size_t time_length;
    SENSING_TIME time[8];
    size_t leadtime_length;
    long leadtime_hour[120];
};

struct PRODUCT_RESPONSE {
    PRODUCT_STATUS state;
    char *id;
    char *location;
    size_t length;

};

/**
 * @brief Print the usage of the `download-cams` program
 * @author Florian Katerndahl
 */
void print_usage(void);

/**
 * @brief Print the version of FORCE
 * @author Florian Katerndahl
 */
void print_version(void);

/**
 * @brief Print the program's purpose
 * @author Florian Katerndahl
 */
void print_purpose(void);

/**
 * @brief Parse API authentication file and save its result into `api_authentication`
 * @param api_authentication_file File handle to authentication file
 * @param api_authentication struct containing the fields extracted from `api_authentication_file`
 * @author Florian Katerndahl
 */
void parse_authentication(FILE *api_authentication_file, struct API_AUTHENTICATION *api_authentication);

/**
 * @brief Read the file holding center coordinates of all WRS-2 tiles the AOI contains. Find the bounding box (in
 * decimal degrees) which encompasses all tiles. The resulting bounding box is buffered by computing either the floor
 * or ceiling of the edges.
 * @param coordinate_file Path to file containing center coordinates of WRS-2 tiles for which the ADS-query should
 * be performed.
 * @return Bounding box which encapsulates all WRS-2 cells whose center coordinates are in `cooridnate_file`.
 * @note The file should contain two columns separated by white space, and no header. The first column should give the
 * longitude (X), the second column the latitude (Y) with coordinates in decimal degree
 * (negative values for West/South). Any other column is ignored.
 * @author Florian Katerndahl
 */
struct BOUNDING_BOX parse_coordinate_file(char *coordinate_file, double **lon, double **lat);

/**
 * @brief Convert string representation of product to integer representation of type PRODUCT_TYPE
 * @param str Pointer to string denoting which product to download. Currently implemented: REPROCESS, FORECAST
 * @return Integer representation of product
 * @author Florian Katerndahl
 */
PRODUCT_TYPE product_string_to_type(const char *str);

/**
 * @brief Small wrapper function which which dynamically tries to open a custom authentication file or tries to retrieve
 * the environment variable `ADSAUTH`.
 * @param api_authentication struct holding fields from API authentication file. To be populated
 * @param options struct containing parsed command-line arguments; used to decide from where to retrieve authentication file.
 * @return Zero on success
 * @warning Implementation does not allow non-zero exit codes!
 * @author Florian Katerndahl
 */
int init_api_authentication(struct API_AUTHENTICATION *api_authentication, const struct OPTIONS *options);

/**
 * @brief Free allocated strings in struct `API_AUTHENTICATION`
 * @param api_authentication Pointer to struct `API_AUTHENTICATION`
 * @author Florian Katerndahl
 */
void free_api_authentication(const struct API_AUTHENTICATION *api_authentication);

/**
 * @brief Given the optopt code, which ist set be getopt, return a pointer to the string (i.e. long) version of that argument.
 * Used for error reporting.
 * @param dest Pointer to set
 * @param optopt `optopt` code from getopt_long
 * @return `dest` pointing to a static character string
 * @warning This could easily get out of sync with the rest of the program. Terminates program with exit code 129 if
 * no optcode could be matched
 */
const char *reverse_code_optopt(const char *dest, int optopt);

/**
 * @brief Given the path to a file, checks against flags specified.
 * @param fname File path
 * @param flags Flags to check against (e.g. F_OK | R_OK)
 * @return True if all checks pass, false otherwise.
 * @author Florian Katerndahl
 */
bool validate_file(const char *fname, int flags);

/**
 * @brief Given the path to a directory, checks if the path resolves and it is indeed a directory.
 * @param dname Directory path
 * @return True if all checks pass, false otherwise.
 * @author Florian Katerndahl
 */
bool validate_directory(const char *dname);

/**
 * @brief Given the result of a curl request/performance, print the accompanying description from the libcurl docs
 * and possibly halt the program execution.
 * @param res Return value curl_easy_perform
 * @param stop_execution Should the execution be stopped if an error is detected that was not caught by CURL (i.e.
 * when the HTTP code is not >= 400).
 * @author Florian Katerndahl
 */
void interpret_curl_result(CURLcode res, int stop_execution);

/**
 * @brief Initialize curl handle with options specified in API authentication.
 * @param handle Curl handle
 * @param client client object containing various options
 * @author Florian Katerndahl
 */
void init_curl_handle(CURL **handle, const struct CLIENT *client);

/**
 * @brief Check the status of the Atmospheric Data Store
 * @param handle Curl handle
 * @return ADS_STATUS_OK if no warnings are reported, ADS_STATUS_WARNING otherwise
 * @author Florian Katerndahl
 */
ADS_STATUS check_ads_status(CURL **handle, const struct CLIENT *client);

/**
 * @brief Function passed to CURLOPT_WRITEFUNCTION to save strings in memory.
 * @param message Pointer to the data delivered data.
 * @param size Always 1.
 * @param nmemb Size of `content`, i.e. the size of the data which gets passed to this function.
 * @param data_container_p Pointer to struct which resembles the data structure passed into this function. Actual struct
 * into which data shall be saved needs to be set with CURLOPT_WRITEDATA.
 * @return Bytes handled, i.e. size * nmemb.
 * @warning The data delivered by cURL is not null-terminated. The function must null-terminate the data on each call.
 * @author Florian Katerndahl
 */
size_t write_curl_string(char *message, size_t size, size_t nmemb, void *data_container_p);

/**
 * @brief Generic version of `write_curl_string`. Here, the lsat byte is not set to zero.
 * @param message Pointer to the data delivered data
 * @param size Always 1
 * @param n Size of `content`, i.e. the size of the data which gets passed to this function.
 * @param data_container_p Pointer to struct which resembles the data structure passed into this function. Actual struct
 * into which data shall be saved needs to be set with CURLOPT_WRITEDATA.
 * @return Bytes handled, i.e. size * nmemb.
 * @author Florian Katerndahl
 */
size_t write_curl_generic(char *message, size_t size, size_t n, void *data_container_p);

/**
 * @brief Translate a SENSING_TIME type to its string representation.
 * @param time Time which should be translated.
 * @return Pointer to string representation.
 * @author Florian Katerndahl
 */
const char *time_as_string(SENSING_TIME time);

/**
 * @brief Translate a long value to its SENSING_TIME representation while also checking validity.
 * @param v Long which should be translated.
 * @return SENSING_TIME representation
 * @author Florian Katerndahl
 */
SENSING_TIME long_to_time(long v);

/**
 * @brief Construct a string in JSON format which can is passed onto CURL for a product POST request.
 * @param request Pointer to request struct
 * @return A pointer to the JSON formatted string. The caller is responsible for freeing the string after usage!
 * @note The CDS-API allows for multiple model base times, this current implementation only allows for a single base
 * time per request.
 * @warning The caller is responsible for freeing the string after usage!
 * @author Florian Katerndahl
 */
const char *assemble_request(const struct PRODUCT_REQUEST *request);

/**
 * @brief Given a download directory and the request parameters, generate a suitable download path.
 * @param request Pointer to request struct
 * @param options Pointer to options struct
 * @return A pointer to the formatted download path
 * @warning The caller is responsible for freeing the string after usage!
 * @author Florian Katerndahl
 */
const char *assemble_download_path(const struct PRODUCT_REQUEST *request, const struct OPTIONS *options);

/**
 * @brief Post product request to ADS-API
 * @param request Request struct containing query options, representing JSON being sent to ADS
 * @param handle cURL handle
 * @param client Client struct
 * @return Response struct containing a selection of JSON response fields
 * @note Currently, only 'cams-global-reanalysis-eac4' is implemented.
 * @author Florian Katerndahl
 */
struct PRODUCT_RESPONSE
ads_request_product(const struct PRODUCT_REQUEST *request, CURL **handle, const struct CLIENT *client);

/**
 * @brief Check if a given date range conforms to semantic meaning.
 * @param dates Pointer to struct DATE_RANGE
 * @return True, if START_DATE <= END_DATE, false otherwise
 * @author Florian Katerndahl
 */
bool constrain_dates(struct DATE_RANGE *dates);

/**
 * @brief Convert string representation of product status in ADS-response to enum
 * @param status Product status as string
 * @return Product status as integer
 * @author Florian Katerndahl
 */
PRODUCT_STATUS convert_to_product_status(const char *status);

/**
 * @brief Query the API endpoint if product status is 'finished' and downloads the data product
 * @param response Response struct
 * @param handle cURL handle
 * @param client Client struct
 * @param fp Character string, representing absolute file path where to save file
 * @return Integer-encoded status. Zero on success
 * @warning Only a zero exit code is currently possible
 * @author Florian Katerndahl
 */
int ads_download_product(struct PRODUCT_RESPONSE *response, CURL **handle, struct CLIENT *client, const char *fp);

/**
 * @brief Query the ADS API to check for the product status
 * @param response Response struct
 * @param handle cURL handle
 * @param client Client struct
 * @author Florian Katerndahl
 */
void ads_check_product_state(struct PRODUCT_RESPONSE *response, CURL **handle, struct CLIENT *client);

/**
 * @brief Send a request to the ADS API to delete a product request
 * @param response Response struct
 * @param handle cURL handle
 * @param client Client struct
 * @return Integer-encoded status. Zero on success
 * @note Implementation does not allow for non-zero exit code!
 * @author Florian Katerndahl
 */
int ads_delete_product_request(struct PRODUCT_RESPONSE *response, CURL **handle, struct CLIENT *client);

#endif //CAMS_DOWNLOAD_H
