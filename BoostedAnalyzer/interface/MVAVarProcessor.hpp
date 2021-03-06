#ifndef BOOSTEDTTH_BOOSTEDANALYZER_MVAVARPROCESSOR_HPP
#define BOOSTEDTTH_BOOSTEDANALYZER_MVAVARPROCESSOR_HPP

#include "BoostedTTH/BoostedAnalyzer/interface/TreeProcessor.hpp"
#include "BoostedTTH/BoostedAnalyzer/interface/BoostedUtils.hpp"
#include "MiniAOD/MiniAODHelper/interface/BDTvars.h"
#include "BoostedTTH/TTHReconstruction/interface/ReconstructionQuality.hpp"

class MVAVarProcessor: public TreeProcessor{
  
public:
  
  MVAVarProcessor();
  ~MVAVarProcessor();
  
  void Init(const InputCollections& input,VariableContainer& var);
  void Process(const InputCollections& input,VariableContainer& var);
private:
  BDTvars bdtvar;
  ReconstructionQuality quality;
};

#endif
