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
//! @file   loadFunctions.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains classes and functions used to recreate models from load data
//!
//$Id$

#ifndef LOADFUNCTIONS_H
#define LOADFUNCTIONS_H

#include "Utilities/XMLUtilities.h"
#include "common.h"


//Forward Declarations
class LibraryWidget;
class ModelObject;
class SystemObject;
class TextBoxWidget;
class ImageWidget;


ModelObject* loadModelObject(const QDomElement &domElement, SystemObject* pSystem, UndoStatusEnumT undoSettings=Undo);

ModelObject* loadSystemPortObject(QDomElement &rDomElement, SystemObject* pSystem, UndoStatusEnumT undoSettings=Undo);

bool loadConnector(QDomElement &rDomElement, SystemObject* pContainer, UndoStatusEnumT undoSettings=Undo);

void loadParameterValue(QDomElement &rDomElement, ModelObject* pObject, UndoStatusEnumT undoSettings=Undo);

void loadStartValue(QDomElement &rDomElement, ModelObject* pObject, UndoStatusEnumT undoSettings=Undo);

void loadSystemParameter(QDomElement &rDomElement, bool doAdd, const QString hmfVersion, SystemObject* pContainer);

void loadPlotAlias(QDomElement &rDomElement, SystemObject* pContainer);

TextBoxWidget* loadTextBoxWidget(QDomElement &rDomElement, SystemObject *pContainer, UndoStatusEnumT undoSettings=Undo);

ImageWidget* loadImageWidget(QDomElement &rDomElement, SystemObject *pContainer, UndoStatusEnumT undoSettings=Undo);

#endif // LOADFUNCTIONS_H
