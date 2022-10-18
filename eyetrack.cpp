#include <opencv2/opencv.hpp>
#include <stdio.h>

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480
#define FRAME_RATE 15

int size_of_mosaic = 0;

int main(int argc, char *argv[]){

    // 1. load classifier
    std::string cascadeName = "/usr/share/opencv4/haarcascades/haarcascade_eye.xml"; //Haar−like
    //std::string cascadeName = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml"; //Haar−like
    cv::CascadeClassifier cascade;
    if(!cascade.load(cascadeName)){
        printf("ERROR: cascadeFile not found\n");
        return -1;
    }

    // 2. initialize VideoCapture
    cv::Mat frame;
    cv::VideoCapture cap;
    cap.open(0, cv::CAP_V4L2);

    cap.set(cv::CAP_PROP_FPS, FRAME_RATE);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    cap >> frame;
    if(frame.empty()){
        return 0;
    }

    // 3. prepare window and trackbar
    cv::namedWindow("result", 1);


    double scale = 1.0;
    cv::Mat gray, smallImg(cv::saturate_cast<int>(frame.rows / scale), cv::saturate_cast<int>(frame.cols / scale), CV_8UC1);
    std::vector<cv::Rect> eyes;
    cv::Point center;
    int radius;

    for(;;){

        // 4. capture frame
        cap >> frame;
        
        // convert to gray scale
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // 5. scale −down the image
        cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
        cv::equalizeHist(smallImg, smallImg);

        // 6. detect face using Haar− classifier
        //std::vector<cv::Rect> eyes;
        // multi−scale face searching
        // image, size , scale , num, flag , smallest rect
        cascade.detectMultiScale(smallImg, eyes,
        1.1,
        2,
        cv::CASCADE_SCALE_IMAGE,
        cv::Size(30, 30));
        // 7. mosaic( pixelate ) face−region
        for(int i = 0; i < 2; i++){
            //cv::Point center;
            //int radius;
            center.x = cv::saturate_cast<int>((eyes[i].x + eyes[i].width * 0.5) * scale);
            center.y = cv::saturate_cast<int>((eyes[i].y + eyes[i].height * 0.5) * scale);
            radius = cv::saturate_cast<int>((eyes[i].width + eyes[i].height) * 0.25 * scale);

            cv::Rect roi_rect(center.x - radius, center.y - radius, radius * 2, radius * 2);


            cv::rectangle(frame, roi_rect, 0);//rectangle

        }

        // 8. show mosaiced image to window
        cv::imshow("result", frame);

        int key = cv::waitKey(10);
        if(key == 'q' || key == 'Q')
        break;

    }
    return 0;
}