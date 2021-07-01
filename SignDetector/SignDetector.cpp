#include "SignDetector.h"
SignDetector::SignDetector() {}
SignDetector::~SignDetector() {}

void SignDetector::Run(std::string path) {
    vc = cv::VideoCapture(path);
    cv::Mat tmpdst;
    cv::Mat imageX, gray, probabilistic_hough;
    cv::Mat mat_color[3];
    cv::Mat K;
    cv::Mat hsl;
    cv::Mat I, dst, hsv;
    cv::Mat1b mask;
    int key = 0;
    cv::Mat xmat = (cv::Mat_<char>(3, 3) << 0, -1, 0,
       0, 5, 0,
       0, -1, 0);
    auto kern = cv::Mat(cv::Size(3, 3), CV_8UC1);
    xmat.copyTo(kern);
    while (key != 'q' && key != 'Q')
    {
      vc >> image;
      if (image.empty())
           break;
      cv::resize(image, tmpdst, cv::Size(image.size().width / 4, image.size().height / 4));
      image = tmpdst;
      filter2D(image, K, image.depth(), kern);
      cvtColor(K, hsv, cv::COLOR_BGR2HSV);
      auto yellowBlobs = getColorZones(m_settings, cv::Scalar(20, 190, 20), cv::Scalar(30, 255, 255), image, hsv, "Yellow", false, false, true);
      auto redBlobs = getColorZonesRed(m_settings, cv::Scalar(10, 70, 60), cv::Scalar(350, 255, 80), image, hsv, "Red", false, false, false);
      auto blueBlobs = getColorZones(m_settings, cv::Scalar(78, 158, 124), cv::Scalar(138, 255, 255), image, hsv, "Blue", true, true, false);
      IntersectionColorZones(image, yellowBlobs, blueBlobs, "BlueYellow", cv::Scalar(0, 0, 255));
      IntersectionColorZones(image, redBlobs, blueBlobs, "RedBlue", cv::Scalar(0, 0, 255));
      key = cv::waitKey(1);
    }
}

std::vector< cv::Rect>
SignDetector::Run(cv::Mat& image, bool IsShowWindows, bool IsRecogSigns, std::map<cv::Rect, typeSign>& _mapSigns) {
    cv::Mat tmpdst;
    std::vector< cv::Rect> zones1, zones2;
    cv::Mat imageX, gray, probabilistic_hough;
    cv::Mat mat_color[3];
    cv::Mat K;
    cv::Mat hsl;
    cv::Mat I, dst, hsv;
    cv::Mat1b mask;
    int key = 0;
    cv::Mat xmat = (cv::Mat_<char>(3, 3) << 0, -1, 0,
        0, 5, 0,
        0, -1, 0);
    auto kern = cv::Mat(cv::Size(3, 3), CV_8UC1);
    xmat.copyTo(kern);
    if (!image.empty()) {
      cv::resize(image, tmpdst, cv::Size(image.size().width / 4, image.size().height / 4));
      image = tmpdst;
      cv::filter2D(image, K, image.depth(), kern);
      cv::cvtColor(K, hsv, cv::COLOR_BGR2HSV);
      auto yellowBlobs = getColorZones(m_settings, cv::Scalar(20, 190, 20), cv::Scalar(30, 255, 255), image, hsv, "Yellow", false, false, IsShowWindows);
      auto redBlobs = getColorZonesRed(m_settings, cv::Scalar(10, 70, 60), cv::Scalar(350, 255, 80), image, hsv, "Red", false, false, false);
      auto blueBlobs = getColorZones(m_settings, cv::Scalar(78, 158, 124), cv::Scalar(138, 255, 255), image, hsv, "Blue", true, true, false);
      zones1 = IntersectionColorZones(image, yellowBlobs, blueBlobs, "BlueYellow", cv::Scalar(0, 0, 255), IsShowWindows);
      zones2 = IntersectionColorZones(image, redBlobs, blueBlobs, "RedBlue", cv::Scalar(0, 0, 255), IsShowWindows);
      if (zones2.size() > 0)
         zones1.insert(zones1.end(), zones2.begin(), zones2.end());
    }

    if (IsRecogSigns && zones1.size() > 0)
      RecogSigns(image, zones1, _mapSigns);

    return zones1;
}


