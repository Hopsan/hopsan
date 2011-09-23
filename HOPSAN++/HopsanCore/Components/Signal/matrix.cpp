/* matrix.cpp
*
*    Utility routines for allocating space for matrices and vectors,
*    printing matrices and vectors, and copying one matrix to another.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
using namespace std;
#include "matrix.h"

void Matrix::create(int rows, int cols)
{
	if (rows==nrows && cols==ncols) return;
	while (nrows) delete [] body[--nrows];
	if (body) delete body;
	nrows = rows;
	ncols = cols;
	body = new double * [nrows];
	assert(body);
	for (int i=0; i<nrows; i++) {
		body[i]= new double[ncols];
		assert(body[i]);
	}
}

/*!
*   Copy Matrix src to the current Matrix
*/
Matrix &Matrix::copy(const Matrix &src)
{
	int i, j;
	double *up, *vp;
	create(src.rows(),src.cols());
	for (i=0; i<nrows; i++) {
		up = src[i];
		vp = body[i];
		for (j=0; j<ncols; j++) *vp++ = *up++;
	}
	return *this;
}

Matrix::~Matrix()
{
	while (nrows) delete [] body[--nrows];
	delete [] body;
}

Matrix &Matrix::set(double v)
{
	int i, j;
	double *p;
	for (i=0; i<nrows; i++) {
		p = body[i];
		for (j=0; j<ncols; j++) *p++ = v;
	}
	return *this;
}

void Matrix::swaprows(int i,int j)
{
	double *p = body[i];
	body[i]=body[j];
	body[j]=p;
}

Matrix Matrix::transpose()
{
	int i, j;
	Matrix out(ncols,nrows);
	for (i=0; i<nrows; i++) {
		for (j=0; j<ncols; j++) {
			out[j][i] = body[i][j];
		}
	}
	return out;
}

//! Matrix multiplication

Matrix Matrix::operator *(const Matrix &mx) const
{
	int i, j, k;
	double sum;
	double *vp;
	assert(ncols == mx.rows()); // check for non-conforming Matrix multiplication
	Matrix out(nrows,mx.cols());
	for (i=0; i<nrows; i++) {
		vp = out[i];
		for (j=0; j<mx.cols(); j++) {
			sum = 0.0;
			for (k=0; k<ncols; k++)
				sum += body[i][k] * mx[k][j];
			*vp++ = sum;
		}
	}
	return out;
}

Vec operator *(const Matrix &mx, const Vec &v)
{
	int i, j;
	int m = mx.rows();
	int n = mx.cols();
	double sum;
	assert(m>0 && n>0);
	assert(n==v.length());
	Vec vout(m);
	for (i=0; i<m; i++) {
		sum = 0.0;
		double *ptr = mx[i];
		for (j=0; j<n; j++) sum += (*ptr++) * v[j];
		vout[i] = sum;
	}
	return vout;
}

Vec operator *(const Vec &v, const Matrix &mx)
{
	int i, j;
	int m = mx.rows();
	int n = mx.cols();
	double sum;
	assert(m>0 && n>0);
	assert(m==v.length());
	Vec vout(n);
	for (j=0; j<n; j++) {
		sum = 0.0;
		for (i=0; i<m; i++) sum += v[i] * mx[i][j];
		vout[j] = sum;
	}
	return vout;
}

//!    create an Identity Matrix
Matrix identity(int size)
{
	int i, j;
	double *vp;
	Matrix out(size,size);
	for (i=0; i<size; i++) {
		vp = out[i];
		for (j=0; j<size; j++) vp[j]=0.0;
		vp[i] = 1.0;
	}
	return out;
}


//! create a diagonal Matrix
/*!
 *  @param v is an array of diagonal elements
 *  @param size is the length of the array
 *  
 */
Matrix diagonal(double *v, int size)
{
	int i, j;
	double *vp;
	Matrix out(size,size);
	for (i=0; i<size; i++) {
		vp = out[i];
		for (j=0; j<size; j++) vp[j]=0.0;
		vp[i] = v[i];
	}
	return out;
}

//! read Matrix from text file
/*! @param mx destination Matrix
 *  @param title c-string comment (remainder of header line)
 *  @param in input file
 *  
*  The first two entries are number of rows and columns.\n
*  The rest of the line is a title.\n
*  The body of the Matrix follows.\n
*/

int read_matrix(Matrix &mx, string &title, FILE *in)
{
	int i, j;
	int nrows, ncols;
	double *v;
	double vin;
	char *cp;
	char buf[256];

//	fscanf_s(in," %d %d",&nrows,&ncols);
	if (feof(in)) return -1;
	fgets(buf,255,in);
	if (feof(in)) return -1;
	cp = buf;
	while (*cp) {
		if (*cp=='\n') *cp = 0;
		else cp++;
	}
	title = buf;
	mx.create(nrows,ncols);
	for (j=0; j<nrows; j++) {
		v  = mx[j];
		for (i=0; i<ncols; i++) {
//			fscanf_s(in," %lf",&vin);
			*v++ = vin;
		}
		if (feof(in)) {
			printf("\nerror reading %s\n",title);
			printf("Matrix has %d rows and %d columns\n",
				mx.rows(),mx.cols());
			printf("Unexpected EOF reading row %d",j+1);
			return -1;
		}
	}
	return 0;
}



