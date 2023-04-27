#ifndef CAMS_SORT_H
#define CAMS_SORT_H

#include <stdlib.h>

void sort_double(double *array, size_t length, void (*f) (double *, size_t));

void bubble(double *array, size_t length);

void merge(double *array, size_t length);

void merge_arrays();

#endif //CAMS_SORT_H
