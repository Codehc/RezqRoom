#include "ArrayUtils.h"

// Function to take the average value of an array of integers
// used in our moving average filter
int ArrayUtils::avg(int *array, int size) {
    int total = 0;
    for (int i = 0; i < size; i++) {
        total += array[i];
    }
    return total / size;
}

// Function to shift contents of array 1 to the "left"
void ArrayUtils::shift(int *array, int size) {
    int tmp[5];
    for (int i = size - 1; i > 0; i--) {
        tmp[i - 1] = array[i];
    }

    for (int i = 0; i < size; i++) {
        array[i] = tmp[i];
    }
}

// Shifts array to the left and then adds a value to the end
void ArrayUtils::push(int *array, int size, int value) {
    shift(array, size);

    array[size - 1] = value;
}

void ArrayUtils::printArray(int * array, int size) {
    for (int i = 0; i < size; i++) {
        printf("%i, ", array[i]);
    }
    printf("\n");
}
