#include <opencv2/core/core.hpp>
2 #include <iostream>
3 #include <string>
4
5 using namespace cv;
6 using namespace std;
7
8 class MyData
9 {
10 public:
11 MyData() : A(0), X(0), id()
12 {}
13 explicit MyData(int) : A(97), X(CV_PI), id("mydata1234") // explicit to avoid implicit conversion
14 {}
15 void write(FileStorage& fs) const //Write serialization for this class
16 {
17 fs << "{" << "A" << A << "X" << X << "id" << id << "}";
18 }
19 void read(const FileNode& node) //Read serialization for this class
20 {
21 A = (int)node["A"];
22 X = (double)node["X"];
23 id = (string)node["id"];
24 }
25 public: // Data Members
26 int A;
27 double X;
28 string id;
29 };
30
31 //These write and read functions must be defined for the serialization in FileStorage to work
32 static void write(FileStorage& fs, const std::string&, const MyData& x)
33 {
34 x.write(fs);
35 }
36 static void read(const FileNode& node, MyData& x, const MyData& default_value = MyData()){
37 if(node.empty())
38 x = default_value;
39 else
40 x.read(node);
41 }
42
43 // This function will print our custom class to the console
44 static ostream& operator<<(ostream& out, const MyData& m)
45 {
46 out << "{ id = " << m.id << ", ";
47 out << "X = " << m.X << ", ";
48 out << "A = " << m.A << "}";
49 return out;
50 }
51
52 int main(int ac, char** av)
{
54 if (ac != 2)
55 {
56 help(av);
57 return 1;
58 }
59
60 string filename = av[1];
61 { //write
62 Mat R = Mat_<uchar>::eye(3, 3),
63 T = Mat_<double>::zeros(3, 1);
64 MyData m(1);
65
66 FileStorage fs(filename, FileStorage::WRITE);
67
68 fs << "iterationNr" << 100;
69 fs << "strings" << "["; // text - string sequence
70 fs << "image1.jpg" << "Awesomeness" << "baboon.jpg";
71 fs << "]"; // close sequence
72
73 fs << "Mapping"; // text - mapping
74 fs << "{" << "One" << 1;
75 fs << "Two" << 2 << "}";
76
77 fs << "R" << R; // cv::Mat
78 fs << "T" << T;
79
80 fs << "MyData" << m; // your own data structures
81
82 fs.release(); // explicit close
83 cout << "Write Done." << endl;
84 }
85
86 {//read
87 cout << endl << "Reading: " << endl;
88 FileStorage fs;
89 fs.open(filename, FileStorage::READ);
90
91 int itNr;
92 //fs["iterationNr"] >> itNr;
93 itNr = (int) fs["iterationNr"];
94 cout << itNr;
95 if (!fs.isOpened())
96 {
97 cerr << "Failed to open " << filename << endl;
98 help(av);
99 return 1;
100 }
101
102 FileNode n = fs["strings"]; // Read string sequence - Get node
103 if (n.type() != FileNode::SEQ)
104 {
105 cerr << "strings is not a sequence! FAIL" << endl;
106 return 1;
107 }
108
109 FileNodeIterator it = n.begin(), it_end = n.end(); // Go through the node
110 for (; it != it_end; ++it)
cout << (string)*it << endl;
112
113
114 n = fs["Mapping"]; // Read mappings from a sequence
115 cout << "Two " << (int)(n["Two"]) << "; ";
116 cout << "One " << (int)(n["One"]) << endl << endl;
117
118
119 MyData m;
120 Mat R, T;
121
122 fs["R"] >> R; // Read cv::Mat
123 fs["T"] >> T;
124 fs["MyData"] >> m; // Read your own structure_
125
126 cout << endl
127 << "R = " << R << endl;
128 cout << "T = " << T << endl << endl;
129 cout << "MyData = " << endl << m << endl << endl;
130
131 //Show default behavior for non existing nodes
132 cout << "Attempt to read NonExisting (should initialize the data structure with its default).";
133 fs["NonExisting"] >> m;
134 cout << endl << "NonExisting = " << endl << m << endl;
135 }
136
137 cout << endl
138 << "Tip: Open up " << filename << " with a text editor to see the serialized data." << endl;
139
140 return 0;
141 }