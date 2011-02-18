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


//! @brief Clears all contentse in the copy stack
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
