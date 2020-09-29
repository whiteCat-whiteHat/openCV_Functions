#include <iostream> // for standard I/O
2 #include <string> // for strings
3 #include <iomanip> // for controlling float print precision
4 #include <sstream> // string to number conversion
5
6 #include <opencv2/core/core.hpp> // Basic OpenCV structures (cv::Mat, Scalar)
7 #include <opencv2/imgproc/imgproc.hpp> // Gaussian Blur
8 #include <opencv2/highgui/highgui.hpp> // OpenCV window I/O
9
10 using namespace std;
using namespace cv;
12
13 double getPSNR ( const Mat& I1, const Mat& I2);
14 Scalar getMSSIM( const Mat& I1, const Mat& I2);
15
16 int main(int argc, char *argv[])
17 help();
18
19 if (argc != 5)
20 {
21 cout << "Not enough parameters" << endl;
22 return -1;
23 }
24
25 stringstream conv;
26
27 const string sourceReference = argv[1], sourceCompareWith = argv[2];
28 int psnrTriggerValue, delay;
29 conv << argv[3] << endl << argv[4]; // put in the strings
30 conv >> psnrTriggerValue >> delay; // take out the numbers
31
32 char c;
33 int frameNum = -1; // Frame counter
34
35 VideoCapture captRefrnc(sourceReference), captUndTst(sourceCompareWith);
36
37 if (!captRefrnc.isOpened())
38 {
39 cout << "Could not open reference " << sourceReference << endl;
40 return -1;
41 }
42
43 if (!captUndTst.isOpened())
44 {
45 cout << "Could not open case test " << sourceCompareWith << endl;
46 return -1;
47 }
48
49 Size refS = Size((int) captRefrnc.get(CV_CAP_PROP_FRAME_WIDTH),
50 (int) captRefrnc.get(CV_CAP_PROP_FRAME_HEIGHT)),
51 uTSi = Size((int) captUndTst.get(CV_CAP_PROP_FRAME_WIDTH),
52 (int) captUndTst.get(CV_CAP_PROP_FRAME_HEIGHT));
53
54 if (refS != uTSi)
55 {
56 cout << "Inputs have different size!!! Closing." << endl;
57 return -1;
58 }
59
60 const char* WIN_UT = "Under Test";
61 const char* WIN_RF = "Reference";
62
63 // Windows
64 namedWindow(WIN_RF, CV_WINDOW_AUTOSIZE);
65 namedWindow(WIN_UT, CV_WINDOW_AUTOSIZE);
66 cvMoveWindow(WIN_RF, 400 , 0); //750, 2 (bernat =0)
67 cvMoveWindow(WIN_UT, refS.width, 0); //1500, 2
cout << "Reference frame resolution: Width=" << refS.width << " Height=" << refS.height
70 << " of nr#: " << captRefrnc.get(CV_CAP_PROP_FRAME_COUNT) << endl;
71
72 cout << "PSNR trigger value " << setiosflags(ios::fixed) << setprecision(3)
73 << psnrTriggerValue << endl;
74
75 Mat frameReference, frameUnderTest;
76 double psnrV;
77 Scalar mssimV;
78
79 for(;;) //Show the image captured in the window and repeat
80 {
81 captRefrnc >> frameReference;
82 captUndTst >> frameUnderTest;
83
84 if (frameReference.empty() || frameUnderTest.empty())
85 {
86 cout << " < < < Game over! > > > ";
87 break;
88 }
89
90 ++frameNum;
91 cout << "Frame: " << frameNum << "# ";
92
93 ///////////////////////////////// PSNR ////////////////////////////////////////////////////
94 psnrV = getPSNR(frameReference,frameUnderTest);
95 cout << setiosflags(ios::fixed) << setprecision(3) << psnrV << "dB";
96
97 //////////////////////////////////// MSSIM /////////////////////////////////////////////////
98 if (psnrV < psnrTriggerValue && psnrV)
99 {
100 mssimV = getMSSIM(frameReference, frameUnderTest);
101
102 cout << " MSSIM: "
103 << " R " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100 << "%"
104 << " G " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100 << "%"
105 << " B " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100 << "%";
106 }
107
108 cout << endl;
109
110 ////////////////////////////////// Show Image /////////////////////////////////////////////
111 imshow(WIN_RF, frameReference);
112 imshow(WIN_UT, frameUnderTest);
113
114 c = (char)cvWaitKey(delay);
115 if (c == 27) break;
116 }
117
118 return 0;
119 }
120
121 double getPSNR(const Mat& I1, const Mat& I2)
122 {
123 Mat s1;
124 absdiff(I1, I2, s1); // |I1 - I2|
125 s1.convertTo(s1, CV_32F); // cannot make a square on 8 bits
126 s1 = s1.mul(s1); // |I1 - I2|^2
Scalar s = sum(s1); // sum elements per channel
129
130 double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels
131
132 if( sse <= 1e-10) // for small values return zero
133 return 0;
134 else
135 {
136 double mse = sse / (double)(I1.channels() * I1.total());
137 double psnr = 10.0 * log10((255 * 255) / mse);
138 return psnr;
139 }
140 }
141
142 Scalar getMSSIM( const Mat& i1, const Mat& i2)
143 {
144 const double C1 = 6.5025, C2 = 58.5225;
145 /***************************** INITS **********************************/
146 int d = CV_32F;
147
148 Mat I1, I2;
149 i1.convertTo(I1, d); // cannot calculate on one byte large values
150 i2.convertTo(I2, d);
151
152 Mat I2_2 = I2.mul(I2); // I2^2
153 Mat I1_2 = I1.mul(I1); // I1^2
154 Mat I1_I2 = I1.mul(I2); // I1 * I2
155
156 /*************************** END INITS **********************************/
157
158 Mat mu1, mu2; // PRELIMINARY COMPUTING
159 GaussianBlur(I1, mu1, Size(11, 11), 1.5);
160 GaussianBlur(I2, mu2, Size(11, 11), 1.5);
161
162 Mat mu1_2 = mu1.mul(mu1);
163 Mat mu2_2 = mu2.mul(mu2);
164 Mat mu1_mu2 = mu1.mul(mu2);
165
166 Mat sigma1_2, sigma2_2, sigma12;
167
168 GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
169 sigma1_2 -= mu1_2;
170
171 GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
172 sigma2_2 -= mu2_2;
173
174 GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
175 sigma12 -= mu1_mu2;
176
177 ///////////////////////////////// FORMULA ////////////////////////////////
178 Mat t1, t2, t3;
179
180 t1 = 2 * mu1_mu2 + C1;
181 t2 = 2 * sigma12 + C2;
182 t3 = t1.mul(t2); // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
183
184 t1 = mu1_2 + mu2_2 + C1;
t2 = sigma1_2 + sigma2_2 + C2;
186 t1 = t1.mul(t2); // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
187
188 Mat ssim_map;
189 divide(t3, t1, ssim_map); // ssim_map = t3./t1;
190
191 Scalar mssim = mean(ssim_map); // mssim = average of ssim map