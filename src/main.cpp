#include <opencv2/opencv.hpp>
#include <iostream>
#include "game.hpp"

using namespace cv;
using namespace std;

void tile(const vector<Mat> &src, Mat &dst, int grid_x, int grid_y)
{
    // patch size
    int width  = dst.cols/grid_x;
    int height = dst.rows/grid_y;

    // iterate through grid
    int k = 0;
    for(int i = 0; i < grid_y; i++) {
        for(int j = 0; j < grid_x; j++) {
            Mat s = src[k++];
            resize(s,s,Size(width,height));
            s.copyTo(dst(Rect(j*width,i*height,width,height)));
        }
    }
}

vector<Point2f> getBoardCorners(Mat frame)
{
    Size patternsize(7, 7); //interior number of corners
    vector<Point2f> corners; //this will be filled by the detected corners
    bool patternfound = findChessboardCorners(frame, patternsize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
    if(patternfound) {
        // improve accuracy
        cornerSubPix(frame, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

        cout << endl;
        for (std::vector<Point2f>::const_iterator i = corners.begin(); i != corners.end(); ++i)
            cout << *i << ", ";
        cout << endl;

        return corners;
    }
    return vector<Point2f>();
}

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

    int gridx = 2;
    int gridy = 2;
    namedWindow("grid", WINDOW_NORMAL);
    while(true) {
        vector<Mat> grid;

        Mat frame;
        cap >> frame; // get a new frame from camera
        //grid.push_back(frame);

        Mat gray;
        cvtColor(frame, gray, CV_BGR2GRAY);
        grid.push_back(gray);

        Mat boardCorners = gray.clone();
        vector<Point2f> corners = getBoardCorners(gray);
        drawChessboardCorners(boardCorners, Size(7, 7), Mat(corners), !corners.empty());
        grid.push_back(boardCorners);

        Mat edges;
        GaussianBlur(gray, edges, Size(7, 7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        grid.push_back(edges);

        while(grid.size() != gridx*gridy) {
            grid.push_back(gray);
        }

        int height = 800;
        int width = height*4/3;
        Mat res = Mat(height, width, CV_8UC1);
        tile(grid, res, gridx, gridy);
        imshow("grid", res);
        if(waitKey(30) == 'q') break;
    }

    return 0;
}

