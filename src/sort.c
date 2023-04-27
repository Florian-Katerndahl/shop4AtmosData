#include <stdlib.h>
#include "sort.h"

void sort_double(double *array, size_t length, void (*f) (double *, size_t)) {
    f(array, length);
}

void bubble(double *array, size_t length) {
    double temp;

    for (size_t i = 0; i < length; i++) {
        for (size_t j = 0; j < length - 1; j++) {
            if (array[j] > array[j + 1]) {
                temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
            }
        }
    }
}
