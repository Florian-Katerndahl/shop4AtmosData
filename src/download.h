#ifndef CAMS_DOWNLOAD_H
#define CAMS_DOWNLOAD_H

#include <stdbool.h>

#define __USE_XOPEN

#include <time.h>

#include <curl/curl.h>
#include <jansson.h>

#define FORCE_VERSION "Test, Test!"

typedef enum {
    ADS_STATUS_OK = 0,
    ADS_STATUS_WARNING = 1
} ADS_STATUS;

typedef enum {
    PRODUCT_CAMS_REPROCESSED = 0,
} PRODUCT_TYPE;

typedef enum {
    TIME_00 = 0,
    TIME_03 = 1,
    TIME_06 = 2,
    TIME_09 = 3,
    TIME_12 = 4,
    TIME_15 = 5,
    TIME_18 = 6,
    TIME_21 = 7,
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
    int quiet;                          ///< logging not used for now
    int debug;                          ///< debugging not used for now
    unsigned int timeout;
    int progress;                       ///< display progress bar
    int full_stack;
    int delete;
    unsigned int max_retries;
    unsigned int max_sleep;
    unsigned int last_state;
    int wait_until_complete;
    char *metadata;
    int forget;
    CURL **curl_handle;
};

/**
 * @brief
 * @author Florian Katerndahl
 */
struct CURL_STRING {
    char *string;
    size_t length;
};

/**
 * @brief
 * @author Florian Katerndahl
 */
struct ADS_STATUS_RESPONSE {
    struct CURL_STRING curl_string;
};

/**
 * @brief
 * @author Florian Katerndahl
 */
struct PRODUCT_REQUEST {
    struct BOUNDING_BOX bbox;
    char *variable;
    struct DATE_RANGE dates;
    SENSING_TIME time;
    char *format;
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
 */
struct BOUNDING_BOX parse_coordinate_file(char *coordinate_file, double **lon, double **lat);

/**
 * @brief
 * @param api_authentication
 * @param options
 * @return
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
 * @brief
 * @param dest
 * @param optopt
 * @return
 * @warning This could easily get out of sync with the rest of the program. Terminates program with exit code 129 if
 * no optcode could be matched
 */
const char *reverse_code_optopt(const char *dest, int optopt);

/**
 * Given the path to a file, checks against flags specified.
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
 * @warning The data is not null-terminated. The function must null-terminate the data on each call.
 * @author Florian Katerndahl
 */
size_t write_curl_string(char *message, size_t size, size_t nmemb, void *data_container_p);

/**
 * @brief Main entry point to CDS API
 * @param product_name
 * @param request
 * @param download_path
 * @param handle
 * @param client
 * @author Florian Katerndahl
 */
void ads_retrieve(PRODUCT_TYPE product, const struct PRODUCT_REQUEST *request, const char *download_path, CURL **handle,
                  const struct CLIENT *client);

/**
 * @brief
 * @author Florian Katerndahl
 */
void ads_post_product_request();

/**
 * @brief Check if a given date range conforms to semantic meaning.
 * @param dates Pointer to struct DATE_RANGE
 * @return True, if START_DATE <= END_DATE, false otherwise
 * @author Florian Katerndahl
 */
bool constrain_dates(struct DATE_RANGE *dates);

#endif //CAMS_DOWNLOAD_H
