#ifndef CAMS_GRIBUTILS_H
#define CAMS_GRIBUTILS_H

#define FORCE_VERSION "69.420"

/**
 * @brief Program's CLI options
 * @author Florian Katerndahl
 */
struct PROCESS_OPTIONS {
    char *in_file;          ///< blubb
    char *out_dir;          ///< blabb
    int daily_tables;       ///< flag if daily tables should be build
    int climatology;        ///< flag if climatology should be build
    int convert_to_tiff;    ///< flag if each entry in input file should be converted and exported as GTif
};

/**
 * @brief Print the program's usage
 * @author Florian Katerndahl
 */
void print_usage(void);

/**
 * @brief Print the program's purpose
 * @author Florian Katerndahl
 */
void print_purpose(void);

/**
 * @brief Print FORCE's version
 * @author Florian Katerndahl
 */
void print_version(void);

/**
 * @brief
 * @author Florian Katerndahl
 */
void grib_data_from_file(void);

/**
 * @brief
 * @author Florian Katerndahl
 */
void grib_data_from_memory(void);

/**
 * @brief
 * @author Florian Katerndahl
 */
void netcdf_data_from_file(void);

/**
 * @brief
 * @author Florian Katerndahl
 */
void netcdf_data_from_memory(void);

/**
 * @brief
 * @author Florian Katerndahl
 */
void build_tables(void); // both for climatology and daily tables

/**
 * @brief
 * @author Florian Katerndahl
 */
void export_data_to_gtiff(void);

#endif //CAMS_GRIBUTILS_H
