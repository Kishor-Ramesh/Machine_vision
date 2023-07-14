//program to measure the dimensions of a given object
//instructions: 
//1. a reference object of size 2 x 2cm is to be placed or drawn.
//2. make sure the reference object is the first object on the left hand side.
//2. when prompted for the file name of the image, make sure to include the extension as well.


#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

Mat image, gray, blurred, canny, dilation, kernel;


int main()
{
    // Read input image
    string name;
    cout << "Enter the file name of your image : " << endl;
    cin >> name;
    Mat image = imread(name, -1);
    if (image.empty())
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    //performing image preprocessing operations
    cvtColor(image, gray, COLOR_BGR2GRAY); // converting to grayscale to reduce computation.
    GaussianBlur(gray, blurred, Size(7, 5), 10, 10); // to remove noise.
    Canny(blurred, canny, 10, 33); // to detect edges.
    kernel = getStructuringElement(MORPH_RECT, Size(7, 5)); // to make sure the boundaries of edges are closed or well highlighted for proper segmentation.
    dilate(canny, dilation, kernel);
   

    resize(image, image, Size(), 0.50, 0.50);
    resize(gray, gray, Size(), 0.50, 0.50);
    resize(blurred, blurred, Size(), 0.50, 0.50);
    resize(canny, canny, Size(), 0.50, 0.50);
    resize(dilation, dilation, Size(), 0.50, 0.50);

    //creating a vector of vector of points to store the set of x,y coordinates of all the contours
    vector<vector<Point>> contours;
    findContours(dilation, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Removing small contours below the threshold of area and storing the necessary ones in filteredContours
    vector<vector<Point>> filteredContours;
    for (auto contour : contours)
    {
        if (contourArea(contour) > 1000)
            filteredContours.push_back(contour);
    }

    // Sort contours from left to right to find the reference objects
    sort(filteredContours.begin(), filteredContours.end(), [](const vector<Point>& c1, const vector<Point>& c2) {
        return boundingRect(c1).x < boundingRect(c2).x;
        });


    // caliberating the measurement value 
    double referenceObjectWidth = 0;
    for (size_t i = 0; i < filteredContours.size(); i++) {
        RotatedRect rotatedRect = minAreaRect(filteredContours[i]);
        double aspectRatio = rotatedRect.size.width / rotatedRect.size.height;
        if (aspectRatio > 0.9 && aspectRatio < 1.1) {
            referenceObjectWidth = rotatedRect.size.width;
            break;
        }
    }
         
    // Computing the results
    double errorfunction = 2 / referenceObjectWidth; // 2 cm is the width of the reference object. if an object of different width is used, the number 2 can be changed to the used width.
    for (size_t i = 0; i < filteredContours.size(); i++) {
        RotatedRect rotatedRect = minAreaRect(filteredContours[i]);
        double objectWidth = rotatedRect.size.width * errorfunction;
        double objectHeight = rotatedRect.size.height * errorfunction;
        Point2f rect_points[4];
        rotatedRect.points(rect_points);
        for (int j = 0; j < 4; j++) {
            line(image, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 0), 2);
        }
        putText(image, format("%.2f", objectWidth) + "x" + format("%0.2fcm", objectHeight), rect_points[1], FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
        cout << format("%0.2f" ,objectWidth )<< "x" << format("%0.2fcm", objectHeight) << endl;
    }

    // Display the result
    //namedWindow("Result", WINDOW_NORMAL);
    imshow("Result", image);
    //imshow("Result2", dilation);
    waitKey(0);

    return 0;
}