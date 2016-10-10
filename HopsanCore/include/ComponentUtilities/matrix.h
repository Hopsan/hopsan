/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   matrix.h
//! @author ?? <??@??.??>
//! @date   2010-11-29
//!
//! @brief Contains a classes for matrix and vector
//!
//!
//! matrix data structure, and routines for matrix manipulation.
//! (found by Petter on the net, url may be nice)
//! Found on John Loomis's website http://www.johnloomis.org/ece538/notes/Matrix/Matrix_C.html#matrix.h
//!
//$Id$

#ifndef __MATRIX_H
#define __MATRIX_H

#include "win32dll.h"
#include <cassert>

namespace hopsan {

    // \typedef
    //! pointer to a function which returns a double and takes as argument a double
    typedef double (*V_FCT_PTR)(double);

    //! defines a vector of doubles
    //! @ingroup ComponentUtilityClasses
    class DLLIMPORTEXPORT Vec {
    public:
        Vec(); //!< default no-argument constructor
        Vec(const Vec &src); //!< copy constructor
        Vec(int size); //!< constructs a vector of length n
        ~Vec(); //!< destructor
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
        DLLIMPORTEXPORT friend Vec operator+(const Vec &a, const Vec &b);  //!< returns a+b
        friend Vec DLLIMPORTEXPORT operator-(const Vec &a, const Vec &b); //!< returns a-b
        DLLIMPORTEXPORT friend Vec operator*(double c, const Vec &a);     //!< returns scalar multiplied by Vec
        DLLIMPORTEXPORT friend Vec operator*(const Vec &a, double c); //!< returns Vec multiplied by scalar
        DLLIMPORTEXPORT friend Vec operator/(const Vec &a, double c); //!< returns Vec divided by scalar

    private:
        int n;	                    //!< number of elements in the vector
        double *body;	            //!< contains the elements of the vector
        Vec &copy(const Vec &src);  //!< replace current vector with src
    };

    //! defines a two-dimensional Matrix of doubles
    //! @ingroup ComponentUtilityClasses
    class DLLIMPORTEXPORT Matrix {
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
        Vec operator *(const Vec &v) const; //!< matix multipled by Vec
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



    DLLIMPORTEXPORT Matrix identity(int size);
    DLLIMPORTEXPORT Matrix diagonal(double *v, int size);
    //DLLIMPORTEXPORT int read_matrix(Matrix &mx, std::string &title, FILE *in);
    DLLIMPORTEXPORT void print_vector(double *v, int n);
    DLLIMPORTEXPORT void copy_matrix(Matrix &dst, Matrix &src);
    DLLIMPORTEXPORT double* vector(int length);
    DLLIMPORTEXPORT int* ivector(int length);
    DLLIMPORTEXPORT void errmsg(char *text);
}

#endif
