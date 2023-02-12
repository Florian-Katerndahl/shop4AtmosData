#include <stdio.h>
#include <stdlib.h>
#include "download.h"

void print_usage(void) {
    printf(
        "Usage: cams-download [-h|--help] [-v|--version] [-i|--purpose] [-c|--coordinates] "
        "[-o|--output_directory] [-t|--daily_tables] [-s|--climatology] <-a|--authentication>\n\n"
        "[-h|--help]\tprint this help page and exit\n"
        "[-v|--version]\tprint version\n"
        "[-i|--purpose]\tshow program's purpose\n\n"
        "[-c|--coordinates]\t...\n"
        "[-o|--output_directory]\t...\n"
        "[-t|--daily_tables]\tbuild daily tables? Default: false\n"
        "[-s|--climatology]\tbuild climatology? Default: false\n\n"
        "Optional argument:\n"
        "<-a|--authentication>\toptional...\n");
}

void print_version(void) {
    printf("FORCE version: %s\n", FORCE_VERSION);
}

void print_purpose(void) {
    printf("Download ECMWF CAMS data from the Atmosphere Data Store\n");
}

int init_api_authentication(const struct API_AUTHENTICATION *api_authentication, const struct OPTIONS *options) {
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
        default:
            exit(129);
    }

    return dest;
}
