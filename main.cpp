#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>

cv::Mat imgG, imgR, imgB, img, frame;
std::vector<cv::Mat> arr, arr2;

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

const int MAX_NUM_OBJECTS = 50;
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;

int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

const int size = 50;

bool mouseMove, rectangleSelected, calibrationMode, mouseIsDragging;
cv::Point initialClickPoint, currentMousePoint;
cv::Rect rectangleROI;
std::vector<int> H_ROI, S_ROI, V_ROI;

const std::string windowName("INPUT");

std::string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void drawObject(int x, int y, cv::Mat &frame){
	cv::circle(frame, cv::Point(x, y), 20, cv::Scalar(0, 255, 0), 2);
	if (y - 25>0)cv::line(frame, cv::Point(x, y), cv::Point(x, y - 25), cv::Scalar(0, 255, 0), 2);

	else cv::line(frame, cv::Point(x, y), cv::Point(x, 0), cv::Scalar(0, 255, 0), 2);
	if (y + 25<FRAME_HEIGHT)cv::line(frame, cv::Point(x, y), cv::Point(x, y + 25), cv::Scalar(0, 255, 0), 2);

	else cv::line(frame, cv::Point(x, y), cv::Point(x, FRAME_HEIGHT), cv::Scalar(0, 255, 0), 2);
	if (x - 25>0)cv::line(frame, cv::Point(x, y), cv::Point(x - 25, y), cv::Scalar(0, 255, 0), 2);

	else cv::line(frame, cv::Point(x, y), cv::Point(0, y), cv::Scalar(0, 255, 0), 2);
	if (x + 25<FRAME_WIDTH)cv::line(frame, cv::Point(x, y), cv::Point(x + 25, y), cv::Scalar(0, 255, 0), 2);

	else cv::line(frame, cv::Point(x, y), cv::Point(FRAME_WIDTH, y), cv::Scalar(0, 255, 0), 2);

	cv::putText(frame, intToString(x) + "," + intToString(y), cv::Point(x, y + 30), 1, 1, cv::Scalar(0, 255, 0), 2);
}


void clickAndDrag_Rectangle(int event, int x, int y, int flags, void *param) {
    if (calibrationMode == true){
		cv::Mat* videoFeed = (cv::Mat*) param;

		if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false) {
			initialClickPoint = cv::Point(x, y);
			mouseIsDragging = true;
		}

		if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true) {
			currentMousePoint = cv::Point(x, y);
			mouseMove = true;
		}

		if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true) {
			rectangleROI = cv::Rect(initialClickPoint, currentMousePoint);

			mouseIsDragging = false;
			mouseMove = false;
			rectangleSelected = true;
		}

		if (event == CV_EVENT_RBUTTONDOWN) {
			H_MIN = 0;
			S_MIN = 0;
			V_MIN = 0;
			H_MAX = 255;
			S_MAX = 255;
			V_MAX = 255;
		}

		if (event == CV_EVENT_MBUTTONDOWN) {

		}
	}
}

void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame){
	if (mouseMove == false && rectangleSelected == true){
		
		if (H_ROI.size()>0) H_ROI.clear();
		if (S_ROI.size()>0) S_ROI.clear();
		if (V_ROI.size()>0) V_ROI.clear();

        if (rectangleROI.width < 1 || rectangleROI.height < 1) printf("Please drag a rectangle, not a line\n");
        else {
			for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++){
				for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++){
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}

		rectangleSelected = false;
					
		if (H_ROI.size() > 0) {
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
            printf("MIN 'H' VALUE: %d\nMAX 'H' VALUE: %d\n", H_MIN, H_MAX);
		}
		if (S_ROI.size() > 0) {
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
            printf("MIN 'S' VALUE: %d\nMAX 'S' VALUE: %d\n", S_MIN, S_MAX);
		}
		if (V_ROI.size() > 0) {
			V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
			V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
            printf("MIN 'V' VALUE: %d\nMAX 'V' VALUE: %d\n", V_MIN, V_MAX);
		}
	}

	if (mouseMove == true) {
		rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
	}
}

