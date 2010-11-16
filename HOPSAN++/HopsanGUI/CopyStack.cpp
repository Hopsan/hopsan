//!
//! @file   Configuration.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for the configuration object
//!
//$Id$

#include "common.h"
#include "CopyStack.h"
#include <QDomElement>

#include <QDomDocument>
#include <QDomElement>

CopyStack::CopyStack()
{
    this->clear();
}


void CopyStack::clear()
{
    mCopyRoot.clear();
    mCopyRoot = mDomDocument.createElement("hopsancopydata");
    mDomDocument.appendChild(mCopyRoot);
}


QDomElement *CopyStack::getCopyRoot()
{
    return &mCopyRoot;
}
