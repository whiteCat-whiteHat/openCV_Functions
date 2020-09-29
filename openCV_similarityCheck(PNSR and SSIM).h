double getPSNR(const Mat& I1, const Mat& I2)
2 {
3 Mat s1;
4 absdiff(I1, I2, s1); // |I1 - I2|
5 s1.convertTo(s1, CV_32F); // cannot make a square on 8 bits
6 s1 = s1.mul(s1); // |I1 - I2|^2
7
8 Scalar s = sum(s1); // sum elements per channel
9
10 double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels
11
12 if( sse <= 1e-10) // for small values return zero
13 return 0;
14 else
15 {
16 double mse =sse /(double)(I1.channels() * I1.total());
17 double psnr = 10.0*log10((255*255)/mse);
18 return psnr;
19 }
20 }
21
22
23
24 double getPSNR_GPU_optimized(const Mat& I1, const Mat& I2, BufferPSNR& b)
25 {
26 b.gI1.upload(I1);
27 b.gI2.upload(I2);
28
29 b.gI1.convertTo(b.t1, CV_32F);
b.gI2.convertTo(b.t2, CV_32F);
31
32 gpu::absdiff(b.t1.reshape(1), b.t2.reshape(1), b.gs);
33 gpu::multiply(b.gs, b.gs, b.gs);
34
35 double sse = gpu::sum(b.gs, b.buf)[0];
36
37 if( sse <= 1e-10) // for small values return zero
38 return 0;
39 else
40 {
41 double mse = sse /(double)(I1.channels() * I1.total());
42 double psnr = 10.0*log10((255*255)/mse);
43 return psnr;
44 }
45 }
46
47 struct BufferPSNR // Optimized GPU versions
48 { // Data allocations are very expensive on GPU. Use a buffer to solve: allocate once reuse later.
49 gpu::GpuMat gI1, gI2, gs, t1,t2;
50
51 gpu::GpuMat buf;
52 };
53
54 double getPSNR_GPU(const Mat& I1, const Mat& I2)
55 {
56 gpu::GpuMat gI1, gI2, gs, t1,t2;
57
58 gI1.upload(I1);
59 gI2.upload(I2);
60
61 gI1.convertTo(t1, CV_32F);
62 gI2.convertTo(t2, CV_32F);
63
64 gpu::absdiff(t1.reshape(1), t2.reshape(1), gs);
65 gpu::multiply(gs, gs, gs);
66
67 Scalar s = gpu::sum(gs);
68 double sse = s.val[0] + s.val[1] + s.val[2];
69
70 if( sse <= 1e-10) // for small values return zero
71 return 0;
72 else
73 {
74 double mse =sse /(double)(gI1.channels() * I1.total());
75 double psnr = 10.0*log10((255*255)/mse);
76 return psnr;
77 }
1 }
2
3 Scalar getMSSIM( const Mat& i1, const Mat& i2)
4 {
5 const double C1 = 6.5025, C2 = 58.5225;
78 }
/***************************** INITS **********************************/
7 int d = CV_32F;
8
9 Mat I1, I2;
10 i1.convertTo(I1, d); // cannot calculate on one byte large values
11 i2.convertTo(I2, d);
12
13 Mat I2_2 = I2.mul(I2); // I2^2
14 Mat I1_2 = I1.mul(I1); // I1^2
15 Mat I1_I2 = I1.mul(I2); // I1 * I2
16
17 /*************************** END INITS **********************************/
18
19 Mat mu1, mu2; // PRELIMINARY COMPUTING
20 GaussianBlur(I1, mu1, Size(11, 11), 1.5);
21 GaussianBlur(I2, mu2, Size(11, 11), 1.5);
22
23 Mat mu1_2 = mu1.mul(mu1);
24 Mat mu2_2 = mu2.mul(mu2);
25 Mat mu1_mu2 = mu1.mul(mu2);
26
27 Mat sigma1_2, sigma2_2, sigma12;
28
29 GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
30 sigma1_2 -= mu1_2;
31
32 GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
33 sigma2_2 -= mu2_2;
34
35 GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
36 sigma12 -= mu1_mu2;
37
38 ///////////////////////////////// FORMULA ////////////////////////////////
39 Mat t1, t2, t3;
40
41 t1 = 2 * mu1_mu2 + C1;
42 t2 = 2 * sigma12 + C2;
43 t3 = t1.mul(t2); // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
44
45 t1 = mu1_2 + mu2_2 + C1;
46 t2 = sigma1_2 + sigma2_2 + C2;
47 t1 = t1.mul(t2); // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
48
49 Mat ssim_map;
50 divide(t3, t1, ssim_map); // ssim_map = t3./t1;
51
52 Scalar mssim = mean( ssim_map ); // mssim = average of ssim map
53 return mssim;
54 }
55
56 Scalar getMSSIM_GPU( const Mat& i1, const Mat& i2)
57 {
58 const float C1 = 6.5025f, C2 = 58.5225f;
59 /***************************** INITS **********************************/
60 gpu::GpuMat gI1, gI2, gs1, tmp1,tmp2;
61
62 gI1.upload(i1);
63 gI2.upload(i2);
gI1.convertTo(tmp1, CV_MAKE_TYPE(CV_32F, gI1.channels()));
66 gI2.convertTo(tmp2, CV_MAKE_TYPE(CV_32F, gI2.channels()));
67
68 vector<gpu::GpuMat> vI1, vI2;
69 gpu::split(tmp1, vI1);
70 gpu::split(tmp2, vI2);
71 Scalar mssim;
72
73 for( int i = 0; i < gI1.channels(); ++i )
74 {
75 gpu::GpuMat I2_2, I1_2, I1_I2;
76
77 gpu::multiply(vI2[i], vI2[i], I2_2); // I2^2
78 gpu::multiply(vI1[i], vI1[i], I1_2); // I1^2
79 gpu::multiply(vI1[i], vI2[i], I1_I2); // I1 * I2
80
81 /*************************** END INITS **********************************/
82 gpu::GpuMat mu1, mu2; // PRELIMINARY COMPUTING
83 gpu::GaussianBlur(vI1[i], mu1, Size(11, 11), 1.5);
84 gpu::GaussianBlur(vI2[i], mu2, Size(11, 11), 1.5);
85
86 gpu::GpuMat mu1_2, mu2_2, mu1_mu2;
87 gpu::multiply(mu1, mu1, mu1_2);
88 gpu::multiply(mu2, mu2, mu2_2);
89 gpu::multiply(mu1, mu2, mu1_mu2);
90
91 gpu::GpuMat sigma1_2, sigma2_2, sigma12;
92
93 gpu::GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
94 gpu::subtract(sigma1_2, mu1_2, sigma1_2); // sigma1_2 -= mu1_2;
95
96 gpu::GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
97 gpu::subtract(sigma2_2, mu2_2, sigma2_2); // sigma2_2 -= mu2_2;
98
99 gpu::GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
100 gpu::subtract(sigma12, mu1_mu2, sigma12); // sigma12 -= mu1_mu2;
101
102 ///////////////////////////////// FORMULA ////////////////////////////////
103 gpu::GpuMat t1, t2, t3;
104
105 mu1_mu2.convertTo(t1, -1, 2, C1); // t1 = 2 * mu1_mu2 + C1;
106 sigma12.convertTo(t2, -1, 2, C2); // t2 = 2 * sigma12 + C2;
107 gpu::multiply(t1, t2, t3); // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
108
109 gpu::addWeighted(mu1_2, 1.0, mu2_2, 1.0, C1, t1); // t1 = mu1_2 + mu2_2 + C1;
110 gpu::addWeighted(sigma1_2, 1.0, sigma2_2, 1.0, C2, t2); // t2 = sigma1_2 + sigma2_2 + C2;
111 gpu::multiply(t1, t2, t1); // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
112
113 gpu::GpuMat ssim_map;
114 gpu::divide(t3, t1, ssim_map); // ssim_map = t3./t1;
115
116 Scalar s = gpu::sum(ssim_map);
117 mssim.val[i] = s.val[0] / (ssim_map.rows * ssim_map.cols);
118
119 }
120 return mssim;
121 }
struct BufferMSSIM // Optimized GPU versions
123 { // Data allocations are very expensive on GPU. Use a buffer to solve: allocate once reuse later.
124 gpu::GpuMat gI1, gI2, gs, t1,t2;
125
126 gpu::GpuMat I1_2, I2_2, I1_I2;
127 vector<gpu::GpuMat> vI1, vI2;
128
129 gpu::GpuMat mu1, mu2;
130 gpu::GpuMat mu1_2, mu2_2, mu1_mu2;
131
132 gpu::GpuMat sigma1_2, sigma2_2, sigma12;
133 gpu::GpuMat t3;
134
135 gpu::GpuMat ssim_map;
136
137 gpu::GpuMat buf;
138 };
139 Scalar getMSSIM_GPU_optimized( const Mat& i1, const Mat& i2, BufferMSSIM& b)
140 {
141 const float C1 = 6.5025f, C2 = 58.5225f;
142 /***************************** INITS **********************************/
143
144 b.gI1.upload(i1);
145 b.gI2.upload(i2);
146
147 gpu::Stream stream;
148
149 stream.enqueueConvert(b.gI1, b.t1, CV_32F);
150 stream.enqueueConvert(b.gI2, b.t2, CV_32F);
151
152 gpu::split(b.t1, b.vI1, stream);
153 gpu::split(b.t2, b.vI2, stream);
154 Scalar mssim;
155
156 gpu::GpuMat buf;
157
158 for( int i = 0; i < b.gI1.channels(); ++i )
159 {
160 gpu::multiply(b.vI2[i], b.vI2[i], b.I2_2, stream); // I2^2
161 gpu::multiply(b.vI1[i], b.vI1[i], b.I1_2, stream); // I1^2
162 gpu::multiply(b.vI1[i], b.vI2[i], b.I1_I2, stream); // I1 * I2
163
164 gpu::GaussianBlur(b.vI1[i], b.mu1, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
165 gpu::GaussianBlur(b.vI2[i], b.mu2, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
166
167 gpu::multiply(b.mu1, b.mu1, b.mu1_2, stream);
168 gpu::multiply(b.mu2, b.mu2, b.mu2_2, stream);
169 gpu::multiply(b.mu1, b.mu2, b.mu1_mu2, stream);
170
171 gpu::GaussianBlur(b.I1_2, b.sigma1_2, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
172 gpu::subtract(b.sigma1_2, b.mu1_2, b.sigma1_2, gpu::GpuMat(), -1, stream);
173 //b.sigma1_2 -= b.mu1_2; - This would result in an extra data transfer operation
174
175 gpu::GaussianBlur(b.I2_2, b.sigma2_2, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
176 gpu::subtract(b.sigma2_2, b.mu2_2, b.sigma2_2, gpu::GpuMat(), -1, stream);
177 //b.sigma2_2 -= b.mu2_2;
178
179 gpu::GaussianBlur(b.I1_I2, b.sigma12, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
gpu::subtract(b.sigma12, b.mu1_mu2, b.sigma12, gpu::GpuMat(), -1, stream);
181 //b.sigma12 -= b.mu1_mu2;
182
183 //here too it would be an extra data transfer due to call of operator*(Scalar, Mat)
184 gpu::multiply(b.mu1_mu2, 2, b.t1, 1, -1, stream); //b.t1 = 2 * b.mu1_mu2 + C1;
185 gpu::add(b.t1, C1, b.t1, gpu::GpuMat(), -1, stream);
186 gpu::multiply(b.sigma12, 2, b.t2, 1, -1, stream); //b.t2 = 2 * b.sigma12 + C2;
187 gpu::add(b.t2, C2, b.t2, gpu::GpuMat(), -12, stream);
188
189 gpu::multiply(b.t1, b.t2, b.t3, 1, -1, stream); // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
190
191 gpu::add(b.mu1_2, b.mu2_2, b.t1, gpu::GpuMat(), -1, stream);
192 gpu::add(b.t1, C1, b.t1, gpu::GpuMat(), -1, stream);
193
194 gpu::add(b.sigma1_2, b.sigma2_2, b.t2, gpu::GpuMat(), -1, stream);
195 gpu::add(b.t2, C2, b.t2, gpu::GpuMat(), -1, stream);
196
197
198 gpu::multiply(b.t1, b.t2, b.t1, 1, -1, stream); // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
199 gpu::divide(b.t3, b.t1, b.ssim_map, 1, -1, stream); // ssim_map = t3./t1;
200
201 stream.waitForCompletion();
202
203 Scalar s = gpu::sum(b.ssim_map, b.buf);
204 mssim.val[i] = s.val[0] / (b.ssim_map.rows * b.ssim_map.cols);
205
206 }
207 return mssim;
208 }