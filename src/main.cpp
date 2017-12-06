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
    namedWindow("corners", 1);
    while(true)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera

        Mat gray;
        cvtColor(frame, gray, CV_BGR2GRAY);

        Size patternsize(7, 7); //interior number of corners
        vector<Point2f> corners; //this will be filled by the detected corners
        bool patternfound = findChessboardCorners(frame, patternsize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
        if(patternfound) {
            cout << endl;
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            for (std::vector<Point2f>::const_iterator i = corners.begin(); i != corners.end(); ++i)
                cout << *i << ", ";
            cout << endl;
        }

        Mat img = gray.clone();
        drawChessboardCorners(img, patternsize, Mat(corners), patternfound);

        Mat edges;
        GaussianBlur(gray, edges, Size(7, 7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);

        if(!edges.empty()) imshow("edges", edges);
        if(!img.empty()) imshow("corners", img);

        if(waitKey(30) == 'q') break;
    }

    return 0;
}

