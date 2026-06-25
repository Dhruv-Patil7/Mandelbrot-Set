#include <complex>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

enum class FractalType {// To switch between Julia and Mandelbrot
    Mandelbrot,
    Julia
};
class MandelbrotSet{
  public:
    std::vector<double> image;
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

    void setJuliaPreset(int preset)
    {
        if(preset < 1 || preset > 5) {
            return;
        }

        m_currentJuliaPreset = preset;

        switch (preset)// Switch based in key pressed
        {
            case 1:
                m_juliaC = {-0.8, 0.156};
                break;

            case 2:
                m_juliaC = {0.285, 0.01};
                break;

            case 3:
                m_juliaC = {-0.4, 0.6};
                break;

            case 4:
                m_juliaC = {-0.70176, -0.3842};
                break;

            case 5:
                m_juliaC = {0.355, 0.355};
                break;
        }
    }
    
    //renders fractal using multiplethreading
    void renderFract(){
        std::vector<std::thread> threads;
        NUM_THREADS = std::thread::hardware_concurrency();
        if(NUM_THREADS == 0) NUM_THREADS = 4; // Fallback

        int rowsPerThread = HEIGHT / NUM_THREADS;

        for (int i = 0; i < NUM_THREADS; i++) {
            int startRow = i * rowsPerThread;
            int endRow = (i == NUM_THREADS - 1) ? HEIGHT : startRow + rowsPerThread;
            threads.push_back(std::thread(&MandelbrotSet::renderFractPart, this, startRow, endRow));
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
        if(m_type == FractalType::Mandelbrot){
            REAL_MIN = -2;
            REAL_MAX = 2;
            IMAGE_MIN = -1.5;
            IMAGE_MAX = 1.5;
        }
        else{
            REAL_MIN = -1.5;
            REAL_MAX = 1.5;
            IMAGE_MIN = -1.5;
            IMAGE_MAX = 1.5;

        }
    }
    void setFractalType(FractalType type) {
        m_type = type;
    }

    FractalType getFractalType() const {
        return m_type;
    }

    void setJuliaConstant(std::complex<double> c) {
        m_juliaC = c;
    }

    int getJuliaPreset() const
    {
        return m_currentJuliaPreset;
    }


  private:
    int NUM_THREADS;//Count of threads.

    FractalType m_type = FractalType::Mandelbrot;
    std::complex<double> m_juliaC{-0.8, 0.156};
    int m_currentJuliaPreset = 1;

    void renderFractPart(int startrow, int endrow) {
        for(int y = startrow; y < endrow; y++){
            for(int x = 0; x < WIDTH; x++){
                double realP = REAL_MIN + (REAL_MAX - REAL_MIN) * x / WIDTH;
                double imagP = IMAGE_MIN + (IMAGE_MAX - IMAGE_MIN) * y / HEIGHT;
                std::complex<double> z;
                std::complex<double> c;
                if (m_type == FractalType::Mandelbrot)
                {
                    z = {0.0, 0.0};
                    c = {realP, imagP};
                }
                else
                {
                    z = {realP, imagP};
                    c = m_juliaC;
                }
                int itr = 0;
                //Calculating the number of iteration for each pixels.
                while(std::norm(z) <= 4.0 && itr < MAX_ITR){
                    z = z * z + c;
                    itr++;
                }
                if(itr == MAX_ITR) image[y * WIDTH + x] = MAX_ITR;
                else {
                    double magnitude = std::abs(z);
                    // Converting to double for more precise control on color
                    double smooth = itr + 1 - (std::log(std::log(magnitude)) / std::log(2.0));

                    image[y * WIDTH + x] = smooth;
                }
            }
        }
    }
};