//! create memory for elements of vector
void Vec::create(int size)
{
	assert(size);
	if (size == n) return;
	if (body != 0) delete [] body;
	body = new double[size];
	assert(body!=0);
	n = size;
	//memset((void *)body, 0, (size_t)(n*sizeof(double));
}
//! copy constructor
Vec &Vec::copy(const Vec &src)
{
	int size = src.length();
	create(size);
	for (int i=0; i<n; i++) body[i] = src[i];
	return *this;
}

double Vec::max()
{
	double vmax = body[0];
	for (int i=1; i<n; i++) if (body[i]>vmax) vmax = body[i];
	return vmax;
}

double Vec::min()
{
	double vmin = body[0];
	for (int i=1; i<n; i++) if (body[i]<vmin) vmin = body[i];
	return vmin;
}

double Vec::norm()
{
	double sum = 0;0;
	for (int i=0; i<n; i++) sum += body[i]*body[i];
	return sqrt(sum);
}

void Vec::normalize()
{
	double vnorm = norm();
	for (int i=0; i<n; i++) body[i] /= vnorm;
}
	

Vec& Vec::apply(V_FCT_PTR fct)
{
	for (int i=0; i<n; i++) body[i] = (*fct)(body[i]);
	return *this;
}

//! returns a + b
Vec operator +(const Vec &a, const Vec &b)
{
	int n = a.length();
	assert(b.length()==n);
	Vec sum(n);
	for (int i=0; i<n; i++) sum[i] = a[i] + b[i];
	return sum;
}

//! returns a - b
Vec operator -(const Vec &a, const Vec &b)
{
	int n = a.length();
	assert(b.length()==n);
	Vec diff(n);
	for (int i=0; i<n; i++) diff[i] = a[i] - b[i];
	return diff;
}

//! returns scalar multiplied by a vector
Vec operator *(double c, const Vec &a)
{
	int n = a.length();
	Vec vout(n);
	for (int i=0; i<n; i++) vout[i] = c*a[i];
	return vout;
}

//! returns vector mutliplied by a scalar
Vec operator *(const Vec &a, double c)
{
	int n = a.length();
	Vec vout(n);
	for (int i=0; i<n; i++) vout[i] = c*a[i];
	return vout;
}

//! returns vector divided by a scalar
Vec operator /(const Vec &a, double c)
{
	int n = a.length();
	Vec vout(n);
	for (int i=0; i<n; i++) vout[i] = a[i]/c;
	return vout;
}

double *vector(int n)
{
	double *v;
	v = new double[n];
	assert(v);
	return v;
}


int *ivector(int n)
{
	int *iv;
	iv = new int[n];
	assert(iv);
	return iv;
}


void Matrix::print() const
{
	int i, j;
	const double tiny = 1e-13;
	double *vp;
	double v;
	for (j=0; j<nrows; j++) {
		vp = body[j];
		for (i=0; i<ncols; i++) {
			v = *vp++;
			if (fabs(v)<tiny) v = 0.0;
			
			printf("% 10.4g", v);
		}
		printf("\n");
	}
}

void Vec::print() const
{
	double *v = body;
	int size = n;
	while (size--) {
		printf(" %10.4g", *v++);
	}
	printf("\n");
}

//! copy one Matrix to another
/*!
 * @param dst destination Matrix
 * @param src source Matrix
 * if destination is smaller than the source, the source is truncated.\n
 * if destination is larger than the source, the remainder is
 * zero-filled\n
 */

void copy_matrix(Matrix &dst, Matrix &src)
{
	int i, j;
	int nrows, ncols;
	double *s, *t;;
	nrows = (dst.rows()<src.rows()? dst.rows(): src.rows());
	ncols = (dst.cols()<src.cols()? dst.cols(): src.cols());
	for (j=0; j<nrows; j++) {
		s = src[j];
		t = dst[j];
		for (i=0; i<ncols; i++) *t++ = *s++;
	}
	// Append zero-filled rows to destination
	for (j=src.rows(); j<nrows; j++) {
		t = dst[j];
		for (i=0; i<ncols; i++) t[i] = 0.0;
	}
	// Append zero-filled columns to destination
	if (src.cols() < ncols) {
		for (j=0; j<dst.rows(); j++) {
			t = dst[j];
			for (i=src.cols(); i<ncols; i++) t[i] = 0.0;
		}
	}
}

// overloaded insertion operator <<
std::ostream& operator << (std::ostream& s, const Vec& v)
{
	char buf[16];
	int n = v.length();
	if (n > 0)
		{
		//int old_precision = s.precision() ; // get current precision
		s << "[" ;
		for (int i=0; i<n; i++)
		{
//			sprintf_s(buf,15,"%.4g",v[i]);
			//s << setprecision (2)
				//<< setiosflags (ios_base::showpoint|ios_base::fixed)
			s << buf ;
			i!=n-1 ? s << ", " : s << "]" ;
			}
      //s << endl ;
		// reset precision and ios flags
		//s.precision (old_precision) ;
		//std::resetiosflags (ios::showpoint|std::ios::fixed) ;
		}
	return s ;
	}
