#include <iostream>
2 #include <opencv2/core/core.hpp>
3 #include <opencv2/highgui/highgui.hpp>
4 #include <opencv2/ml/ml.hpp>
5
6 #define NTRAINING_SAMPLES 100 // Number of training samples per class
7 #define FRAC_LINEAR_SEP 0.9f // Fraction of samples which compose the linear separable part
8
9 using namespace cv;
using namespace std;
11
12 int main()
13 {
14 // Data for visual representation
15 const int WIDTH = 512, HEIGHT = 512;
16 Mat I = Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
17
18 //--------------------- 1. Set up training data randomly ---------------------------------------
19 Mat trainData(2*NTRAINING_SAMPLES, 2, CV_32FC1);
20 Mat labels (2*NTRAINING_SAMPLES, 1, CV_32FC1);
21
22 RNG rng(100); // Random value generation class
23
24 // Set up the linearly separable part of the training data
25 int nLinearSamples = (int) (FRAC_LINEAR_SEP * NTRAINING_SAMPLES);
26
27 // Generate random points for the class 1
28 Mat trainClass = trainData.rowRange(0, nLinearSamples);
29 // The x coordinate of the points is in [0, 0.4)
30 Mat c = trainClass.colRange(0, 1);
31 rng.fill(c, RNG::UNIFORM, Scalar(1), Scalar(0.4 * WIDTH));
32 // The y coordinate of the points is in [0, 1)
33 c = trainClass.colRange(1,2);
34 rng.fill(c, RNG::UNIFORM, Scalar(1), Scalar(HEIGHT));
35
36 // Generate random points for the class 2
37 trainClass = trainData.rowRange(2*NTRAINING_SAMPLES-nLinearSamples, 2*NTRAINING_SAMPLES);
38 // The x coordinate of the points is in [0.6, 1]
39 c = trainClass.colRange(0 , 1);
40 rng.fill(c, RNG::UNIFORM, Scalar(0.6*WIDTH), Scalar(WIDTH));
41 // The y coordinate of the points is in [0, 1)
42 c = trainClass.colRange(1,2);
43 rng.fill(c, RNG::UNIFORM, Scalar(1), Scalar(HEIGHT));
44
45 //------------------ Set up the non-linearly separable part of the training data ---------------
46
47 // Generate random points for the classes 1 and 2
48 trainClass = trainData.rowRange( nLinearSamples, 2*NTRAINING_SAMPLES-nLinearSamples);
49 // The x coordinate of the points is in [0.4, 0.6)
50 c = trainClass.colRange(0,1);
51 rng.fill(c, RNG::UNIFORM, Scalar(0.4*WIDTH), Scalar(0.6*WIDTH));
52 // The y coordinate of the points is in [0, 1)
53 c = trainClass.colRange(1,2);
54 rng.fill(c, RNG::UNIFORM, Scalar(1), Scalar(HEIGHT));
55
56 //------------------------- Set up the labels for the classes ---------------------------------
57 labels.rowRange( 0, NTRAINING_SAMPLES).setTo(1); // Class 1
58 labels.rowRange(NTRAINING_SAMPLES, 2*NTRAINING_SAMPLES).setTo(2); // Class 2
59
60 //------------------------ 2. Set up the support vector machines parameters --------------------
61 CvSVMParams params;
62 params.svm_type = SVM::C_SVC;
63 params.C = 0.1;
64 params.kernel_type = SVM::LINEAR;
65 params.term_crit = TermCriteria(CV_TERMCRIT_ITER, (int)1e7, 1e-6);
66
67 //------------------------ 3. Train the svm ----------------------------------------------------
cout << "Starting training process" << endl;
69 CvSVM svm;
70 svm.train(trainData, labels, Mat(), Mat(), params);
71 cout << "Finished training process" << endl;
72
73 //------------------------ 4. Show the decision regions ----------------------------------------
74 Vec3b green(0,100,0), blue (100,0,0);
75 for (int i = 0; i < I.rows; ++i)
76 for (int j = 0; j < I.cols; ++j)
77 {
78 Mat sampleMat = (Mat_<float>(1,2) << i, j);
79 float response = svm.predict(sampleMat);
80
81 if (response == 1) I.at<Vec3b>(j, i) = green;
82 else if (response == 2) I.at<Vec3b>(j, i) = blue;
83 }
84
85 //----------------------- 5. Show the training data --------------------------------------------
86 int thick = -1;
87 int lineType = 8;
88 float px, py;
89 // Class 1
90 for (int i = 0; i < NTRAINING_SAMPLES; ++i)
91 {
92 px = trainData.at<float>(i,0);
93 py = trainData.at<float>(i,1);
94 circle(I, Point( (int) px, (int) py ), 3, Scalar(0, 255, 0), thick, lineType);
95 }
96 // Class 2
97 for (int i = NTRAINING_SAMPLES; i <2*NTRAINING_SAMPLES; ++i)
98 {
99 px = trainData.at<float>(i,0);
100 py = trainData.at<float>(i,1);
101 circle(I, Point( (int) px, (int) py ), 3, Scalar(255, 0, 0), thick, lineType);
102 }
103
104 //------------------------- 6. Show support vectors --------------------------------------------
105 thick = 2;
106 lineType = 8;
107 int x = svm.get_support_vector_count();
108
109 for (int i = 0; i < x; ++i)
110 {
111 const float* v = svm.get_support_vector(i);
112 circle( I, Point( (int) v[0], (int) v[1]), 6, Scalar(128, 128, 128), thick, lineType);
113 }
114
115 imwrite("result.png", I); // save the Image
116 imshow("SVM for Non-Linear Training Data", I); // show it to the user
117 waitKey(0);
118 }