#include <opencv2/opencv.hpp>
#include <iostream>
#include "game.hpp"

#define STRIDE 7

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
    static std::vector<cv::Point2f> result = std::vector<cv::Point2f>();
    if (++count == 5) {
        std::cout << "Calculating board corners at frequency" << count << std::endl;
        count = 0;
        cv::Size patternsize(7, 7); //interior number of corners
        std::vector<cv::Point2f> tmp = std::vector<cv::Point2f>();
        bool patternfound = cv::findChessboardCorners(frame, patternsize, tmp, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
        if(patternfound) {
            // improve accuracy
            cv::cornerSubPix(frame, tmp, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            result = std::vector<cv::Point2f>(tmp);
        }
    }

    return result;
}

void printVector(std::vector<cv::Point2f> v) {
    for (std::vector<cv::Point2f>::const_iterator i = v.begin(); i != v.end(); ++i)
        std::cout << *i << ", ";
    std::cout << std::endl;
    std::cout << std::endl;
}

std::vector<cv::Point2f> getPositionCorners(std::vector<cv::Point2f> corners, int x, int y)
{
    std::vector<cv::Point2f> quad;
    quad.push_back(corners.at(STRIDE*x + y));
    quad.push_back(corners.at(STRIDE*x + (y + 1)));
    quad.push_back(corners.at(STRIDE*(x + 1) + y));
    quad.push_back(corners.at(STRIDE*(x + 1) + (y + 1)));
    return quad;
}

bool isPointInsideQuad(cv::Point2f c, std::vector<cv::Point2f> quad)
{
    cv::Point2f p0 = quad.at(0);
    cv::Point2f p1 = quad.at(1);
    cv::Point2f p2 = quad.at(2);
    cv::Point2f p3 = quad.at(3);
    return p0.x <= c.x && c.x <= p3.x &&
           p0.y <= c.y && c.y <= p3.x;
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

    int gridx = 2;
    int gridy = 2;
    cv::namedWindow("grid", cv::WINDOW_NORMAL);
    while(true) {
        Game game;
        std::vector<cv::Mat> grid;

        // Extract frame, apply bounding box and warp
        cv::Mat frame;
        cap >> frame;

        // Normalize image for finding board corners
        cv::Mat gray;
        cv::cvtColor(frame, gray, CV_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        // Find board corners
        cv::Mat boardCorners = gray.clone();
        std::vector<cv::Point2f> corners = getBoardCorners(gray);
        cv::drawChessboardCorners(boardCorners, cv::Size(7, 7), cv::Mat(corners), !corners.empty());
        grid.push_back(boardCorners);

        // Threshold the HSV image, keep only the red pixels
        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::medianBlur(hsv, hsv, 11);
        cv::Mat reds;
        cv::inRange(hsv, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), reds);
        cv::GaussianBlur(reds, reds, cv::Size(9, 9), 2, 2);
        grid.push_back(reds);

        // Find red circles
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(reds, circles, CV_HOUGH_GRADIENT, 1, reds.rows/8, 100, 20, 0, 0);
        cv::Mat redCircles = reds.clone();
        for(size_t i = 0; i < circles.size(); ++i) {
            cv::Point2f center(round(circles[i][0]), round(circles[i][1]));
            int radius = round(circles[i][2]);
            cv::circle(redCircles, center, radius, cv::Scalar(0, 255, 0), 5);

            if (!corners.empty()) {
                for (int x = 0; x < STRIDE-1; x++) {
                    for (int y = 0; y < STRIDE-1; y++) {
                        std::vector<cv::Point2f> quad = getPositionCorners(corners, x, y);
                        if (isPointInsideQuad(center, quad)) {
                            std::cout << std::endl << "Circle with center" << center << "is a piece that's inside game position (" << x << "," << y << ")" << std::endl;
                            game.set_red(x, y);
                        }
                    }
                }
            }
        }
        grid.push_back(redCircles);

        game.print();

        // Fill up grid
        while(grid.size() != gridx*gridy)
            grid.push_back(cv::Mat(frame.rows, frame.cols, CV_8UC1, 255.0));
        int height = 800;
        int width = height*4/3;
        cv::Mat res = cv::Mat(height, width, CV_8UC1);
        tile(grid, res, gridx, gridy);
        cv::imshow("grid", res);
        if(cv::waitKey(30) == 'q') break;
    }

    return 0;
}

