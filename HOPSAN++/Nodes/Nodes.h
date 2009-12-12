#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

class Node
{
public:
    Node(string name);

    virtual void writeData(int dataPos, double data) =0;

    virtual double readData(int dataPos) =0;

private:
    string mName;
    vector<double> mDataVector;

};

class MekNode : public Node // Måste ha samma uppsättning attribut och metoder som Node för att vara "polymorphic"
{
public:
    MekNode(string name);

    void writeData(int dataPos, double data);

    double readData(int dataPos);

};

#endif // NODES_H_INCLUDED
