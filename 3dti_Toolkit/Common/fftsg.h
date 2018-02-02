/*
General Purpose FFT(Fast Fourier / Cosine / Sine Transform) Package

Copyright:
Copyright(C) 1996-2001 Takuya OOURA
email: ooura@mmm.t.u-tokyo.ac.jp
download: http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html
You may use, copy, modify this code for any purpose and
without fee. You may distribute this ORIGINAL package.
*/
/*
Fast Fourier/Cosine/Sine Transform
dimension   :one
data length :power of 2
decimation  :frequency
radix       :split-radix
data        :inplace
table       :use
functions
cdft: Complex Discrete Fourier Transform
rdft: Real Discrete Fourier Transform
ddct: Discrete Cosine Transform
ddst: Discrete Sine Transform
dfct: Cosine Transform of RDFT (Real Symmetric DFT)
dfst: Sine Transform of RDFT (Real Anti-symmetric DFT)
function prototypes
void cdft(int, int, double *, int *, double *);
void rdft(int, int, double *, int *, double *);
void ddct(int, int, double *, int *, double *);
void ddst(int, int, double *, int *, double *);
void dfct(int, double *, double *, int *, double *);
void dfst(int, double *, double *, int *, double *);
macro definitions
USE_CDFT_PTHREADS : default=not defined
CDFT_THREADS_BEGIN_N  : must be >= 512, default=8192
CDFT_4THREADS_BEGIN_N : must be >= 512, default=65536
USE_CDFT_WINTHREADS : default=not defined
CDFT_THREADS_BEGIN_N  : must be >= 512, default=32768
CDFT_4THREADS_BEGIN_N : must be >= 512, default=524288


-------- Complex DFT (Discrete Fourier Transform) --------
[definition]
<case1>
X[k] = sum_j=0^n-1 x[j]*exp(2*pi*i*j*k/n), 0<=k<n
<case2>
X[k] = sum_j=0^n-1 x[j]*exp(-2*pi*i*j*k/n), 0<=k<n
(notes: sum_j=0^n-1 is a summation from j=0 to n-1)
[usage]
<case1>
ip[0] = 0; // first time only
cdft(2*n, 1, a, ip, w);
<case2>
ip[0] = 0; // first time only
cdft(2*n, -1, a, ip, w);
[parameters]
2*n            :data length (int)
n >= 1, n = power of 2
a[0...2*n-1]   :input/output data (double *)
input data
a[2*j] = Re(x[j]),
a[2*j+1] = Im(x[j]), 0<=j<n
output data
a[2*k] = Re(X[k]),
a[2*k+1] = Im(X[k]), 0<=k<n
ip[0...*]      :work area for bit reversal (int *)
length of ip >= 2+sqrt(n)
strictly,
length of ip >=
2+(1<<(int)(log(n+0.5)/log(2))/2).
ip[0],ip[1] are pointers of the cos/sin table.
w[0...n/2-1]   :cos/sin table (double *)
w[],ip[] are initialized if ip[0] == 0.
[remark]
Inverse of
cdft(2*n, -1, a, ip, w);
is
cdft(2*n, 1, a, ip, w);
for (j = 0; j <= 2 * n - 1; j++) {
a[j] *= 1.0 / n;
}
.


-------- Real DFT / Inverse of Real DFT --------
[definition]
<case1> RDFT
R[k] = sum_j=0^n-1 a[j]*cos(2*pi*j*k/n), 0<=k<=n/2
I[k] = sum_j=0^n-1 a[j]*sin(2*pi*j*k/n), 0<k<n/2
<case2> IRDFT (excluding scale)
a[k] = (R[0] + R[n/2]*cos(pi*k))/2 +
sum_j=1^n/2-1 R[j]*cos(2*pi*j*k/n) +
sum_j=1^n/2-1 I[j]*sin(2*pi*j*k/n), 0<=k<n
[usage]
<case1>
ip[0] = 0; // first time only
rdft(n, 1, a, ip, w);
<case2>
ip[0] = 0; // first time only
rdft(n, -1, a, ip, w);
[parameters]
n              :data length (int)
n >= 2, n = power of 2
a[0...n-1]     :input/output data (double *)
<case1>
output data
a[2*k] = R[k], 0<=k<n/2
a[2*k+1] = I[k], 0<k<n/2
a[1] = R[n/2]
<case2>
input data
a[2*j] = R[j], 0<=j<n/2
a[2*j+1] = I[j], 0<j<n/2
a[1] = R[n/2]
ip[0...*]      :work area for bit reversal (int *)
length of ip >= 2+sqrt(n/2)
strictly,
length of ip >=
2+(1<<(int)(log(n/2+0.5)/log(2))/2).
ip[0],ip[1] are pointers of the cos/sin table.
w[0...n/2-1]   :cos/sin table (double *)
w[],ip[] are initialized if ip[0] == 0.
[remark]
Inverse of
rdft(n, 1, a, ip, w);
is
rdft(n, -1, a, ip, w);
for (j = 0; j <= n - 1; j++) {
a[j] *= 2.0 / n;
}
.


-------- DCT (Discrete Cosine Transform) / Inverse of DCT --------
[definition]
<case1> IDCT (excluding scale)
C[k] = sum_j=0^n-1 a[j]*cos(pi*j*(k+1/2)/n), 0<=k<n
<case2> DCT
C[k] = sum_j=0^n-1 a[j]*cos(pi*(j+1/2)*k/n), 0<=k<n
[usage]
<case1>
ip[0] = 0; // first time only
ddct(n, 1, a, ip, w);
<case2>
ip[0] = 0; // first time only
ddct(n, -1, a, ip, w);
[parameters]
n              :data length (int)
n >= 2, n = power of 2
a[0...n-1]     :input/output data (double *)
output data
a[k] = C[k], 0<=k<n
ip[0...*]      :work area for bit reversal (int *)
length of ip >= 2+sqrt(n/2)
strictly,
length of ip >=
2+(1<<(int)(log(n/2+0.5)/log(2))/2).
ip[0],ip[1] are pointers of the cos/sin table.
w[0...n*5/4-1] :cos/sin table (double *)
w[],ip[] are initialized if ip[0] == 0.
[remark]
Inverse of
ddct(n, -1, a, ip, w);
is
a[0] *= 0.5;
ddct(n, 1, a, ip, w);
for (j = 0; j <= n - 1; j++) {
a[j] *= 2.0 / n;
}
.


-------- DST (Discrete Sine Transform) / Inverse of DST --------
[definition]
<case1> IDST (excluding scale)
S[k] = sum_j=1^n A[j]*sin(pi*j*(k+1/2)/n), 0<=k<n
<case2> DST
S[k] = sum_j=0^n-1 a[j]*sin(pi*(j+1/2)*k/n), 0<k<=n
[usage]
<case1>
ip[0] = 0; // first time only
ddst(n, 1, a, ip, w);
<case2>
ip[0] = 0; // first time only
ddst(n, -1, a, ip, w);
[parameters]
n              :data length (int)
n >= 2, n = power of 2
a[0...n-1]     :input/output data (double *)
<case1>
input data
a[j] = A[j], 0<j<n
a[0] = A[n]
output data
a[k] = S[k], 0<=k<n
<case2>
output data
a[k] = S[k], 0<k<n
a[0] = S[n]
ip[0...*]      :work area for bit reversal (int *)
length of ip >= 2+sqrt(n/2)
strictly,
length of ip >=
2+(1<<(int)(log(n/2+0.5)/log(2))/2).
ip[0],ip[1] are pointers of the cos/sin table.
w[0...n*5/4-1] :cos/sin table (double *)
w[],ip[] are initialized if ip[0] == 0.
[remark]
Inverse of
ddst(n, -1, a, ip, w);
is
a[0] *= 0.5;
ddst(n, 1, a, ip, w);
for (j = 0; j <= n - 1; j++) {
a[j] *= 2.0 / n;
}
.


-------- Cosine Transform of RDFT (Real Symmetric DFT) --------
[definition]
C[k] = sum_j=0^n a[j]*cos(pi*j*k/n), 0<=k<=n
[usage]
ip[0] = 0; // first time only
dfct(n, a, t, ip, w);
[parameters]
n              :data length - 1 (int)
n >= 2, n = power of 2
a[0...n]       :input/output data (double *)
output data
a[k] = C[k], 0<=k<=n
t[0...n/2]     :work area (double *)
ip[0...*]      :work area for bit reversal (int *)
length of ip >= 2+sqrt(n/4)
strictly,
length of ip >=
2+(1<<(int)(log(n/4+0.5)/log(2))/2).
ip[0],ip[1] are pointers of the cos/sin table.
w[0...n*5/8-1] :cos/sin table (double *)
w[],ip[] are initialized if ip[0] == 0.
[remark]
Inverse of
a[0] *= 0.5;
a[n] *= 0.5;
dfct(n, a, t, ip, w);
is
a[0] *= 0.5;
a[n] *= 0.5;
dfct(n, a, t, ip, w);
for (j = 0; j <= n; j++) {
a[j] *= 2.0 / n;
}
.


-------- Sine Transform of RDFT (Real Anti-symmetric DFT) --------
[definition]
S[k] = sum_j=1^n-1 a[j]*sin(pi*j*k/n), 0<k<n
[usage]
ip[0] = 0; // first time only
dfst(n, a, t, ip, w);
[parameters]
n              :data length + 1 (int)
n >= 2, n = power of 2
a[0...n-1]     :input/output data (double *)
output data
a[k] = S[k], 0<k<n
(a[0] is used for work area)
t[0...n/2-1]   :work area (double *)
ip[0...*]      :work area for bit reversal (int *)
length of ip >= 2+sqrt(n/4)
strictly,
length of ip >=
2+(1<<(int)(log(n/4+0.5)/log(2))/2).
ip[0],ip[1] are pointers of the cos/sin table.
w[0...n*5/8-1] :cos/sin table (double *)
w[],ip[] are initialized if ip[0] == 0.
[remark]
Inverse of
dfst(n, a, t, ip, w);
is
dfst(n, a, t, ip, w);
for (j = 1; j <= n - 1; j++) {
a[j] *= 2.0 / n;
}
.


Appendix :
The cos/sin table is recalculated when the larger table required.
w[] and ip[] are compatible with all routines.
*/

