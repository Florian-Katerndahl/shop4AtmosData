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

    static struct API_AUTHENTICATION api_authentication = {0};

    struct CLIENT client = {0};

    struct PRODUCT_REQUEST request = {0};

    CURLcode curl;

    CURL *handle;

    __attribute__((unused)) CURLcode res;

    // populate default/non-user-changeable-options
    request.dates.start = (struct tm) {.tm_year = 70, .tm_mon = 0, .tm_mday = 1};
    request.dates.end = (struct tm) {.tm_year = 70, .tm_mon = 0, .tm_mday = 1};
    request.variable = "total_aerosol_optical_depth_469nm";
    request.format = "grib";

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"purpose", no_argument, NULL, 'i'},
        {"authentication", required_argument, NULL, 'a'},
        {"coordinates", required_argument, NULL, 'c'},
        {"start", required_argument, NULL, '0'},
        {"end", required_argument, NULL, '1'},
        {"output_directory", required_argument, NULL, 'o'},
        {0, 0, 0, 0}
    };

    // TODO why can I remove a letter from shortopts and still match the short version?
    while ((optid = getopt_long_only(argc, argv, "+:hvia:c:o:01", long_options, &option_index)) != -1) {
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
        validate_file(options.coordinates, F_OK | R_OK) == false ||
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

    double *longitude, *latitude;

    request.bbox = parse_coordinate_file(options.coordinates,
                                         &longitude, &latitude);

    printf("North: %d, East: %d, South: %d, West: %d\n",
           request.bbox.north, request.bbox.east, request.bbox.south, request.bbox.west);

    free(longitude);

    free(latitude);

    //abort();

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

    // init_curl_handle(&handle, &client); => inside function calls!

    // TODO
    //  - some sort of URL-generator (typedef'd enum in switch statement with static char array)?
    //  - result struct -> implemented to check_ads_status
    //  - writeback function -> implemented to check_ads_status
    //  - function to handle redirects etc.
    if (check_ads_status(&handle, &client) == ADS_STATUS_WARNING) {
        fprintf(stderr,
                "Error: Encountered warning with ADS. Please visit %s/%s.\n",
                client.auth.base_url,
                "status.json");
        exit(EXIT_FAILURE);
    }

    ads_retrieve(PRODUCT_CAMS_REPROCESSED, &request, "/home/florian/Desktop/cams-download.grib", &handle, &client);

    curl_easy_cleanup(handle);

    curl_global_cleanup();

    free_api_authentication(&api_authentication);

    return 0;
}
