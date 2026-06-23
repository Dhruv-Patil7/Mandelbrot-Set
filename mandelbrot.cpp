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
    double REAL_MIN = -2;
    double REAL_MAX = 2;
    double IMAGE_MAX = 1.5;
    double IMAGE_MIN = -1.5;

    int HEIGHT;
    int WIDTH;
    const int MAX_ITR;

    MandelbrotSet(int height, int width, int max_itr) : 
                  WIDTH(width), HEIGHT(height), MAX_ITR(max_itr){
        image.resize(WIDTH * HEIGHT);
    }

    void renderFract() {
        for(int y = 0; y < HEIGHT; y++){
            for(int x = 0; x < WIDTH; x++){
                complex<double> c((REAL_MIN + (REAL_MAX - REAL_MIN) * x / WIDTH),
                                IMAGE_MIN + (IMAGE_MAX - IMAGE_MIN) * y / HEIGHT);
                complex<double> z = 0;
                int itr = 0;
                
                while(norm(z) <= 4.0 && itr < MAX_ITR){
                    z = z * z + c;
                    itr++;
                }
                image[y * WIDTH + x] = itr;
            }
        }
    }

    void saveFract(const string &fileloc){
        ofstream ofs(fileloc, ios::binary);
        ofs << "P5\n" << WIDTH << " " << HEIGHT << "\n255\n";

        for(int i = 0; i < HEIGHT * WIDTH; i++){
            int itr = image[i];
            unsigned char color = static_cast<unsigned char>(255 * itr / MAX_ITR);
            ofs << color;
        }
        ofs.close();
    }
};

int main(){
    int height, width, max_itr;
    cout << "Enter <height, width, max_itr>" << endl;
    cin >> height >> width >> max_itr;

    MandelbrotSet fractal = MandelbrotSet(height, width, max_itr);
    fractal.renderFract();
    cout << "Fractal Rendered" << endl; 

    fractal.saveFract("mandelbrot.pgm");
    cout << "executed" << endl;
}