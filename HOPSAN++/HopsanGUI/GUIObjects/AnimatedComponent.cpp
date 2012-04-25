#include "AnimatedComponent.h"
#include <float.h>
#include "GUIPort.h"



AnimatedComponent::AnimatedComponent(ModelObject* unanimatedComponent, QString basePath, QStringList movablePaths, QStringList dataPorts,
                                     QStringList dataNames, QStringList parameterMultipliers, QStringList parameterDivisors, QVector<double> movementX, QVector<double> movementY, QVector<double> movementTheta, QVector<double> startX,
                                     QVector<double> startY, QVector<double> startTheta, QVector<double> transformOriginX, QVector<double> transformOriginY, AnimationWidget *parent)
    : QObject(parent /*parent*/)
{

    //Something all Animated Components Should have. ie. Base, movable, etc.
    mpParent = parent;
    mpUnanimatedComponent = unanimatedComponent;
    mpOriginal = new ModelObject(unanimatedComponent->getCenterPos().toPoint(),unanimatedComponent->rotation(),unanimatedComponent->getAppearanceData(),DESELECTED,USERGRAPHICS, unanimatedComponent->getParentContainerObject(),unanimatedComponent->parentItem());
    gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->scene()->removeItem(mpOriginal);
    mpParent = parent;
    this->mnMovableParts = movablePaths.size();
    this->mpData = new QList<QVector<double> >();
    this->mpMins = new QVector<double>();
    this->mpMaxes = new QVector<double>();

    mStartX = startX;
    mStartY = startY;
    mStartTheta = startTheta;

    mvMovementX = movementX;
    mvMovementY = movementY;
    mvMovementTheta = movementTheta;

    mParameterMultipliers = parameterMultipliers;
    mParameterDivisors = parameterDivisors;

   // qDebug() << "mvMovementTheta = " << mvMovementTheta;

    if(mnMovableParts > 0)
    {
        setupAnimationBase(basePath);

        for(int i=0; i<movablePaths.size(); ++i)
        {
            setupAnimationMovable(0,movablePaths.at(i),startX.at(i),startY.at(i),transformOriginX.at(i),transformOriginY.at(i));
            if(unanimatedComponent->getPort(dataPorts.at(i))->isConnected())
            {
                mpData->insert(i,mpParent->getPlotDataPtr()->at(mpParent->getNumberOfPlotGenerations()-1).find(unanimatedComponent->getName()).value().find(dataPorts.at(i)).value().find(dataNames.at(i)).value().second);
            }
        }

        this->calculateMinsAndMaxes();
    }
}

ModelObject* AnimatedComponent::getOriginal()
{
    return this->mpOriginal;
}

void AnimatedComponent::draw()
{
    //mpParent->getScenePtr()->addItem(mpOriginal);
    if(mnMovableParts > 0)
    {
        this->mpParent->getScenePtr()->addItem(this->mpBase);
        for(int b=0; b<mnMovableParts; ++b)
        {
            this->mpParent->getScenePtr()->addItem(this->mpMovables.at(b));
        }
    }
    else
    {
        this->mpParent->getScenePtr()->addItem(this->mpOriginal);
    }
}

void AnimatedComponent::update()
{
    for(int b=0; b<mnMovableParts; ++b)
    {
        double data = mpData->at(b).at(mpParent->getIndex());
        if(mParameterMultipliers.at(b) != QString())
        {
            data = data*mpUnanimatedComponent->getParameterValue(mParameterMultipliers.at(b)).toDouble();
        }
        if(mParameterDivisors.at(b) != QString())
        {
            data = data/mpUnanimatedComponent->getParameterValue(mParameterDivisors.at(b)).toDouble();
        }

        double x = mpOriginal->pos().x() + mStartX.at(b) - (data*mvMovementX.at(b));
        double y = mpOriginal->pos().y() + mStartY.at(b) - (data*mvMovementY.at(b));
        mpMovables.at(b)->setPos(x, y);
        mpMovables.at(b)->setRotation(data/mpMaxes->at(b)*mvMovementTheta.at(b) - mStartTheta.at(b));
    }
    return;
}

void AnimatedComponent::calculateMinsAndMaxes()
{
    for(int h=0; h<mpData->size(); ++h)
    {
        double max = -DBL_MAX;
        for(int i=0; i<mpData->at(h).size(); ++i)
        {
            if(mpData->at(h).at(i) > max)
            {
                max = mpData->at(h).at(i);
            }
        }
        mpMaxes->insert(h,max);
    }

    for(int j=0;j<mpData->size();j++)
    {
        double min = DBL_MAX;
        for(int k=0;k<mpData->at(j).size();k++)
        {
            if(mpData->at(j).at(k)<min)
            {
                min = mpData->at(j).at(k);
            }
        }
        mpMins->insert(j,min);
    }
}


void AnimatedComponent::setupAnimationBase(QString basePath)
{
    ModelObjectAppearance *baseAppearance = new ModelObjectAppearance();
    baseAppearance->setIconPath(basePath, USERGRAPHICS, RELATIVE);
    mpBase = new ModelObject(mpUnanimatedComponent->pos().toPoint(),mpUnanimatedComponent->rotation(),baseAppearance,DESELECTED,USERGRAPHICS,0,0);
}


void AnimatedComponent::setupAnimationMovable(int n, QString movablePath, int startX, int startY, double transformOriginX, double transformOriginY)
{
    ModelObjectAppearance* tempNUMBER = new ModelObjectAppearance();
    tempNUMBER->setIconPath(movablePath,USERGRAPHICS, RELATIVE);
    this->mpMovables.append(new ModelObject(QPoint(0,0),0, tempNUMBER ,DESELECTED,USERGRAPHICS,0,0));
    this->mpMovables.at(n)->setTransformOriginPoint(transformOriginX,transformOriginY);
    mpMovables.at(n)->setPos(mpOriginal->pos().x()+startX, mpOriginal->pos().y()+startY);
}


