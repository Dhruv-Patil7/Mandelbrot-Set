#include <complex>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

class MandelbrotSet{
  public:
    vector<int> image;
    //Setting the size of graph
    double REAL_MIN = -2;
    double REAL_MAX = 2;
    double IMAGE_MIN = -1.5;
    double IMAGE_MAX = 1.5;

    int HEIGHT;
    int WIDTH;
    const int MAX_ITR;

    MandelbrotSet(int width, int height, int max_itr, int numThreads) : 
                  WIDTH(width), HEIGHT(height), MAX_ITR(max_itr), NUM_THREADS(numThreads){
        image.resize(WIDTH * HEIGHT);// Making the vector the size of total pixels.
    }
    
    //renders fractal using multiplethreading
    void renderFract(){
        vector<thread> threads;
        NUM_THREADS = thread::hardware_concurrency();
        if(NUM_THREADS == 0) NUM_THREADS = 4; // Fallback

        int rowsPerThread = HEIGHT / NUM_THREADS;

        for (int i = 0; i < NUM_THREADS; i++) {
            int startRow = i * rowsPerThread;
            int endRow = (i == NUM_THREADS - 1) ? HEIGHT : startRow + rowsPerThread;
            threads.push_back(thread(&MandelbrotSet::renderFractPart, this, startRow, endRow));
        }

        for (auto& t : threads) {
            t.join();
        }
    }

    //Changing the region (Zooming In);
    void updateRegion(double realMin, double realMax, double imageMin, double imageMax){
        REAL_MIN = realMin;
        REAL_MAX = realMax;
        IMAGE_MIN = imageMin;
        IMAGE_MAX = imageMax;
    }
    
    //Setting it to default values.
    void defaultRegion(){
        REAL_MIN = -2;
        REAL_MAX = 2;
        IMAGE_MIN = -1.5;
        IMAGE_MAX = 1.5;
    }
  private:
    int NUM_THREADS;//Count of threads.

    void renderFractPart(int startrow, int endrow) {
        for(int y = startrow; y < endrow; y++){
            for(int x = 0; x < WIDTH; x++){
                double realP = REAL_MIN + (REAL_MAX - REAL_MIN) * x / WIDTH;
                double imagP = IMAGE_MIN + (IMAGE_MAX - IMAGE_MIN) * y / HEIGHT;
                complex<double> c(realP, imagP);//Calculating the value of C.
                complex<double> z = 0;
                int itr = 0;
                //Calculating the number of iteration for each pixels.
                while(norm(z) <= 4.0 && itr < MAX_ITR){
                    z = z * z + c;
                    itr++;
                }
                //Storing the number of iteration.
                image[y * WIDTH + x] = itr;
            }
        }
    }
};