bool SignDetector::CircleDetection(const cv::Mat& src_gray, const cv::Mat& src_display, int cannyThreshold, int accumulatorThreshold, std::string name)
{
    bool result = false;
    std::vector<cv::Vec3f> circles;
    HoughCircles(src_gray, circles, cv::HOUGH_GRADIENT,
      2,   // accumulator resolution (size of the image / 2)
      20,  // minimum distance between two circles
      100, // Canny high threshold
      std::max(src_gray.size().height, src_gray.size().width) / 5, // minimum number of votes
      0, 
      1000
    );
    auto Red = cv::Scalar(0, 0, 255);
    auto Green = cv::Scalar(0, 255, 0);
    auto Blue = cv::Scalar(255, 0, 0);
    cv::Mat display = src_display.clone();
    char buf[20] = "";
    int niters = 5;
    float SquareImage = (float)src_gray.size().area();
    cv::Rect interestRegionForNumber;
    int BaseRadiusLength = 10;
    bool GoodSizeAllCoins = false;
    for (size_t i = 0; i < circles.size(); i++)
    {
       cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
       int radius = cvRound(circles[i][2]);
       std::cout << "Programm found " << circles.size() << "coins" << std::endl;
       circle(display, center, radius, Red);
    }
    cv::imshow("Circle" + name, display);
    return result;
};
cv::Mat SignDetector::filterByMask(settings& m_settings, cv::Mat1b& mask) {
    for (int i = 2; i < mask.rows - 2; ++i)
       for (int j = 2; j < mask.cols - 2; ++j) 
       {
           if (((mask.at<uchar>(i - 1, j) > m_settings.MinBoarderLight && mask.at<uchar>(i + 1, j) > m_settings.MinBoarderLight) ||
              (mask.at<uchar>(i, j - 1) > m_settings.MinBoarderLight && mask.at<uchar>(i, j + 1) > m_settings.MinBoarderLight) ||
              (mask.at<uchar>(i, j - 2) > m_settings.MinBoarderLight && mask.at<uchar>(i, j + 2) > m_settings.MinBoarderLight && mask.at<uchar>(i, j - 1) == 0 && mask.at<uchar>(i, j + 1) == 0) ||
              (mask.at<uchar>(i - 2, j) > m_settings.MinBoarderLight && mask.at<uchar>(i + 2, j) > m_settings.MinBoarderLight && mask.at<uchar>(i - 1, j) == 0 && mask.at<uchar>(i + 1, j) == 0))
              && mask.at<uchar>(i, j) == 0)
           {
                mask.at<uchar>(i, j + 1) = 255;
            }
        }

    return cv::Mat(mask);
};
void SignDetector::drawSquares(cv::Mat& _image, const std::vector<std::vector<cv::Point> >& squares, std::vector<cv::Vec4i>& _lines, std::string _color)
{
    std::vector<cv::Point> res_pts_Rect;
    for (size_t i = 0; i < squares.size(); i++) 
    {
       const cv::Point* p = &squares[i][0];
       int n = (int)squares[i].size();
       auto tmp_rect = boundingRect(squares[i]);
       rectangle(_image, tmp_rect, cv::Scalar(0, 255, 0), 1);
       res_pts_Rect.push_back(cv::Point(tmp_rect.x, tmp_rect.y));
       res_pts_Rect.push_back(cv::Point(tmp_rect.x + tmp_rect.width, tmp_rect.y + tmp_rect.height));
    }
    auto img_rect = boundingRect(res_pts_Rect);
    cv::imshow("Result" + _color, _image);
};
cv::Mat SignDetector::drawSquaresBoth(const cv::Mat& image, const std::vector<std::vector<cv::Point> >& sqs, std::vector<cv::Vec4i>& _lines, std::string color)
{
    cv::Mat imgToShow(cv::Size(image.cols, image.rows), image.type());
    image.copyTo(imgToShow);
    drawSquares(imgToShow, sqs, _lines, color);
    return imgToShow;
};
std::vector<std::vector<cv::Point>> 
SignDetector::getColorZones(settings& m_settings, cv::Scalar lowLev, cv::Scalar highLev, cv::Mat& image, cv::Mat hsv, std::string nameColor, bool IsShowDebugWindows, bool IsShowCircles, bool IsShowResult) {
    std::vector<std::vector<cv::Point>> colorZones;
    cv::Mat probabilistic_hough;
    cv::Mat1b mask1e, mask2e;
    inRange(hsv, lowLev, highLev, mask1e);
    cv::Mat gray = filterByMask(m_settings, mask1e);
    if (IsShowDebugWindows) cv::imshow("gray" + nameColor, gray);
    dilate(gray, gray, cv::Mat(), cv::Point(-1, -1), 3);
    std::vector<cv::Vec4i> lines; 
    HoughLinesP(gray, lines, 1, CV_PI / 180, 50, 50, 10);
    cv::Canny(gray, probabilistic_hough, m_settings.MinBoarderLight, m_settings.MaxBoarderLight, 3);
    if (IsShowDebugWindows)
       cv::imshow("Debug" + nameColor, probabilistic_hough);
    if (IsShowCircles)
       CircleDetection(probabilistic_hough, image, 1, 100, nameColor);
    std::vector<std::vector<cv::Point> > contours;
    std::vector<std::vector<cv::Point>> squares;
    cv::findContours(probabilistic_hough, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);
    std::vector<cv::Point> approx;
    int maxWidthRibbonRect = boundingRect(contours[0]).width;
    for (size_t i = 0; i < contours.size(); i++)
    {
       approxPolyDP(contours[i], approx, arcLength(contours[i], true) * 0.02, true);
       double maxCosine = 0;
       if (approx.size() >= 3 && approx.size() <= 6)
       {
            auto contArea = contourArea(approx);
            auto _rect = boundingRect(approx);
            if ( contArea > m_settings.MinSquareRibbon || _rect.width > m_settings.MinLengthRibbon) 
            {
                if (m_settings.MaxSquareRibbon > contArea) 
                {
                  squares.push_back(approx);
                  colorZones.push_back(approx);
                }
            }
        }
    }
    if (IsShowResult) drawSquaresBoth(image, squares, lines, nameColor);
    return colorZones;
};
std::vector<std::vector<cv::Point>> 
SignDetector::getColorZonesRed(settings& m_settings, cv::Scalar lowLev, cv::Scalar highLev, cv::Mat& image, cv::Mat& hsv, std::string nameColor, bool IsShowDebugWindows, bool IsShowCircles, bool IsShowResult) {
    std::vector<std::vector<cv::Point>> colorZones;
    cv::Mat probabilistic_hough;
    cv::Mat1b mask1e, mask2e;
    inRange(hsv, cv::Scalar(0, 200, 150), cv::Scalar(4, 240, 255), mask1e);
    inRange(hsv, cv::Scalar(172, 200, 150), cv::Scalar(180, 240, 255), mask2e);
    cv::Mat1b mask = mask1e | mask2e;
    if (IsShowDebugWindows) cv::imshow("Mask" + nameColor, mask);

    cv::Mat gray = filterByMask(m_settings, mask);
    if (IsShowDebugWindows) cv::imshow("gray" + nameColor, gray);
    
    dilate(gray, gray, cv::Mat(), cv::Point(-1, -1), 3);
    if (IsShowCircles) CircleDetection(mask, image, 50, 50, nameColor);

    std::vector<cv::Vec4i> lines; 
    HoughLinesP(gray, lines, 1, CV_PI / 180, 50, 50, 10);
    cv::Canny(gray, probabilistic_hough, m_settings.MinBoarderLight, m_settings.MaxBoarderLight, 3);
    std::vector<std::vector<cv::Point> > contours;
    std::vector<std::vector<cv::Point>> squares;
    cv::findContours(probabilistic_hough, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);
    std::vector<cv::Point> approx;
    int maxWidthRibbonRect = boundingRect(contours[0]).width;
    for (size_t i = 0; i < contours.size(); i++)
    {
       approxPolyDP(contours[i], approx, arcLength(contours[i], true) * 0.02, true);
       double maxCosine = 0;
       if (approx.size() >= 3)
       {
            auto contArea = contourArea(approx);
            auto _rect = boundingRect(approx);
            if (contArea > m_settings.MinSquareRibbon || _rect.width > m_settings.MinLengthRibbon) 
            {
                if (m_settings.MaxSquareRibbon > contArea) 
                {
                    squares.push_back(approx);
                    colorZones.push_back(approx);
                }
            }
        }
    }
    if (IsShowResult) drawSquaresBoth(image, squares, lines, nameColor);
    return colorZones;
};

