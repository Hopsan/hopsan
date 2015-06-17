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
class ContainerObject;
class TextBoxWidget;


ModelObject* loadModelObject(QDomElement &rDomElement, ContainerObject* pContainer, UndoStatusEnumT undoSettings=Undo);

ModelObject* loadContainerPortObject(QDomElement &rDomElement, ContainerObject* pContainer, UndoStatusEnumT undoSettings=Undo);

bool loadConnector(QDomElement &rDomElement, ContainerObject* pContainer, UndoStatusEnumT undoSettings=Undo);

void loadParameterValue(QDomElement &rDomElement, ModelObject* pObject, UndoStatusEnumT undoSettings=Undo);

void loadStartValue(QDomElement &rDomElement, ModelObject* pObject, UndoStatusEnumT undoSettings=Undo);

void loadSystemParameter(QDomElement &rDomElement, const QString hmfVersion, ContainerObject* pContainer);

//void loadFavoriteVariable(QDomElement &rDomElement, ContainerObject* pContainer);

void loadPlotAlias(QDomElement &rDomElement, ContainerObject* pContainer);

TextBoxWidget* loadTextBoxWidget(QDomElement &rDomElement, ContainerObject *pContainer, UndoStatusEnumT undoSettings=Undo);

#endif // LOADFUNCTIONS_H
