//  optimal hyperplane computed?
// s, given labeled training data (supervised learning), the algorithm outputs an optimal hyperplane which
// categorizes new examples
#include <opencv2/core/core.hpp>
2 #include <opencv2/highgui/highgui.hpp>
3 #include <opencv2/ml/ml.hpp>
4
5 using namespace cv;
6
7 int main()
8 {
9 // Data for visual representation
10 int width = 512, height = 512;
11 Mat image = Mat::zeros(height, width, CV_8UC3);
12
13 // Set up training data
14 float labels[4] = {1.0, -1.0, -1.0, -1.0};
Mat labelsMat(4, 1, CV_32FC1, labels);
16
17 float trainingData[4][2] = { {501, 10}, {255, 10}, {501, 255}, {10, 501} };
18 Mat trainingDataMat(4, 2, CV_32FC1, trainingData);
19
20 // Set up SVM's parameters
21 CvSVMParams params;
22 params.svm_type = CvSVM::C_SVC;
23 params.kernel_type = CvSVM::LINEAR;
24 params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);
25
26 // Train the SVM
27 CvSVM SVM;
28 SVM.train(trainingDataMat, labelsMat, Mat(), Mat(), params);
29
30 Vec3b green(0,255,0), blue (255,0,0);
31 // Show the decision regions given by the SVM
32 for (int i = 0; i < image.rows; ++i)
33 for (int j = 0; j < image.cols; ++j)
34 {
35 Mat sampleMat = (Mat_<float>(1,2) << j,i);
36 float response = SVM.predict(sampleMat);
37
38 if (response == 1)
39 image.at<Vec3b>(i,j) = green;
40 else if (response == -1)
41 image.at<Vec3b>(i,j) = blue;
42 }
43
44 // Show the training data
45 int thickness = -1;
46 int lineType = 8;
47 circle( image, Point(501, 10), 5, Scalar( 0, 0, 0), thickness, lineType);
48 circle( image, Point(255, 10), 5, Scalar(255, 255, 255), thickness, lineType);
49 circle( image, Point(501, 255), 5, Scalar(255, 255, 255), thickness, lineType);
50 circle( image, Point( 10, 501), 5, Scalar(255, 255, 255), thickness, lineType);
51
52 // Show support vectors
53 thickness = 2;
54 lineType = 8;
55 int c = SVM.get_support_vector_count();
56
57 for (int i = 0; i < c; ++i)
58 {
59 const float* v = SVM.get_support_vector(i);
60 circle( image, Point( (int) v[0], (int) v[1]), 6, Scalar(128, 128, 128), thickness, lineType);
61 }
62
63 imwrite("result.png", image); // save the image
64
65 imshow("SVM Simple Example", image); // show it to the user
66 waitKey(0);
67
68 }