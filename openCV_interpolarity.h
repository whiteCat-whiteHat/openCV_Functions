#include <stdio.h>
2 #include <iostream>
#include <opencv2/core/core.hpp>
5 #include <opencv2/imgproc/imgproc.hpp>
6 #include <opencv2/highgui/highgui.hpp>
7
8 using namespace cv; // The new C++ interface API is inside this namespace. Import it.
9 using namespace std;
10 #define DEMO_MIXED_API_USE
11
12 int main( int argc, char** argv )
13 {
14 const char* imagename = argc > 1 ? argv[1] : "lena.jpg";
15
16 #ifdef DEMO_MIXED_API_USE
17 Ptr<IplImage> IplI = cvLoadImage(imagename); // Ptr<T> is safe ref-counting pointer class
18 if(IplI.empty())
19 {
20 cerr << "Can not load image " << imagename << endl;
21 return -1;
22 }
23 Mat I(IplI); // Convert to the new style container. Only header created. Image not copied.
24 #else
25 Mat I = imread(imagename); // the newer cvLoadImage alternative, MATLAB-style function
26 if( I.empty() ) // same as if( !I.data )
27 {
28 cerr << "Can not load image " << imagename << endl;
29 return -1;
30 }
31 #endif
// convert image to YUV color space. The output image will be created automatically.
2 Mat I_YUV;
3 cvtColor(I, I_YUV, COLOR_BGR2YCrCb);
4
5 vector<Mat> planes; // Use the STL's vector structure to store multiple Mat objects
6 split(I_YUV, planes); // split the image into separate color planes (Y U V)
// Method 1. process Y plane using an iterator
2 MatIterator_<uchar> it = planes[0].begin<uchar>(), it_end = planes[0].end<uchar>();
3 for(; it != it_end; ++it)
4 {
5 double v = *it * 1.7 + rand()%21 - 10;
6 *it = saturate_cast<uchar>(v*v/255);
7 }
8
9 for( int y = 0; y < I_YUV.rows; y++ )
10 {
11 // Method 2. process the first chroma plane using pre-stored row pointer.
12 uchar* Uptr = planes[1].ptr<uchar>(y);
for( int x = 0; x < I_YUV.cols; x++ )
14 {
15 Uptr[x] = saturate_cast<uchar>((Uptr[x]-128)/2 + 128);
16
17 // Method 3. process the second chroma plane using individual element access
18 uchar& Vxy = planes[2].at<uchar>(y, x);
19 Vxy = saturate_cast<uchar>((Vxy-128)/2 + 128);
20 }
21 }
Mat noisyI(I.size(), CV_8U); // Create a matrix of the specified size and type
2
3 // Fills the matrix with normally distributed random values (around number with deviation off).
4 // There is also randu() for uniformly distributed random number generation
5 randn(noisyI, Scalar::all(128), Scalar::all(20));
6
7 // blur the noisyI a bit, kernel size is 3x3 and both sigma's are set to 0.5
8 GaussianBlur(noisyI, noisyI, Size(3, 3), 0.5, 0.5);
9
10 const double brightness_gain = 0;
11 const double contrast_gain = 1.7;
12
13 #ifdef DEMO_MIXED_API_USE
14 // To pass the new matrices to the functions that only work with IplImage or CvMat do:
15 // step 1) Convert the headers (tip: data will not be copied).
16 // step 2) call the function (tip: to pass a pointer do not forget unary "&" to form pointers)
17
18 IplImage cv_planes_0 = planes[0], cv_noise = noisyI;
19 cvAddWeighted(&cv_planes_0, contrast_gain, &cv_noise, 1, -128 + brightness_gain, &cv_planes_0);
20 #else
21 addWeighted(planes[0], contrast_gain, noisyI, 1, -128 + brightness_gain, planes[0]);
22 #endif
23
24 const double color_scale = 0.5;
25 // Mat::convertTo() replaces cvConvertScale.
26 // One must explicitly specify the output matrix type (we keep it intact - planes[1].type())
27 planes[1].convertTo(planes[1], planes[1].type(), color_scale, 128*(1-color_scale));
28
29 // alternative form of cv::convertScale if we know the datatype at compile time ("uchar" here).
30 // This expression will not create any temporary arrays ( so should be almost as fast as above)
31 planes[2] = Mat_<uchar>(planes[2]*color_scale + 128*(1-color_scale));
32
33 // Mat::mul replaces cvMul(). Again, no temporary arrays are created in case of simple expressions.
34 planes[0] = planes[0].mul(planes[0], 1./255);
merge(planes, I_YUV); // now merge the results back
2 cvtColor(I_YUV, I, CV_YCrCb2BGR); // and produce the output RGB image
namedWindow("image with grain", WINDOW_AUTOSIZE); // use this to create images
6
7 #ifdef DEMO_MIXED_API_USE
8 // this is to demonstrate that I and IplI really share the data - the result of the above
9 // processing is stored in I and thus in IplI too.
10 cvShowImage("image with grain", IplI);
11 #else
12 imshow("image with grain", I); // the new MATLAB style function show