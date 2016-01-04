#ifndef MODEL_H
#define MODEL_H

class ComponentSystem;

class Model
{
public:
    Model();

    double simStartTime;
    double simStopTime;
    ComponentSystem *mpToplevelSystem;
};

#endif // MODEL_H
