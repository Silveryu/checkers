#include <opencv2/opencv.hpp>
#include <iostream>
#include "game.hpp"

void tile(const std::vector<cv::Mat> &src, cv::Mat &dst, int grid_x, int grid_y)
{
    // patch size
    int width  = dst.cols/grid_x;
    int height = dst.rows/grid_y;

    // iterate through grid
    int k = 0;
    for(int i = 0; i < grid_y; i++) {
        for(int j = 0; j < grid_x; j++) {
            cv::Mat s = src[k++];
            resize(s, s, cv::Size(width, height));
            s.copyTo(dst(cv::Rect(j*width, i*height, width, height)));
        }
    }
}

std::vector<cv::Point2f> getBoardCorners(cv::Mat frame)
{
    static int count = 0;
    if (++count == 10) {
        std::cout << "Calculating board corners..." << count << std::endl;
        count = 0;
        cv::Size patternsize(7, 7); //interior number of corners
        std::vector<cv::Point2f> corners; //this will be filled by the detected corners
        bool patternfound = cv::findChessboardCorners(frame, patternsize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
        if(patternfound) {
            // improve accuracy
            cv::cornerSubPix(frame, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

            std::cout << std::endl;
            for (std::vector<cv::Point2f>::const_iterator i = corners.begin(); i != corners.end(); ++i)
                std::cout << *i << ", ";
            std::cout << std::endl;

            return corners;
        }
    }
    return std::vector<cv::Point2f>();
}

int main(int argc, char* argv[])
{
    cv::VideoCapture cap;
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
    std::cout << std::endl << game.cell(2, 3) << std::endl;

    int gridx = 2;
    int gridy = 2;
    cv::namedWindow("grid", cv::WINDOW_NORMAL);
    while(true) {
        std::vector<cv::Mat> grid;

        cv::Mat frame;
        cap >> frame;

        // Normalize image
        cv::Mat gray;
        cv::cvtColor(frame, gray, CV_BGR2GRAY);
        medianBlur(gray, gray, 11);
        equalizeHist(gray, gray);

        cv::Mat boardCorners = gray.clone();
        std::vector<cv::Point2f> corners = getBoardCorners(gray);
        cv::drawChessboardCorners(boardCorners, cv::Size(7, 7), cv::Mat(corners), !corners.empty());
        grid.push_back(boardCorners);

        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

        // Threshold the HSV image, keep only the red pixels
        cv::Mat reds;
        cv::inRange(hsv, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), reds);
        cv::GaussianBlur(reds, reds, cv::Size(9, 9), 2, 2);
        grid.push_back(reds);

        // Find red circles
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(reds, circles, CV_HOUGH_GRADIENT, 1, reds.rows/8, 100, 20, 0, 0);

        // Draw red circles
        cv::Mat redCircles = reds.clone();
        for(size_t current_circle = 0; current_circle < circles.size(); ++current_circle) {
            cv::Point center(round(circles[current_circle][0]), round(circles[current_circle][1]));
            int radius = round(circles[current_circle][2]);
            cv::circle(redCircles, center, radius, cv::Scalar(0, 255, 0), 5);
        }
        std::cout << "Red circles:" << circles.size() << std::endl;
        grid.push_back(redCircles);

        // Fill up grid with empty images
        while(grid.size() != gridx*gridy) grid.push_back(cv::Mat(frame.rows, frame.cols, CV_8UC1, 255.0));

        int height = 800;
        int width = height*4/3;
        cv::Mat res = cv::Mat(height, width, CV_8UC1);
        tile(grid, res, gridx, gridy);
        cv::imshow("grid", res);
        if(cv::waitKey(30) == 'q') break;
    }

    return 0;
}