#ifndef _FFTSG_H_
#define _FFTSG_H_


#include <math.h>

//General Purpose FFT(Fast Fourier / Cosine / Sine Transform) Package

void cdft(int, int, double *, int *, double *);
void rdft(int, int, double *, int *, double *);
void ddct(int, int, double *, int *, double *);
void ddst(int, int, double *, int *, double *);
void dfct(int, double *, double *, int *, double *);
void dfst(int, double *, double *, int *, double *);


void cftfsub(int n, double *a, int *ip, int nw, double *w);
void cftbsub(int n, double *a, int *ip, int nw, double *w);
void rftfsub(int n, double *a, int nc, double *c);
void rftbsub(int n, double *a, int nc, double *c);
void dctsub(int n, double *a, int nc, double *c);
void dstsub(int n, double *a, int nc, double *c);

//initializing routines
void makewt(int nw, int *ip, double *w);
void makect(int nc, int *ip, double *c);
void makeipt(int nw, int *ip);

//child routines

void bitrv2(int n, int *ip, double *a);
void bitrv216(double *a);
void bitrv208(double *a);
void cftf1st(int n, double *a, double *w);
void cftrec4(int n, double *a, int nw, double *w);
void cftleaf(int n, int isplt, double *a, int nw, double *w);
void cftfx41(int n, double *a, int nw, double *w);
void cftf040(double *a);
void cftx020(double *a);

void bitrv2conj(int n, int *ip, double *a);
void bitrv216neg(double *a);
void bitrv208neg(double *a);
void cftb1st(int n, double *a, double *w);
void cftrec4(int n, double *a, int nw, double *w);
void cftleaf(int n, int isplt, double *a, int nw, double *w);
void cftfx41(int n, double *a, int nw, double *w);
void cftb040(double *a);
void cftx020(double *a);

int cfttree(int n, int j, int k, double *a, int nw, double *w);
void cftmdl1(int n, double *a, double *w);
void cftmdl2(int n, double *a, double *w);
void cftf161(double *a, double *w);
void cftf162(double *a, double *w);
void cftf081(double *a, double *w);
void cftf082(double *a, double *w);


#endif