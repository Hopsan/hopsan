/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#ifndef HOPSANLABVIEWGENERATOR_H
#define HOPSANLABVIEWGENERATOR_H

#include "HopsanGeneratorBase.h"

class HopsanLabViewGenerator : public HopsanGeneratorBase
{
public:
    HopsanLabViewGenerator(const QString &hopsanInstallPath);
    bool generateToLabViewSIT(QString savePath, hopsan::ComponentSystem *pSystem);
};

#endif // HOPSANLABVIEWGENERATOR_H
