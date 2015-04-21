#ifndef INDEXINTERVALCOLLECTION_H
#define INDEXINTERVALCOLLECTION_H

#include <QList>

class IndexIntervalCollection
{
public:
    class MinMaxT
    {
    public:
        MinMaxT(int min, int max);
        int mMin, mMax;
    };

    void addValue(const int val);
    void removeValue(const int val);
    int min() const;
    int max() const;
    bool isContinuos() const;
    bool isEmpty() const;
    bool contains(const int val) const;
    void clear();

    QList<MinMaxT> getList() const;
    QList<int> getCompleteList() const;
    int getNumIIC() const;
    int getNumI() const;

    void testMe();

private:
    void mergeIntervals(int first, int second);
    QList<MinMaxT> mIntervalList;
};

#endif // INDEXINTERVALCOLLECTION_H
