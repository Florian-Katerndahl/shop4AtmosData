#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "src/download.h"

#ifdef DEBUG
#define NO_GETOPT_ERROR_OUTPUT 0
#else
#define NO_GETOPT_ERROR_OUTPUT 1
#endif // DEBUG

int main(int argc, char *argv[]) {
    char error_string[NPOW16];

    int optid;

    int option_index = 0; // will only be set, if parsing was successful

    opterr = NO_GETOPT_ERROR_OUTPUT ? 0 : 1;

    static struct OPTIONS options = {0};

    static struct option long_options[] = {
        {"help",             no_argument,       NULL, 'h'},
        {"version",          no_argument,       NULL, 'v'},
        {"purpose",          no_argument,       NULL, 'i'},
        {"authentication",   required_argument, NULL, 'a'},
        {"coordinates",      required_argument, NULL, 'c'},
        {"output_directory", required_argument, NULL, 'o'},
        {0, 0, 0,                                     0}
    };
    // TODO why can I remove a letter from shortopts and still match the short version?
    while ((optid = getopt_long_only(argc, argv, "+:hvia:c:o:", long_options, &option_index)) != -1) {
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
                // TODO check if I really got a file and if it exists
                if (optarg == 0) {
                    // if text is present, optarg points to it; otherwise it is set to 0
                    fprintf(stderr, "You shouldn't be able to reach this code!!\n");
                }
                break;
            case 'c':
                // TODO check if I really got a file and if it exists
                if (optarg == 0) {
                    // if text is present, optarg points to it; otherwise it is set to 0
                    fprintf(stderr, "You shouldn't be able to reach this code!!\n");
                }
                break;
            case 'o':
                // TODO check if I really got a directory and if not, create it
                if (optarg == 0) {
                    // if text is present, optarg points to it; otherwise it is set to 0
                    fprintf(stderr, "You shouldn't be able to reach this code!!\n");
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

    return 0;
}
