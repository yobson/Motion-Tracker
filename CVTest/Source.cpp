
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <opencv2\highgui.hpp>
#include <opencv2\opencv.hpp>
#include "opencv2\videoio.hpp"
#include <ctime>

using namespace::std;
using namespace cv;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
void on_trackbar(int, void*)
{//This function gets called whenever a

}
string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	resizeWindow(trackbarWindowName, 330, 500);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}
void drawObject(int x, int y, Mat &frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25>0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25<FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25>0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25<FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);

}
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				//putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
void printINS(bool isY, bool morph, bool start, int centrerx, int centery) {
	system("cls");
	cout << "Keys to change settings:" << endl << "p: change plane X/Y\nr: reset clock and start";
	cout << endl << "s: start/stop\nm: Morph Filtering On/Off\nc: center at object position and clear graphics data\nq: quit" << endl;
	cout << "d: debugging on/off" << endl;
	cout << endl;
	cout << "Plane: " << (isY ? "Y" : "X") << endl;
	cout << "Status: " << (start ? "Runnng" : "Stopped") << endl;
	cout << "Center X: " << centrerx << " Y: " << centery << endl;
	cout << "Morph: " << (morph ? "On" : "Off") << endl;
}

int main(int argc, char* argv[])
{
	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = false;
	bool debug = false;
	double timeItorator = 0;
	clock_t begin_time = clock();
	float time;
	bool start = false;
	int data;
	bool usey = true;
	
	ofstream plotdata;
	plotdata.open("C:/gnuplot/plot.dat", std::ios_base::app);

	//char *path = "C:/gnuplot/bin/gnuplot -persist";
	//FILE *pipe = _popen(path, "w");
	//fprintf(pipe, "set term wx\n");
	//fflush(pipe);
	bool loop = true;
	
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	int x = 0, y = 0, oldX = 0, oldY = 0, offsetX = 200, offsetY = 200, newX = 0, newY = 0;
	//create slider bars for HSV filtering
	createTrackbars();
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open(0);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	printINS(usey, useMorphOps, start, offsetX, offsetY);
	while (loop) {
		capture.read(cameraFeed);
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if (useMorphOps)
			morphOps(threshold);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if (trackObjects) {
			trackFilteredObject(x, y, threshold, cameraFeed);
			newX = x - offsetX;
			newY = offsetY - y;
			time = (clock() - begin_time)/(float)CLOCKS_PER_SEC;
			
		}
		if (x != oldX && y != oldY && start) {
			if (debug) { cout << "X:" << newX << " Y:" << newY << " Time:" << time << endl; }
			if (usey) { data = newY; }
			else { data = newX;  }
			plotdata << time << "	" << data << endl;
		}

		//show frames 
		imshow(windowName2, threshold);
		imshow(windowName, cameraFeed);
		imshow(windowName1, HSV);


		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);

		short qKeyState = GetAsyncKeyState('Q');
		short rKeyState = GetAsyncKeyState('R');
		short sKeyState = GetAsyncKeyState('S');
		short cKeyState = GetAsyncKeyState('C');
		short mKeyState = GetAsyncKeyState('M');
		short pKeyState = GetAsyncKeyState('P');
		short dKeyState = GetAsyncKeyState('D');

		if (qKeyState) { loop = false; printINS(usey, useMorphOps, start, offsetX, offsetY); }
		if (rKeyState) {  begin_time = clock(); start = true; printINS(usey, useMorphOps, start, offsetX, offsetY); }
		if (sKeyState) { start = !start; printINS(usey, useMorphOps, start, offsetX, offsetY); }
		if (cKeyState) { 
			offsetX = x; offsetY = y;
			plotdata.close();
			plotdata.open("C:/gnuplot/plot.dat", fstream::out | fstream::trunc);
			plotdata << "";
			plotdata.close();
			plotdata.open("C:/gnuplot/plot.dat", std::ios_base::app);
			printINS(usey, useMorphOps, start, offsetX, offsetY);
		}
		if (mKeyState) { useMorphOps = !useMorphOps; printINS(usey, useMorphOps, start, offsetX, offsetY); }
		if (pKeyState) { usey = !usey; printINS(usey, useMorphOps, start, offsetX, offsetY); }
		if (dKeyState) { debug = !debug; printINS(usey, useMorphOps, start, offsetX, offsetY); }

		oldX = x;
		oldY = y;
	}

	//_pclose(pipe);
	plotdata.close();
	return 0;
}