void trackFilteredObject(int &x, int &y, cv::Mat threshold, cv::Mat &cameraFeed) {
    int Xc, Yc, counter;
	cv::Mat temp;

	// if (!rectangleSelected) return;
	threshold.copyTo(temp);


    Xc = Yc = counter = 0;
    int c=0, c2=0;
    for (y=0; y < temp.rows; y++) {
        uchar* ptr = (uchar*) (temp.data + y * temp.step);
        for (x = 0; x < temp.cols; x++) {
            if(ptr[x] > 0) {
                Xc += x;
                Yc += y;
                counter++;
                c++;
            } else { if (c > c2) c2=c; c=0; }
        }
    }

    if (counter) {
        // printf("COORD: (%.02f; %.02f)\n", float(Xc)/counter, float(Yc)/counter);
        cv::Point p(float(Xc)/counter, float(Yc)/counter);
        cv::circle(frame, p, c2/2, CV_RGB(255, 0, 0));
		if ((p.x < (FRAME_WIDTH/2+size) && p.x > (FRAME_WIDTH/2-size)) && (p.y < (FRAME_HEIGHT/2+size) && p.y > (FRAME_HEIGHT/2-size))) printf("in centre COORD: (%.02f; %.02f)\n", float(Xc)/counter, float(Yc)/counter);
    }

	// std::vector<std::vector<cv::Point>> contours;
	// std::vector<cv::Vec4i> hierarchy;

	// findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	// double refArea = 0;
	// int largestIndex = 0;
	// bool objectFound = false;
	// if (hierarchy.size() > 0) {
	// 	int numObjects = hierarchy.size();
	// 	if (numObjects < MAX_NUM_OBJECTS) {
	// 		for (int index = 0; index >= 0; index = hierarchy[index][0]) {

	// 			cv::Moments moment = moments((cv::Mat)contours[index]);
	// 			double area = moment.m00;

	// 			if (area > MIN_OBJECT_AREA && area < MAX_OBJECT_AREA && area > refArea) {
	// 				x = moment.m10 / area;
	// 				y = moment.m01 / area;
	// 				objectFound = true;
	// 				refArea = area;
	// 				//save index of largest contour to use with drawContours
	// 				largestIndex = index;
	// 			} else objectFound = false;
	// 		}

	// 		if (objectFound == true){
	// 			// putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
	// 			drawObject(x, y, cameraFeed);
	// 		}

	// 	} // else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	// }
}

void move() {
	
}


int main() {
    cv::Mat HSV, out, threshold;
    int x, y, counter, Yc, Xc;
    calibrationMode =  true;

    cv::namedWindow(windowName);
    cv::setMouseCallback(windowName, clickAndDrag_Rectangle, &frame);
    mouseIsDragging = false;
	mouseMove = false;
	rectangleSelected = false;

    cv::VideoCapture camera; 
    camera.open(0);
    if (!camera.isOpened()) return -1;

    // КОСТЫЛЬ!!!
    camera.read(frame);
    imgB = imgG = imgR = frame;
    printf(": %dx%d\n", frame.cols, frame.rows);

    while (camera.read(frame)) {
        cv::split(frame, arr);
        cvtColor(frame, HSV, cv::COLOR_BGR2HSV);
        recordHSV_Values(frame, HSV);

        inRange(HSV, cv::Scalar(H_MIN, S_MIN, V_MIN), cv::Scalar(H_MAX, S_MAX, V_MAX), threshold);

        trackFilteredObject(x, y, threshold, frame);

        imshow("HSV", HSV);
        imshow("threshold", threshold);

        // cv::imshow("BLUE", imgB);
        // cv::imshow("GREEN", imgG);
        // cv::imshow("RED", imgR);

        // cv::imshow("BLUE", arr[0]);
        // cv::imshow("GREEN", arr[1]);
        // cv::imshow("RED", arr[2]);

        // cv::inRange(arr[0], cv::Scalar(47), cv::Scalar(255), imgB);
        // cv::inRange(arr[1], cv::Scalar(85), cv::Scalar(255), imgG);
        // cv::inRange(arr[2], cv::Scalar(0), cv::Scalar(75), imgR);        

        // cv::bitwise_and(imgB, imgG, img);
        // cv::bitwise_and(img, imgR, img);
        
        // cv::imshow("OUT", img);

        // Xc = Yc = counter = 0;
        // int c=0, c2=0;
        // for (y=0; y < img.rows; y++) {
        //     uchar* ptr = (uchar*) (img.data + y * img.step);
        //     for (x = 0; x < img.cols; x++) {
        //         if(ptr[x] > 0) {
        //             Xc += x;
        //             Yc += y;
        //             counter++;
        //             c++;
        //         } else { if (c > c2) c2=c; c=0; }
        //     }
        // }

        // printf("%d %d %d", Xc, Yc, counter);
        // if (counter) {
        //     printf("COORD: (%.02f; %.02f)\n", float(Xc)/counter, float(Yc)/counter);
        //     cv::Point p(float(Xc)/counter, float(Yc)/counter);
        //     cv::circle(frame, p, c2/2, CV_RGB(255, 0, 0));
        // }

		if (false) {
			cv::line(frame, cv::Point(FRAME_WIDTH/2-size, FRAME_HEIGHT/2-size), cv::Point(FRAME_WIDTH/2+size, FRAME_HEIGHT/2-size), cv::Scalar(0, 255, 0), 2);
			cv::line(frame, cv::Point(FRAME_WIDTH/2-size, FRAME_HEIGHT/2-size), cv::Point(FRAME_WIDTH/2-size, FRAME_HEIGHT/2+size), cv::Scalar(0, 255, 0), 2);
			cv::line(frame, cv::Point(FRAME_WIDTH/2+size, FRAME_HEIGHT/2+size), cv::Point(FRAME_WIDTH/2-size, FRAME_HEIGHT/2+size), cv::Scalar(0, 255, 0), 2);
			cv::line(frame, cv::Point(FRAME_WIDTH/2+size, FRAME_HEIGHT/2+size), cv::Point(FRAME_WIDTH/2+size, FRAME_HEIGHT/2-size), cv::Scalar(0, 255, 0), 2);
		}

        cv::imshow(windowName, frame);
		
		if (cv::waitKey(10) == 27) break;
    }

    return 0;
}