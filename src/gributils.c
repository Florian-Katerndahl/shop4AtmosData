#include <stdio.h>

#include "gributils.h"

void print_usage(void) {
    printf(
        "Usage: cams-process <-h|--help> <-v|--version> <-i|--purpose> "
        "<-t|--daily_tables> <-c|--climatology> <-g|--gtiff> in_file out_dir\n"
        "\nOptional arguments:\n"
        "<-h|--help>\tprint this help and exit\n"
        "<-v|--version>\tprint FORCE version and exit\n"
        "<-i|--purpose>\tprint program's purpose and exit\n"
        "<-t|--daily_tables>\tBuild daily tables? Default if not specified: false\n"
        "<-c|--climatology>\tBuild climatology? Default if not specified: false\n"
        "<-g|--gtiff>\tConvert each step from in_file to GTiff? Default if not specified: false\n"
        "\nMandatory positional arguments:\n"
        "in_file\t\t\tAbsolut path to file to process\n"
        "out_dir\t\t\tAbsolut path to directory in which results are stored. Needs to exist before program invocation\n"
        );
}

void print_purpose(void) {
    printf("Process downloaded ECMWF data (CAMS EC4) to daily tables, climatology or convert to GTiff\n");
}

void print_version(void) {
    printf("FORCE version is: %s\n", FORCE_VERSION);
}
