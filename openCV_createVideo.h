#include <iostream> // for standard I/O
2 #include <string> // for strings
3
4 #include <opencv2/core/core.hpp> // Basic OpenCV structures (cv::Mat)
5 #include <opencv2/highgui/highgui.hpp> // Video write
6
7 using namespace std;
8 using namespace cv;
9
10 static void help()
11 {
12 cout
13 << "------------------------------------------------------------------------------" << endl
14 << "This program shows how to write video files." << endl
15 << "You can extract the R or G or B color channel of the input video." << endl
16 << "Usage:" << endl
17 << "./video-write inputvideoName [ R | G | B] [Y | N]" << endl
18 << "------------------------------------------------------------------------------" << endl
19 << endl;
20 }
21
22 int main(int argc, char *argv[])
23 {
help();
25
26 if (argc != 4)
27 {
28 cout << "Not enough parameters" << endl;
29 return -1;
30 }
31
32 const string source = argv[1]; // the source file name
33 const bool askOutputType = argv[3][0] =='Y'; // If false it will use the inputs codec type
34
35 VideoCapture inputVideo(source); // Open input
36 if (!inputVideo.isOpened())
37 {
38 cout << "Could not open the input video: " << source << endl;
39 return -1;
40 }
41
42 string::size_type pAt = source.find_last_of('.'); // Find extension point
43 const string NAME = source.substr(0, pAt) + argv[2][0] + ".avi"; // Form the new name with container
44 int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC)); // Get Codec Type- Int form
45
46 // Transform from int to char via Bitwise operators
47 char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
48
49 Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), // Acquire input size
50 (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
51
52 VideoWriter outputVideo; // Open the output
53 if (askOutputType)
54 outputVideo.open(NAME, ex=-1, inputVideo.get(CV_CAP_PROP_FPS), S, true);
55 else
56 outputVideo.open(NAME, ex, inputVideo.get(CV_CAP_PROP_FPS), S, true);
57
58 if (!outputVideo.isOpened())
59 {
60 cout << "Could not open the output video for write: " << source << endl;
61 return -1;
62 }
63
64 cout << "Input frame resolution: Width=" << S.width << " Height=" << S.height
65 << " of nr#: " << inputVideo.get(CV_CAP_PROP_FRAME_COUNT) << endl;
66 cout << "Input codec type: " << EXT << endl;
67
68 int channel = 2; // Select the channel to save
69 switch(argv[2][0])
70 {
71 case 'R' : channel = 2; break;
72 case 'G' : channel = 1; break;
73 case 'B' : channel = 0; break;
74 }
75 Mat src, res;
76 vector<Mat> spl;
77
78 for(;;) //Show the image captured in the window and repeat
79 {
80 inputVideo >> src; // read
81 if (src.empty()) break; // check if at end
split(src, spl); // process - extract only the correct channel
84 for (int i =0; i < 3; ++i)
85 if (i != channel)
86 spl[i] = Mat::zeros(S, spl[0].type());
87 merge(spl, res);
88
89 //outputVideo.write(res); //save or
90 outputVideo << res;
91 }
92
93 cout << "Finished writing" << endl;
94 return 0;
95 }
