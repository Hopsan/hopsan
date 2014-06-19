/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   CSVParser.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-12
//!
//! @brief Contains the Core Utility CSVParser class
//!
//$Id$

#ifndef CVSPARSER_H_INCLUDED
#define CVSPARSER_H_INCLUDED

#include "win32dll.h"
#include <vector>
#include "HopsanTypes.h"

// Forward declaration
class csv_parser;

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT CSVParser
{
public:
    CSVParser(bool &rSuccess,
              const HString filename,
              const char line_terminator = '\n',
              const char enclosure_char = '"');

    HString getErrorString() const;
    size_t getNumDataRows() const;
    size_t getNumDataCols() const;
    const std::vector<double> getDataColumn(const size_t idx) const;

    int getIncreasingOrDecresing(const size_t idx) const;
    void calcIncreasingOrDecreasing();
    void sortIncreasing(const size_t indexColumn);

    bool isInDataIncOrDec(const size_t inCol);
    double interpolate(const double x, const size_t outCol) const;
    double interpolate(const double x, const size_t inCol, const size_t outCol) const;

    void transpose();

protected:
    size_t intervalHalfSubDiv(const size_t colIdx, const double x, const size_t i1, const size_t iend) const;
    size_t quickSortPartition(std::vector<double> &rIndexArray, const size_t left, const size_t right, const size_t pivotIndex);
    void quickSort(std::vector<double> &rIndexArray, const size_t left, const size_t right);
    void swapRows(const size_t r1, const size_t r2);
    void reverseRows();
    void setFirstLastValues();

    std::vector< std::vector<double> > mData;
    std::vector<double> mFirstValues, mLastValues;
    size_t mnDataRows, mnDataCols;
    std::vector< int > mIncDec;
    HString mErrorString;
};

class DLLIMPORTEXPORT CSVParserNG
{
public:
    CSVParserNG(const char line_terminator = '\n', const char enclosure_char = '"');
    ~CSVParserNG();

    void clearData();
    bool isEmpty() const;

    void setLineTerminator(const char lt);
    void setFieldEnclosureChar(const char fec);

    bool setFile(const HString &rFilepath);

    bool parseEntireFile();

    bool eof() const;

    HString getErrorString() const;
    size_t getNumDataRows() const;
    size_t getNumDataCols() const;

    bool copyRow(const size_t rowIdx, std::vector<double> &rRow);
    bool copyRow(const size_t rowIdx, std::vector<long int> &rRow);
    bool copyColumn(const size_t columnIdx, std::vector<double> &rColumn);
    bool copyRangeFromColumn(const size_t columnIdx, const size_t startRow, const size_t numRows, std::vector<double> &rColumn);
    bool copyEveryNthFromColumn(const size_t columnIdx, const size_t stepSize, std::vector<double> &rColumn);

protected:
    std::vector<HString> mData;
    size_t mNumDataRows, mNumDataCols;

    char mLineTerminator, mFieldEnclosureChar, mFieldSeparator;
    size_t mNumLinesToSkip;
    csv_parser *mpCsvParser;

    HString mErrorString;
    bool mConvertDecimalSeparator;
};



}

#endif // CVSPARSER_H_INCLUDED
