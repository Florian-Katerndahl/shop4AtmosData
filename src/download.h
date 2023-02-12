#ifndef CAMS_DOWNLOAD_H
#define CAMS_DOWNLOAD_H

#include <stdbool.h>

#define FORCE_VERSION "Test, Test!"

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
 */
struct OPTIONS {
    char authentication[NPOW16];
    char coordinates[NPOW16];
    char output_directory[NPOW16];
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
    bool verify;        ///< verify the tls-ssl connection?
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
 * @brief
 * @param api_authentication
 * @param options
 * @return
 * @author Florian Katerndahl
 */
int init_api_authentication(const struct API_AUTHENTICATION *api_authentication, const struct OPTIONS *options);

/**
 * @brief
 * @param api_authentication
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

#endif //CAMS_DOWNLOAD_H
