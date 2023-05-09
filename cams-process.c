#include <stdio.h>
#include <eccodes.h>
#include <getopt.h>
//#include <gdal/gdal.h>

#include "src/gributils.h"

#ifdef DEBUG
#define NO_GETOPT_ERROR_OUTPUT 0
#else
#define NO_GETOPT_ERROR_OUTPUT 1
#endif // DEBUG

int main(int argc, char *argv[]) {
    static struct PROCESS_OPTIONS options = {0};

    int optid, long_index = 0;
    opterr = NO_GETOPT_ERROR_OUTPUT;

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"purpose", no_argument, NULL, 'i'},
        {"version", no_argument, NULL, 'v'},
        {"daily_tables", no_argument, &options.daily_tables, 1},
        {"climatology", no_argument, &options.climatology, 1},
        {"gtiff", no_argument, &options.convert_to_tiff, 1},
        {0, 0, 0, 0}
    };

    while ((optid = getopt_long(argc, argv, "+:hivtcg", long_options, &long_index)) != -1) {
        switch (optid) {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            case 'i':
                print_purpose();
                exit(EXIT_SUCCESS);
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            case 't': // fallthrough
            case 'c': // fallthrough
            case 'g': // fallthrough
            case 0:
                break;
            case '?':
                fprintf(stderr, "ERROR parsing option %c\n", optopt);
                break;
            default:
                fprintf(stderr, "ERROR: Unreachable!\n");
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        options.in_file = argv[optind++];
        options.out_dir = argv[optind];
    } else {
        fprintf(stderr, "ERROR: Either in_file, out_dir or both not specified after arguments\n");
        exit(EXIT_FAILURE);
    }

    if (options.daily_tables) {

    }

    if (options.climatology) {

    }

    if (options.convert_to_tiff) {

    }

    return 0;
}
