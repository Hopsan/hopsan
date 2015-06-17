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
//! @file   ProgressBarThread.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-11
//!
//! @brief Contains a class for displaying a progress bar in a separate class
//!
//$Id$


#ifndef PROGRESSBARTHREAD_H
#define PROGRESSBARTHREAD_H

#include <QThread>
//#include <QProgressDialog>

//class ModelWidget;

class ProgressBarThread : public QThread
{
public:
    ProgressBarThread(QObject *parent/*ModelWidget *parent*/);

protected:
    void run();

private:
    //ModelWidget *mpParentModelWidget;
};

#endif // PROGRESSBARTHREAD_H
