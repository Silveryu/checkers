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

void sharpen(cv::Mat src, cv::Mat dst)
{
    cv::GaussianBlur(src, dst, cv::Size(0, 0), 3);
    cv::addWeighted(src, 1.5, dst, -0.5, 0, dst);
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
        insideCorners[insideCornerDim-1].x + abs(insideCorners[insideCornerDim-1].x - insideCorners[insideCornerDim*2-1].x),
        insideCorners[insideCornerDim-1].y + abs(insideCorners[insideCornerDim-1].y - insideCorners[insideCornerDim-2].y)
    );

    cv::Point2f vertex3 = cv::Point2f(
        insideCorners[insideCorners.size()-insideCornerDim].x - abs(insideCorners[insideCorners.size()-insideCornerDim].x - insideCorners[insideCorners.size()-insideCornerDim*2].x),
        insideCorners[insideCorners.size()-insideCornerDim].y - abs(insideCorners[insideCorners.size()-insideCornerDim].y - insideCorners[insideCorners.size()-insideCornerDim+1].y)
    );

    cv::Point2f vertex4 = cv::Point2f(
        insideCorners[insideCorners.size()-1].x - abs(insideCorners[insideCorners.size()-1].x - insideCorners[insideCorners.size()-1-insideCornerDim].x),
        insideCorners[insideCorners.size()-1].y + abs(insideCorners[insideCorners.size()-1].y - insideCorners[insideCorners.size()-2].y)
    );


    int cornerDim = insideCornerDim+2;
    std::vector<cv::Point2f> outterCorners= std::vector<cv::Point2f>();

    // first column
    
    outterCorners.push_back(vertex1);
    outterCorners.push_back(vertex2);
    outterCorners.push_back(vertex3);
    outterCorners.push_back(vertex4);


    for(int i = 0; i < insideCornerDim; ++i){   
        outterCorners.push_back(cv::Point2f(
            insideCorners[i].x + abs(insideCorners[i].x - insideCorners[insideCornerDim+i].x), 
            insideCorners[i].y
        ));
    }

    for(int i = 0; i < insideCornerDim; ++i){
        int idx = insideCornerDim*(i+1)-1;
        outterCorners.push_back(cv::Point2f(
            insideCorners[idx].x, 
            insideCorners[idx].y + abs(insideCorners[idx].y - insideCorners[idx-1].y))  
        );

    }

    for(int i = 0; i < insideCornerDim; ++i){   
        int idx = insideCorners.size()-insideCornerDim + i;
        outterCorners.push_back(cv::Point2f(
            insideCorners[idx].x - abs(insideCorners[idx].x - insideCorners[idx-insideCornerDim].x), 
            insideCorners[idx].y 
        ));
       
    }


    for(int i = 0; i < insideCornerDim; ++i){
        int idx = insideCornerDim*i;
        outterCorners.push_back(cv::Point2f(
            insideCorners[idx].x,
            insideCorners[idx].y - abs(insideCorners[idx].y - insideCorners[idx+1].y)
        ));
       
    }

    std::vector<cv::Point2f> corners = insideCorners;

    corners.insert(corners.end(), outterCorners.begin(), outterCorners.end());

    printVector(corners);

    std::sort(corners.begin(), corners.end(), [](const cv::Point2f &a, const cv::Point2f &b) { return a.x < b.x; });
    for (auto i = corners.begin(); i != corners.end(); i += 9)
        std::sort(i, i+9, [](const cv::Point2f &a, const cv::Point2f &b) { return a.y < b.y; });

    return std::vector<cv::Point2f>(corners);
}

std::vector<cv::Point2f> getBoardCorners(cv::Mat frame)
{
    static int count = 0;
    static std::vector<cv::Point2f> result = std::vector<cv::Point2f>();
    if (++count == 1) {
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
            cv::cornerSubPix(frame, result, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));


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

cv::Mat differenceBetween(cv::Mat frame, cv::Mat reference)
{
    cv::Mat diff;
    absdiff(frame, reference, diff);

    // Get the mask if difference greater than threshold
    int th = 10;
    cv::Mat mask(frame.size(), CV_8UC1);
    for(int j = 0; j < diff.rows; ++j) {
        for(int i = 0; i < diff.cols; ++i) {
            cv::Vec3b pix = diff.at<cv::Vec3b>(j, i);
            int val = (pix[0] + pix[1] + pix[2]);
            if (val > th) {
                mask.at<unsigned char>(j,i) = 255;
            }
        }
    }

    // get the foreground
    cv::Mat res;
    cv::bitwise_and(reference, reference, res, mask);
    return res;
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

    int gridx = 3;
    int gridy = 3;
    cv::namedWindow("grid", cv::WINDOW_NORMAL);
    cv::Mat currentReferenceFrame;
    cap >> currentReferenceFrame;
    cv::cvtColor(currentReferenceFrame, currentReferenceFrame, CV_BGR2GRAY);

    while(true) {
        Game game;
        std::vector<cv::Mat> grid;

        // Extract frame, apply bounding box and warp
        cv::Mat frame;
        cap >> frame;

        cv::Mat gray;
        cv::cvtColor(frame, gray, CV_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        cv::Mat grayClone = gray.clone();
        sharpen(grayClone, gray);

        // Check if cu
        grid.push_back(currentReferenceFrame);
        cv::Mat diff = differenceBetween(gray, currentReferenceFrame);
        grid.push_back(diff);

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
        cv::inRange(hsv, cv::Scalar(25, 95, 95), cv::Scalar(32, 255, 255), yellows);
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

