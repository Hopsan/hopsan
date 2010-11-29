//!
//! @file   matrix.h
//! @author ?? <??@??.??>
//! @date   2010-11-29
//!
//! @brief Contains a classes for matrix and vector
//!
//!
//! matrix data structure, and routines for matrix manipulation.
//! (found by Petter on the net, ref may be nice)
//!
//$Id$

#ifndef __MATRIX_H
#define __MATRIX_H
#include <string>
#include <assert.h>

namespace hopsan {

    // \typedef
    //! pointer to a function which returns a double and takes as argument a double
    typedef double (*V_FCT_PTR)(double);

    //! defines a vector of doubles
    class Vec {
    public:
	
	Vec(): n(0), body(0) { } //!< default no-argument constructor
	//! constructs a vector of length n
	Vec(int size): n(0), body(0) { create(size); }
	Vec(const Vec &src): n(0), body(0) { copy(src); } //!< copy constructor
	~Vec() { delete [] body; body=0; n=0; } //!< destructor
	//! creates (or resets) vector length and allocated space
	void create(int size);
	//! returns the length (number of elements) of the vector
	int length() const { return n; }
	//! prints vector contents to stdout
	void print() const;
	//! set to constant value
	Vec &set(double v) { for (int i=0; i<length(); i++) body[i] = v; return *this; }
	//! subscript operator (non-const object)
	double &operator[](int n) {return body[n]; }
	//! subscript operator (const object)
	const double &operator[](int n) const { return body[n]; }
	Vec &operator=(const Vec &src) { return (this==&src? *this: copy(src)); } //!< dst = src
	//! applys function fct element-by-element
	Vec &apply(V_FCT_PTR fct);
	double max(); //!< returns maximum Vec element
	double min(); //!< returns minimum Vec element
	double norm(); //< returns 2-norm (magnitude) of Vec
	void normalize(); //< convert Vec to unit magnitude
	//! returns  dot product of a and b
	static double dot(const Vec &a, const Vec &b);
	
	// operator overloads
	friend Vec operator+(const Vec &a, const Vec &b);  //!< returns a+b
	friend Vec operator-(const Vec &a, const Vec &b); //!< returns a-b
	friend Vec operator*(double c, const Vec &a);     //!< returns scalar multiplied by Vec
	friend Vec operator*(const Vec &a, double c); //!< returns Vec multiplied by scalar
	friend Vec operator*(const Vec &a, double c); //!< returns Vec divided by scalar

    private:
	double *body;	            //!< contains the elements of the vector
	int n;	                    //!< number of elements in the vector
	Vec &copy(const Vec &src);  //!< replace current vector with src
    };

    std::ostream& operator << (std::ostream& s, const Vec& v);

    //! defines a two-dimensional Matrix of doubles
    class Matrix {
    public:
	//! default (no argument) constructor	
	Matrix(): nrows(0), ncols(0), body(0) {}
	//! constructs a Matrix of specified size
	Matrix(int rows, int cols): nrows(0), ncols(0), body(0) { create(rows,cols);}
	Matrix(const Matrix &src): nrows(0), ncols(0), body(0) { copy(src); } //!< copy	constructor
	~Matrix(); //!< destructor
	//! creates(or resets) Matrix size and allocated space.
	void create(int rows, int cols);
	int cols()const { return ncols; } //!< returns the number of columns
	int rows()const { return nrows; } //!< returns the number of rows
	//! set Matrix elements to v
	Matrix &set(double v);
	Matrix &operator =(const Matrix &mx) { return (this==&mx? *this: copy(mx)); }
	Matrix operator *(const Matrix &mx) const;
	//static Vec mul(const Matrix &mx, const Vec &v);
	//static Vec mul(const Vec &v, const Matrix &mx);
	//! swap rows
	void swaprows(int i, int j);
	//! returns the transpose of the Matrix
	Matrix transpose();
	//! returns a pointer to a Matrix row
	double * operator [](int n) const { return body[n]; }
	//const double * operator [](int n) const { return body[n]; }
	//! prints Matrix (using mprint)
	void print() const;
	// friends
	friend Vec operator *(const Matrix &mx, const Vec &v); //!< matix multipled by Vec
	friend Vec operator *(const Vec &v, const Matrix &mx); //!< Vec multiplied by Matrix
    private:
	int nrows; //!< number of rows in the Matrix
	int ncols; //!< number of columns in the Matrix
	double **body; //!< space allocated for the Matrix
	Matrix &copy(const Matrix &mx); //!< replaces Matrix with Matrix mx

    };
    /*! returns the inner (or dot) product of a vector
 * @param a first vector
 * @param b second vector
 * @returns (a dot b)
 */
    inline double Vec::dot(const Vec &a, const Vec &b)
    {
	double sum = 0.0;
	int n = a.length();
	assert(b.length()==n);
	for (int i=0; i<n; i++) sum += a[i]*b[i];
	return sum;
    }



    Matrix identity(int size);
    Matrix diagonal(double *v, int size);
    int read_matrix(Matrix &mx, std::string &title, FILE *in);
    void print_vector(double *v, int n);
    void copy_matrix(Matrix &dst, Matrix &src);
    double *vector(int length);
    int *ivector(int length);
    void errmsg(char *text);

}

#endif
