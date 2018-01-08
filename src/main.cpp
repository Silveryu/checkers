#include <opencv2/opencv.hpp>
#include <iostream>
#include "game.hpp"

void printVector(std::vector<cv::Point2f> v) {
    for (std::vector<cv::Point2f>::const_iterator i = v.begin(); i != v.end(); ++i)
        std::cout << *i << ", ";
    std::cout << std::endl;
    std::cout << std::endl;
}

void printVector(std::vector<cv::Vec3f> v) {
    for (std::vector<cv::Vec3f>::const_iterator i = v.begin(); i != v.end(); ++i)
        std::cout << *i << ", ";
    std::cout << std::endl;
    std::cout << std::endl;
}

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

std::vector<cv::Point2f> completeBoardCorners(std::vector<cv::Point2f> insideCorners)
{
    cv::Size patternsize(7, 7);
    int insideCornerDim = patternsize.width;

    cv::Point2f vertex1 = cv::Point2f(
        insideCorners[0].x + abs(insideCorners[0].x - insideCorners[insideCornerDim].x),
        insideCorners[0].y - abs(insideCorners[0].y - insideCorners[1].y)
    );

    cv::Point2f vertex2 = cv::Point2f(
        insideCorners[insideCorners.size()-1].x - abs(insideCorners[insideCorners.size()-1].x - insideCorners[insideCorners.size()-1-insideCornerDim].x),
        insideCorners[insideCorners.size()-1].y + abs(insideCorners[insideCorners.size()-1].y - insideCorners[insideCorners.size()-2].y)
    );

    int cornerDim = insideCornerDim+2;
    std::vector<cv::Point2f> corners = std::vector<cv::Point2f>(cornerDim*cornerDim);

    // first column
    corners[0] = vertex1;
    for(int i = 0; i < insideCornerDim; ++i){
        corners[i+1] =  cv::Point2f(vertex1.x, insideCorners[i].y);
    }
    corners[cornerDim-1] = cv::Point2f(vertex1.x, vertex2.y);

    // for every insideCorners' x coord
    for(int i = 1; i < cornerDim -1 ; ++i){
        float xCoord = insideCorners[(i-1)*insideCornerDim].x;
        corners[i*cornerDim] = cv::Point2f(xCoord, vertex1.y);

        //for every Corners' y coord
        for(int j = 0;j < insideCornerDim;j++){
            corners[i*cornerDim+j+1] = cv::Point2f(xCoord, insideCorners[j].y);
        }

        corners[(i+1)*cornerDim-1] = cv::Point2f(xCoord, vertex2.y);
    }

    // last column
    corners[cornerDim*(cornerDim-1)] = cv::Point2f(vertex2.x, vertex1.y);
    for(int i = 0; i < insideCornerDim; ++i){
        corners[cornerDim*(cornerDim-1)+i+1] =  cv::Point2f(vertex2.x, insideCorners[i].y);
    }
    corners[corners.size()-1] = vertex2;


    return std::vector<cv::Point2f>(corners);
}

