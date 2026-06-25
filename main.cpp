#include <complex>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>
#include "application.cpp"

int main() {
  // The ratio must be 4:3
  int width = 1024;
  int height = 768;
  int maxIter = 500;
  int numThreads = std::thread::
      hardware_concurrency();  // Number of threads for multithreading

  // Create an instance of the Application class
  Application application(width, height, maxIter, numThreads);

  // Run the application
  application.run();

  return 0;
}