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
