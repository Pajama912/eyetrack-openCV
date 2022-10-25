#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <unistd.h>

int INIT_TIME = 50;
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480
#define FRAME_RATE 15
int sens = 10;
void eye_track();

int main(int argc, char *argv[]){
    cv::Mat image;
    image = cv::imread("example.png");
    if(image.empty()){
    printf("ERROR: image not found!\n");
    return 0;
    }

    // 1. load classifier
    std::string cascadeName = "/usr/share/opencv4/haarcascades/haarcascade_eye.xml"; //Haar−like
    std::string cascadeName2 = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml"; //Haar−like
    cv::CascadeClassifier cascade;
    if(!cascade.load(cascadeName)){
        printf("ERROR: cascadeFile not found\n");
        return -1;
    }
    cv::CascadeClassifier cascade2;
    if(!cascade2.load(cascadeName2)){
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
    cv::createTrackbar("sens", "result", &sens, 30, 0);
    cv::namedWindow("eye", 1);

    double scale = 1.0;
    double scale2 = 4.0;
    cv::Mat gray;
    cv::Mat smallImg(cv::saturate_cast<int>(frame.rows / scale), cv::saturate_cast<int>(frame.cols / scale), CV_8UC1);
    cv::Mat smallImg3;
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> eyes;
    //cv::Mat roi_src;
    cv::Mat smallImg2(cv::saturate_cast<int>(frame.rows / scale2),
    cv::saturate_cast<int>(frame.cols / scale2), CV_8UC1);
    cv::Point2f center;
    cv::Point2f center2;
    cv::Mat roi_clone;
    cv::Mat gray_clone;

    //cv::Point prevcenter;
    int radius;
    int radius2;
    cv::Moments mu;
    cv::Point2f mc;
    cv::Point current_eye_center;
    cv::Point eyesight;
    cv::Point eyesight2;
    cv::Point2f center_eye_center;

    printf("look at the center of the screen\n");
    for(int i = 3; i > 0; --i){
        printf("%d\n",i);
        sleep(1);
    }
    for( int j = 0; j < INIT_TIME; j++){
        //printf("%d", j);
        cap >> frame;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::resize(gray, smallImg2, smallImg2.size(), 0, 0, cv::INTER_LINEAR);
        cv::equalizeHist(smallImg2, smallImg2);
        cascade2.detectMultiScale(smallImg2, faces,
        1.1,
        2,
        cv::CASCADE_SCALE_IMAGE,
        cv::Size(30, 30));
        center2.x = cv::saturate_cast<int>((faces[0].x + faces[0].width * 0.5) * scale2);
        center2.y = cv::saturate_cast<int>((faces[0].y + faces[0].height * 0.5) * scale2);
        radius2 = cv::saturate_cast<int>((faces[0].width + faces[0].height) * 0.25 * scale2);
        cv::Rect roi_rect2(center2.x - radius2, center2.y - 0.5 * radius2, radius2, 0.5 * radius2);
        smallImg3 = gray(roi_rect2).clone();
        cv::equalizeHist(smallImg3, smallImg3);
        cascade.detectMultiScale(smallImg3, eyes,
        1.1,
        2,
        cv::CASCADE_SCALE_IMAGE,
        cv::Size(30, 30));
        for(int i = 0; i < 1; i++){
            center.x = cv::saturate_cast<int>(center2.x - radius2 + (eyes[i].x + eyes[i].width * 0.5) * scale);
            center.y = cv::saturate_cast<int>(center2.y - 0.5 * radius2 + (eyes[i].y + eyes[i].height * 0.5) * scale);
            radius = cv::saturate_cast<int>((eyes[i].width + eyes[i].height) * 0.25 * scale);
            cv::Rect roi_rect(center.x - 0.8 * radius, center.y - 0.4 * radius, radius * 1.6, radius * 0.8);
            roi_clone = gray(roi_rect).clone();
        }
        cv::threshold(roi_clone,roi_clone,50,255,cv::THRESH_BINARY);
        roi_clone = 255 - roi_clone;
        mu = moments(roi_clone, true);
        mc = cv::Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
        //printf("%lf,%lf,%lf\n", mc.x, double(radius),mc.x / double(radius) / 1.6 / INIT_TIME);
        if(radius == 0 || isnan(mc.x) || isnan(mc.y)){
            //when the moment returns a nan, do it again.
            j--;
        } else {
            center_eye_center.x += (mc.x + center.x - 0.8 * radius) / INIT_TIME;
            center_eye_center.y += (mc.y + center.y - 0.4 * radius) / INIT_TIME;
        }
    }

    int i;
    double x, y;
    double prevx, prevy;
    for(;;){
        // 4. capture frame
        cap >> frame;
    
        // convert to gray scale
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        cv::resize(gray, smallImg2, smallImg2.size(), 0, 0, cv::INTER_LINEAR);
        cv::equalizeHist(smallImg2, smallImg2);
        cascade2.detectMultiScale(smallImg2, faces,
        1.1,
        2,
        cv::CASCADE_SCALE_IMAGE,
        cv::Size(30, 30));
        center2.x = cv::saturate_cast<int>((faces[0].x + faces[0].width * 0.5) * scale2);
        center2.y = cv::saturate_cast<int>((faces[0].y + faces[0].height * 0.5) * scale2);
        radius2 = cv::saturate_cast<int>((faces[0].width + faces[0].height) * 0.25 * scale2);
        cv::Rect roi_rect2(center2.x - radius2, center2.y - 0.5 * radius2, radius2, 0.5 * radius2);

        // 5. scale −down the image
        smallImg3 = gray(roi_rect2).clone();
        cv::equalizeHist(smallImg3, smallImg3);

        // 6. detect face using Haar− classifier
        cascade.detectMultiScale(smallImg3, eyes,
        1.1,
        2,
        cv::CASCADE_SCALE_IMAGE,
        cv::Size(30, 30));
        
        // 7. mosaic( pixelate ) face−region
        for(int i = 0; i < 1; i++){
            //cv::Point center;
            //int radius;
            center.x = cv::saturate_cast<int>(center2.x - radius2 + (eyes[i].x + eyes[i].width * 0.5) * scale);
            center.y = cv::saturate_cast<int>(center2.y - 0.5 * radius2 + (eyes[i].y + eyes[i].height * 0.5) * scale);
            radius = cv::saturate_cast<int>((eyes[i].width + eyes[i].height) * 0.25 * scale);
            
            cv::Rect roi_rect(center.x - 0.8 * radius, center.y - 0.4 * radius, radius * 1.6, radius * 0.8);

            //prevcenter = center;

            cv::rectangle(frame, roi_rect, 0);//rectangle
            cv::rectangle(frame, roi_rect2, 0);//rectangle


            roi_clone = gray(roi_rect).clone();

        }
        //resize(roi_clone, roi_clone, cv::Size(), 800.0/roi_clone.cols ,400.0/roi_clone.rows);
        cv::threshold(roi_clone,roi_clone,50,255,cv::THRESH_BINARY);
        //chenge into binary. 48 was the right value for the eye's blackness
        roi_clone = 255 - roi_clone;
        //moments will take moment of the white part, so swap black and white
        mu = moments(roi_clone, true);
        //true -> uniformal moment
        mc = cv::Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
        //change moment into Point
        roi_clone = 255 - roi_clone;
        //swapblack and white again.(only for "eye" screens interface)
        cv::circle( roi_clone, mc, 140, cv::Scalar(100), 2, 4);
        cv::imshow("eye", roi_clone);

        // 8. show mosaiced image to window
        current_eye_center.x = center.x - 0.8 * radius + mc.x;
        current_eye_center.y = center.y - 0.4 * radius + mc.y;
        
        cv::circle(frame, current_eye_center,2, cv::Scalar(0,0,255), 2, 4);
        //cv::circle(frame, center_eye_center,1, cv::Scalar(0,255,0), 2, 4);
        cv::drawMarker(frame, center_eye_center,(0, 255, 0),
               cv::MARKER_CROSS,
               20,
               2,
               cv::LINE_4
               );
        double sensitivity = sens / 10.0;
        x = 0.5*FRAME_WIDTH + sens * (mc.x + center.x - 0.8 * radius - center_eye_center.x);
        y = 0.5*FRAME_HEIGHT + sens * (mc.y + center.y - 0.4 * radius - center_eye_center.y);
        
        x = FRAME_WIDTH - x;//flip horizontally
        
        if(x<0){
            eyesight.x = 0;
        } else if(x>FRAME_WIDTH){
            eyesight.x = FRAME_WIDTH;
        } else {
            eyesight.x = x;
        }
        if(y<0){
            eyesight.y = 0;
        } else if(y>FRAME_HEIGHT){
            eyesight.y = FRAME_HEIGHT;
        } else {
            eyesight.y = y;
        }

        x = 0.5 * image.cols + sens * (mc.x + center.x - 0.8 * radius - center_eye_center.x);
        y = 0.5 * image.rows + sens * (mc.y + center.y - 0.4 * radius - center_eye_center.y);
        
        x = image.cols - x;//flip horizontally
        if(isnan(x) || isnan(y)){
            x = prevx;
            y = prevy;
        } else {
            x = (prevx + x)/2;
            y = (prevy + y)/2;//before/after the if may be better
        }
        //printf("x:%lf,y:%lf\n", x, y);
        if(x<0){
            eyesight.x = 0;
        } else if(x>image.cols){
            eyesight2.x = image.cols;
        } else {
            eyesight2.x = x;
        }
        if(y<0){
            eyesight2.y = 0;
        } else if(y>image.rows){
            eyesight2.y = image.rows;
        } else {
            eyesight2.y = y;
        }
        //printf("%d,%d\n", eyesight2.x, eyesight2.y);
        image = cv::imread("example.png");
        cv::circle(image, eyesight2, 7, cv::Scalar(100,100,255),2,4);
        cv::circle(image, eyesight2,5, cv::Scalar(0,0,255), 2, 4);
        cv::circle(image, eyesight2,2, cv::Scalar(255,255,255), 2, 4);
        cv::circle(frame, eyesight,5, cv::Scalar(0,0,0), 2, 4);

        cv::imshow("result", frame);
        cv::imshow("slide", image);

        int key = cv::waitKey(10);
        if(key == 'q' || key == 'Q')
        break;

        /*if(key == 's' || key == 'S'){
            center_eye_center.x = x;
            center_eye_center.y = y;
        }*/
        
        prevx = x;
        prevy = y;

    }
    return 0;
}