void SignDetector::IntersectionColorZones(cv::Mat& img,
    std::vector<std::vector<cv::Point>>& z1,
    std::vector<std::vector<cv::Point>>& z2,
    std::string nameBlob,
    cv::Scalar color)
{
    std::vector<cv::Rect> res;
    for (auto& r1 : z1) 
    {
        for (auto& r2 : z2) 
        {
            std::vector<cv::Point2f> Pt;
            std::vector<cv::Point2f> PartitialFig;
            if (cv::INTERSECT_PARTIAL == (rotatedRectangleIntersection(cv::minAreaRect(r1), cv::minAreaRect(r2), PartitialFig)) ||
               cv::INTERSECT_FULL == (rotatedRectangleIntersection(cv::minAreaRect(r1), cv::minAreaRect(r2), Pt)))
            {
                if (!PartitialFig.empty()) 
                {
                    r1.insert(r1.end(), r2.begin(), r2.end());
                    rectangle(img, cv::boundingRect(r1), color);
                }
                if (!Pt.empty())
                   rectangle(img, cv::boundingRect(Pt), color);
            }
        }
    }
    cv::imshow("Mix" + nameBlob, img);
}

std::vector<cv::Rect>
SignDetector::IntersectionColorZones(
    cv::Mat& img,
    std::vector<std::vector<cv::Point>>& z1,
    std::vector<std::vector<cv::Point>>& z2,
    std::string nameBlob,
    cv::Scalar color,
    bool IsShow) {
    std::vector<cv::Rect> res;
    for (auto& r1 : z1) 
    {
        for (auto& r2 : z2) 
        {
            std::vector<cv::Point2f> Pt;
            std::vector<cv::Point2f> PartitialFig;
            if (cv::INTERSECT_PARTIAL == (rotatedRectangleIntersection(cv::minAreaRect(r1), cv::minAreaRect(r2), PartitialFig)) ||
                cv::INTERSECT_FULL == (rotatedRectangleIntersection(cv::minAreaRect(r1), cv::minAreaRect(r2), Pt)))
            {
                cv::Rect rec;
                if (!PartitialFig.empty()) 
                {
                    r1.insert(r1.end(), r2.begin(), r2.end());
                    rec = cv::boundingRect(r1);
                    res.push_back(rec);
                }
                if (!Pt.empty()) {
                    rec = cv::boundingRect(Pt);
                    res.push_back(rec);
                }
                if (IsShow) rectangle(img, rec, color);
            }
        }
    }
    if (IsShow) cv::imshow("Mix" + nameBlob, img);
    return res;
}

void SignDetector::RecogSigns(cv::Mat image, std::vector<cv::Rect> zones1, std::map<cv::Rect, typeSign>& mapSigns) {
    for (auto& zone : zones1) 
        std::cout << "rect:" << zone << std::endl;
}