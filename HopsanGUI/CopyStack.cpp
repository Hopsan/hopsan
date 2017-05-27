/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   CopyStack.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for an XML-based copy stack
//!
//$Id$

#include "common.h"
#include "CopyStack.h"
#include <QDomElement>

#include <QDomDocument>
#include <QDomElement>


//! @class CopyStack
//! @brief The CopyStack class is used as an XML-based storage for copy/paste information
//!
//! Add the objects you want to copy to the root object in the stack, and retrieve them by using the XML load routines. Clear the stack before each new copy operation.
//!


//! @brief Constructor for the copy stack
CopyStack::CopyStack()
{
    this->clear();
}


//! @brief Clears all contents in the copy stack
void CopyStack::clear()
{
    mCopyRoot.clear();
    mCopyRoot = mDomDocument.createElement("hopsancopydata");
    mDomDocument.appendChild(mCopyRoot);
}


//! @brief Returns the raw XML data from the copy stack (for debugging purposes only)
QString CopyStack::getXML()
{
    return mDomDocument.toString();
}


//! @brief Returns a pointer to the XML root element in the copy stack
QDomElement *CopyStack::getCopyRoot()
{
    return &mCopyRoot;
}
