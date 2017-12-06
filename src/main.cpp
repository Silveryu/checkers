#include <opencv2/opencv.hpp>
#include <iostream>
#include "game.hpp" 

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
    VideoCapture cap;
    if(argc == 2) {
        cap.open(argv[1]);
    } else {
        cap.open(0);
    }

    if(!cap.isOpened()) {
        return -1;
    }

    Game game;
    game.print();
    cout << endl << game.cell(2, 3) << endl;

    namedWindow("edges", 1);
    while(true)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera

        Mat edges;
        cvtColor(frame, edges, COLOR_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);

        imshow("edges", edges);
        if(waitKey(30) == 'q') break;
    }

    return 0;
}

