#include<iostream>
#include<vector>
#include<thread>
#include<chrono>
#include<opencv2/opencv.hpp>
#include<fstream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
using namespace cv;
using namespace chrono;

const string ASCII_CHARS = "Ñ@#W$9876543210?!abc;:+=-,._ ";

char getPixelasASCIIChar(int pixel_intensity)
{
    return ASCII_CHARS[pixel_intensity * ASCII_CHARS.length() / 256];
}

string getASCIIFrame(Mat* frame, int* output_width, int* output_height)
{
    Mat frame_gray, frame_resized;
    cvtColor(*frame, frame_gray, COLOR_BGR2GRAY);
    resize(frame_gray, frame_resized, Size(*output_width, *output_height), 0, 0, INTER_LINEAR);

    string ascii_frame = "";
    for (int i = 0; i < *output_height; i++) {
        for (int j = 0; j < *output_width; j++) {
            ascii_frame += getPixelasASCIIChar(frame_resized.at<uchar>(i, j));
        }
        ascii_frame += "\n";
    }
    return ascii_frame;
}

vector<string> getASCIIFramesFromVideo(VideoCapture* video_file)
{
    int frame_width = video_file->get(CAP_PROP_FRAME_WIDTH);
    int frame_height = video_file->get(CAP_PROP_FRAME_HEIGHT);
    int output_width = 300;
    int output_height = (output_width * frame_height / frame_width) / 2; // we want to keep frame ratio of original video in console

    vector<string> ascii_frames;
    Mat frame;
    while (true)
    {
        (*video_file) >> frame;
        if (frame.empty())
            break;
        ascii_frames.push_back(getASCIIFrame(&frame, &output_width, &output_height));
    }
    return ascii_frames;
}

void clearConsole()
{
    #ifdef _WIN32
        // Windows: Move the cursor to the top-left corner of the console
        COORD coord = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    #else
        // macOS/Linux: Use ANSI escape code to clear the screen
        cout << "\033[H\033[2J" << flush;
    #endif
}

void playVideoAsASCIIFrames(vector<string>* ascii_frames, int* frame_duration)
{
    #ifdef _WIN32 // clear the console
        system("cls");
    #else
        system("clear")
    #endif

    for (int i = 0; i < ascii_frames->size(); i++)
    {
        clearConsole();
        cout << (*ascii_frames)[i];
        std::this_thread::sleep_for(std::chrono::milliseconds(*frame_duration));
    }
}

int main()
{
    // Block console from any text (OpenCV prints some info...)
    streambuf* cout_sbuf = std::cout.rdbuf(); // save original sbuf
    ofstream fout("/dev/null");
    cout.rdbuf(fout.rdbuf()); // redirect 'cout' to a 'fout'
    //

    string video_path = "C:/my_video.mp4";
    VideoCapture video_file(video_path);
    int frame_duration = 1000 / video_file.get(CAP_PROP_FPS); // milliseconds

    vector<string> ascii_frames = getASCIIFramesFromVideo(&video_file); // Save all frames from video as vector with ascii strings

    // Unblock console
    cout.rdbuf(cout_sbuf); // restore the original stream buffer

    playVideoAsASCIIFrames(&ascii_frames, &frame_duration); // Play video as ascii frames in console :)

    return 0;
}
