namespace ArrayUtils;
  // Function to take the average value of an array of integers
  // used in our moving average filter
  int avg(int *array, int size);

  // Function to shift contents of array 1 to the "left"
  void shift(int *array, int size);

  // Shifts array to the left and then adds a value to the end
  void push(int *array, int size, int value);

  void printArray(int * array, int size);
}
