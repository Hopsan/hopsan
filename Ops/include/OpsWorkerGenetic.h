#ifndef OPSWORKERGENETIC_H
#define OPSWORKERGENETIC_H
#include "OpsWorker.h"

#include <random>

namespace Ops {

class OPS_DLLAPI WorkerGenetic : public Worker
{
public:
  WorkerGenetic(Evaluator *pEvaluator, MessageHandler *pMessageHandler);

  AlgorithmT getAlgorithm();

  void initialize();
  void run();

  void setOptVar(const std::string &var, const std::string &value);
  double getOptVar(const std::string &var, bool &ok);

  void setNumberOfParameters(size_t value);

  void setCrossoverProbability(double value);
  void setMutationProbability(double value);
  void setNumberOfElites(size_t value);

private:
  void selectParents();
  void crossOver();
  void mutate();
  double gaussian(double mean, double dev, double min, double max);

  size_t mParent1,mParent2;
  std::vector<double> mChild1, mChild2;

  double mCrossoverProbability = 0.2;
  double mMutationProbability = 0.1;
  size_t mNumElites = 0;
  std::default_random_engine mRandomGenerator;
};

}

#endif // OPSWORKERGENETIC_H