std::vector<cv::Point2f> getBoardCorners(cv::Mat frame)
{
    static int count = 0;
    static std::vector<cv::Point2f> result = std::vector<cv::Point2f>();
    if (++count == 5) {
        count = 0;
        cv::Size patternsize(7, 7); //interior number of corners
        std::vector<cv::Point2f> tmp = std::vector<cv::Point2f>();
        bool patternfound = cv::findChessboardCorners(frame, patternsize, tmp, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
        if(patternfound) {
            // improve accuracy
            cv::cornerSubPix(frame, tmp, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            result = std::vector<cv::Point2f>(tmp);
            printVector(result);
            std::sort(result.begin(), result.end(), [](const cv::Point2f &a, const cv::Point2f &b) { return a.x > b.x; });
            for (auto i = result.begin(); i != result.end(); i += 7)
                std::sort(i, i+7, [](const cv::Point2f &a, const cv::Point2f &b) { return a.y < b.y; });
            printVector(result);
            result = completeBoardCorners(result);
            std::sort(result.begin(), result.end(), [](const cv::Point2f &a, const cv::Point2f &b) { return a.x < b.x; });
            for (auto i = result.begin(); i != result.end(); i += 9)
                std::sort(i, i+9, [](const cv::Point2f &a, const cv::Point2f &b) { return a.y < b.y; });
            printVector(result);

        }
    }

    return result;
}


std::vector<cv::Point2f> getPositionCorners(std::vector<cv::Point2f> corners, int x, int y)
{
    int stride = BOARD_SIZE+1;
    std::vector<cv::Point2f> quad;
    quad.push_back(corners.at(stride*x + y));
    quad.push_back(corners.at(stride*x + (y + 1)));
    quad.push_back(corners.at(stride*(x + 1) + y));
    quad.push_back(corners.at(stride*(x + 1) + (y + 1)));
    return quad;
}

bool isPointInsideQuad(cv::Point2f c, std::vector<cv::Point2f> quad)
{
    cv::Point2f p0 = quad.at(0);
    cv::Point2f p3 = quad.at(3);

    float thresh_x = abs(p3.x - p0.x)/4;
    float thresh_y = abs(p3.y - p0.y)/4;

    return p0.x-thresh_x < c.x && c.x < p3.x+thresh_x &&
           p0.y-thresh_y < c.y && c.y < p3.y+thresh_y;

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

        cv::Mat gray;
        cv::cvtColor(frame, gray, CV_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        // Find board corners
        cv::Mat boardCorners = gray.clone();
        std::vector<cv::Point2f> corners = getBoardCorners(gray);
        cv::drawChessboardCorners(boardCorners, cv::Size(9, 9), cv::Mat(corners), !corners.empty());
        grid.push_back(boardCorners);

        // Threshold the HSV image, keep only the red pixels
        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::medianBlur(hsv, hsv, 11);
        cv::Mat reds;
        cv::inRange(hsv, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), reds);
        cv::GaussianBlur(reds, reds, cv::Size(9, 9), 2, 2);
        grid.push_back(reds);

        // Threshold the HSV image, keep only the yellow pixels
        cv::Mat yellows;
        cv::inRange(hsv, cv::Scalar(25, 20, 20), cv::Scalar(32, 255, 255), yellows);
        cv::GaussianBlur(yellows, yellows, cv::Size(9, 9), 2, 2);
        grid.push_back(yellows);

        // Find red and yellow circles
        std::vector<cv::Vec3f> redCircles;
        cv::HoughCircles(reds, redCircles, CV_HOUGH_GRADIENT, 1, reds.rows/8, 100, 20, 0, 0);
        std::vector<cv::Vec3f> yellowCircles;
        cv::HoughCircles(yellows, yellowCircles, CV_HOUGH_GRADIENT, 1, reds.rows/8, 100, 20, 0, 0);

        // Check if circles are inside game positions
        if (!corners.empty()) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                for (int y = 0; y < BOARD_SIZE; y++) {
                    if ((x + y) % 2 == 0) {
                        for(size_t i = 0; i < redCircles.size(); i++) {
                            cv::Point2f center(redCircles[i][0], redCircles[i][1]);
                            std::vector<cv::Point2f> quad = getPositionCorners(corners, x, y);
                            if (isPointInsideQuad(center, quad)) {
                                std::cout << std::endl << "Circle with center" << center << "is a piece that's inside game position (" << x << "," << y << ")" << std::endl;
                                game.set_red(x, y);
                            }
                        }

                        for(size_t j = 0; j < yellowCircles.size(); j++) {
                            cv::Point2f center(yellowCircles[j][0], yellowCircles[j][1]);
                            std::vector<cv::Point2f> quad = getPositionCorners(corners, x, y);
                            if (isPointInsideQuad(center, quad)) {
                                std::cout << std::endl << "Circle with center" << center << "is a piece that's inside game position (" << x << "," << y << ")" << std::endl;
                                game.set_yellow(x, y);
                            }
                        }
                    }
                }
            }
        }
        game.print();

        // Fill up grid
        while(grid.size() != gridx*gridy)
            grid.push_back(cv::Mat(frame.rows, frame.cols, CV_8UC1, 255.0));
        int height = 800;
        int width = height*4/3;
        cv::Mat res = cv::Mat(height, width, CV_8UC1);
        tile(grid, res, gridx, gridy);
        cv::imshow("grid", res);
        if(cv::waitKey(1) == 'p') while(cv::waitKey(1) != 'p');
    }

    return 0;
}

