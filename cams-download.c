#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>

#define __USE_XOPEN

#include <time.h>

#include <curl/curl.h>
#include <assert.h>
#include "src/download.h"

#define DEBUG

#ifdef DEBUG
#define NO_GETOPT_ERROR_OUTPUT 0
#else
#define NO_GETOPT_ERROR_OUTPUT 1
#endif // DEBUG

int main(int argc, char *argv[]) {
    char error_string[NPOW16];

    char *time_conversion_result;

    char time_formats[NPOW2][NPOW8] = {
        "%F",
        "%D",
        "%d.%m.%Y",
        "%d/%m/%Y"
    };

    int optid;

    int option_index = 0; // will only be set, if parsing was successful

    opterr = NO_GETOPT_ERROR_OUTPUT ? 0 : 1;

    static struct OPTIONS options = {0};
    bool use_area_subset = false;

    static struct API_AUTHENTICATION api_authentication = {0};

    struct CLIENT client = (struct CLIENT) {
        .auth = {0},
        .quiet = 1,
        .debug = 0,
        .timeout = 1800, // cdsapi sets it to 60, to low when downloading 2 GB
        .progress = 0,
        .full_stack = 0,
        .delete = 0,
        .max_retries = 10,
        .max_sleep = 60,
        .last_state = 0,
        .wait_until_complete = 1,
        .metadata = NULL,
        .forget = 0,
        .retries = 0,
        .curl_handle = NULL
    };

    static struct PRODUCT_REQUEST request = {0};

    CURLcode curl;

    CURL *handle;

    __attribute__((unused)) CURLcode res;

    // populate default/non-user-changeable-options
    request.dates.start = (struct tm) {.tm_year = 103, .tm_mon = 0, .tm_mday = 1};
    request.dates.end = (struct tm) {.tm_year = 103, .tm_mon = 0, .tm_mday = 1};
    request.product = PRODUCT_CAMS_REPROCESSED;
    request.variable = "total_aerosol_optical_depth_469nm";
    request.format = "grib";
    request.time = SENSING_TIME_00;

    static struct option long_options[] = {
        {"help",             no_argument,       NULL, 'h'},
        {"version",          no_argument,       NULL, 'v'},
        {"purpose",          no_argument,       NULL, 'i'},
        {"authentication",   required_argument, NULL, 'a'},
        {"coordinates",      required_argument, NULL, 'c'},
        {"start",            required_argument, NULL, '0'},
        {"end",              required_argument, NULL, '1'},
        {"product",         required_argument, NULL, '2'},
        {"output_directory", required_argument, NULL, 'o'},
        {0,                  0,                 0,    0}
    };

    // TODO why can I remove a letter from shortopts and still match the short version?
    while ((optid = getopt_long_only(argc, argv, "+:hvia:c:o:012:", long_options, &option_index)) != -1) {
        switch (optid) {
            case 0: // getopt_long returns `val` if flag == NULL; otherwise 0 (in which case it stores val in flag)
                break;
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            case 'i':
                print_purpose();
                exit(EXIT_SUCCESS);
            case 'a':
                if (optarg == 0) {
                    // if text is present, optarg points to it; otherwise it is set to 0
                    fprintf(stderr, "You shouldn't be able to reach this code!!\n");
                }
                options.use_custom_authentication = 1;
                strncpy(options.authentication, optarg, NPOW16);
                break;
            case 'c':
                if (optarg == 0) {
                    // if text is present, optarg points to it; otherwise it is set to 0
                    fprintf(stderr, "You shouldn't be able to reach this code!!\n");
                }
                use_area_subset = true;
                strncpy(options.coordinates, optarg, NPOW16);
                break;
            case 'o':
                if (optarg == 0) {
                    // if text is present, optarg points to it; otherwise it is set to 0
                    fprintf(stderr, "You shouldn't be able to reach this code!!\n");
                }
                strncpy(options.output_directory, optarg, NPOW16);
                break;
            case '0': {
                int failed_attempts = 0;
                for (int i = 0; i < NPOW2; ++i) {
                    time_conversion_result = strptime(optarg, time_formats[i], &request.dates.start);
                    if (time_conversion_result != NULL) break;
                    failed_attempts++;
                }
                if (failed_attempts == NPOW2) {
                    fprintf(stderr, "ERROR: Failed to parse date \"%s\"\n", optarg);
                    exit(EXIT_FAILURE);
                }
            }
                break;
            case '1': {
                int failed_attempts = 0;
                for (int i = 0; i < NPOW2; ++i) {
                    time_conversion_result = strptime(optarg, time_formats[i], &request.dates.end);
                    if (time_conversion_result != NULL) break;
                    failed_attempts++;
                }
                if (failed_attempts == NPOW2) {
                    fprintf(stderr, "ERROR: Failed to parse date \"%s\"\n", optarg);
                    exit(EXIT_FAILURE);
                }
            }
                break;
            case '2':
                if (optarg == 0) {
                    // if text is present, optarg points to it; otherwise it is set to 0
                    fprintf(stderr, "You shouldn't be able to reach this code!!\n");
                }
                request.product = product_string_to_type(optarg);
                break;
            case ':':
                fprintf(stderr, "Error: expected option for argument -%c/-%s is missing\n", optopt,
                        reverse_code_optopt(error_string, optopt));
                exit(EXIT_FAILURE);
            case '?': // fall through
            default:
                fprintf(stderr, "Error: got unexpected argument \"%c\"\n\n", optopt);
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    // after all parsing is done, the variable optind indexes into argv to point to the first non-option argument
    // that is: an entry in argv, which is not caught by the option-struct above
    while (optind < argc) {
        fprintf(stderr, "Warning: Excess argument \"%s\" is ignored.\n", argv[optind]);
        optind++;
    }

    if (!constrain_dates(&request.dates)) {
        fprintf(stderr, "Error: Start date is more recent than end date.\n");
        exit(EXIT_FAILURE);
    }

    if ((options.use_custom_authentication && validate_file(options.authentication, F_OK | R_OK) == false) ||
        (use_area_subset && validate_file(options.coordinates, F_OK | R_OK) == false) ||
        validate_directory(options.output_directory) == false) {
        fprintf(stderr, "Error: Credential file, coordinate file or output directory either do not "
                        "exist, or are not accessible.\n");
        exit(EXIT_FAILURE);
    }

    if (init_api_authentication(&api_authentication, &options) != 0) {
        fprintf(stderr, "Error: Failed to load API configuration from file\n");
        exit(EXIT_FAILURE);
    }

    client.auth = api_authentication;

    if (use_area_subset) {
        double *longitude, *latitude;

        request.bbox = parse_coordinate_file(options.coordinates,
                                             &longitude, &latitude);

        printf("North: %d, East: %d, South: %d, West: %d\n",
               request.bbox.north, request.bbox.east, request.bbox.south, request.bbox.west);

        free(longitude);
        free(latitude);
    } else {
        request.bbox.area_subset = 0;
    }

    curl = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (curl != 0) {
        fprintf(stderr, "Error: Failed to set up curl\n");
        exit(EXIT_FAILURE);
    }

    handle = curl_easy_init();

    if (!handle) {
        fprintf(stderr, "Error: Failed to perform curl_easy_init\n");
        exit(EXIT_FAILURE);
    }

    if (check_ads_status(&handle, &client) == ADS_STATUS_WARNING) {
        fprintf(stderr,
                "Error: Encountered warning with ADS. Please visit %s/%s.\n",
                client.auth.base_url,
                "status.json");
        exit(EXIT_FAILURE);
    }

    struct PRODUCT_RESPONSE product_response = ads_request_product(&request, &handle, &client);

    if (product_response.state == PRODUCT_STATUS_INVALID) {
        fprintf(stderr, "Error: Encountered unknown product status in response to POST request\n");
        exit(EXIT_FAILURE);
    }

    while (product_response.state != PRODUCT_STATUS_COMPLETED && product_response.state != PRODUCT_STATUS_FAILED &&
           product_response.state != PRODUCT_STATUS_INVALID && client.retries < client.max_retries) {
        printf("Product request in preparation. Try %d/%d. Next request will be made in %d seconds.\n",
               client.retries, client.max_retries, client.max_sleep);

        if (sleep(client.max_sleep)) {
            fprintf(stderr, "Error: Program received a SIGNAL while sleeping. Those are unhandled.\n");
            exit(EXIT_FAILURE);
        }

        ads_check_product_state(&product_response, &handle, &client);

        client.retries++;
    }

    if (client.retries == client.max_retries && product_response.state != PRODUCT_STATUS_FAILED) {
        fprintf(stderr, "Error: Exceed maximum number of retries. Product request unsuccessful.\n"
                        "You can try to run the program with the same request later, to download the requested data.\n");
        exit(EXIT_FAILURE);
    }

    if (product_response.state == PRODUCT_STATUS_FAILED) {
        fprintf(stderr, "Error: Product request failed. Please check the website for more information\n");
        exit(EXIT_FAILURE);
    }

    const char *download_path = assemble_download_path(&request, &options);

    if (ads_download_product(&product_response, &handle, &client, download_path)) {
        fprintf(stderr, "Error: Failed to download file\n");
        exit(EXIT_FAILURE);
    }

    if (client.delete) {
        int deletion_status __attribute__((unused)) = ads_delete_product_request(&product_response, &handle, &client);
    }

    free(product_response.location);
    free(product_response.id);

    free((char *)download_path);

    curl_easy_cleanup(handle);
    curl_global_cleanup();

    free_api_authentication(&api_authentication);

    return 0;
}
