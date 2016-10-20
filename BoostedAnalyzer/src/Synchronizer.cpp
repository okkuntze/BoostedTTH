#include "BoostedTTH/BoostedAnalyzer/interface/Synchronizer.hpp"

using namespace std;

Synchronizer::Synchronizer ():toptagger(TopTag::Likelihood,TopTag::CSV,"toplikelihoodtaggerhistos.root"),bdt3(BDT_v3(BoostedUtils::GetAnalyzerPath()+"/data/bdtweights/weights_v3/")),initializedCutflowsWithSelections(false){
  cutflowSL_nominal.Init();
  cutflowSL_jesup.Init();
  cutflowSL_jesdown.Init();
  cutflowSL_raw.Init();
  cutflowDL_nominal.Init();
  cutflowDL_jesup.Init();
  cutflowDL_jesdown.Init();
  cutflowDL_raw.Init();
}


Synchronizer::~Synchronizer (){
  for(auto f = dumpFiles1.begin(); f!=dumpFiles1.end(); f++){
    (*f)->close();
  }
  for(auto f = dumpFiles2.begin(); f!=dumpFiles2.end(); f++){
    (*f)->close();
  }
  for(auto f = dumpFiles2_jesup.begin(); f!=dumpFiles2_jesup.end(); f++){
    (*f)->close();
  }
  for(auto f = dumpFiles2_jesdown.begin(); f!=dumpFiles2_jesdown.end(); f++){
    (*f)->close();
  }
  for(auto f = dumpFiles2_raw.begin(); f!=dumpFiles2_raw.end(); f++){
    (*f)->close();
  }
  if(cutflowFile!=0){
      cutflowSL_nominal.Print(*cutflowFile);
      cutflowSL_nominal.Print(cout);
      cutflowDL_nominal.Print(*cutflowFile);
      cutflowDL_nominal.Print(cout);
  }
}


void Synchronizer::DumpSyncExe1(int nfile,const InputCollections& input){
  DumpSyncExe1(input,*(dumpFiles1[nfile]));
}


void Synchronizer::DumpSyncExe1(const InputCollections& input, std::ostream &out){
  int run=input.eventInfo.run;
  int lumi=input.eventInfo.lumiBlock;
  long event=input.eventInfo.evt;

  float lep1_pt=-99;
  float lep1_eta=-99;
  float lep1_phi=-99;
  float jet1_pt=-99;
  float jet2_pt=-99;
  float jet3_pt=-99;
  float jet4_pt=-99;
  float jet1_CSVv2=-99;
  float jet2_CSVv2=-99;
  float jet3_CSVv2=-99;
  float jet4_CSVv2=-99;
  float MET=-99;
  int n_jets=0;
  int n_btags=0;
  int ttHFCategory=0;
  int n_toptags=0;
  int n_higgstags=0;

  for(std::vector<pat::Muon>::const_iterator iMuon = input.selectedMuons.begin(), ed = input.selectedMuons.end(); iMuon != ed; ++iMuon ){
    if(iMuon->pt()>lep1_pt){
      lep1_pt=iMuon->pt();
      lep1_eta=iMuon->eta();
      lep1_phi=iMuon->phi();}
  }
  for(std::vector<pat::Electron>::const_iterator iEle = input.selectedElectrons.begin(), ed = input.selectedElectrons.end(); iEle != ed; ++iEle ){
    if(iEle->pt()>lep1_pt){
      lep1_pt=iEle->pt();
      lep1_eta=iEle->eta();
      lep1_phi=iEle->phi();}
  }


  if(input.selectedJets.size()>0){
    jet1_pt=input.selectedJets.at(0).pt();
    jet1_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(0),"pfCombinedInclusiveSecondaryVertexV2BJetTags");
  }

  if(input.selectedJets.size()>1){
    jet2_pt=input.selectedJets.at(1).pt();
    jet2_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(1),"pfCombinedInclusiveSecondaryVertexV2BJetTags");
  }

  if(input.selectedJets.size()>2){
    jet3_pt=input.selectedJets.at(2).pt();
    jet3_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(2),"pfCombinedInclusiveSecondaryVertexV2BJetTags");
  }

  if(input.selectedJets.size()>3){
    jet4_pt=input.selectedJets.at(3).pt();
    jet4_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(3),"pfCombinedInclusiveSecondaryVertexV2BJetTags");
  }
  n_jets=int(input.selectedJets.size());
  for(auto jet=input.selectedJets.begin();jet!=input.selectedJets.end(); jet++){
    if(BoostedUtils::PassesCSV(*jet)) n_btags++;
  }

  vector<boosted::BoostedJet> syncTopJets;
  for(auto topjet = input.selectedBoostedJets.begin() ; topjet != input.selectedBoostedJets.end(); topjet++ ){
    // pt and eta requirements on top jet
    if( !(topjet->fatjet.pt() > 200. && abs(topjet->fatjet.eta()) < 2.) ) continue;
    std::vector<pat::Jet> subjets;
    subjets.push_back(topjet->W1);
    subjets.push_back(topjet->W2);
    subjets.push_back(topjet->nonW);
    bool subjetcuts=true;
    for(auto j = subjets.begin(); j!=subjets.end();j++){
      if(j->pt()<20 || fabs(j->eta())>2.4) {
	subjetcuts=false;
	break;
      }
    }
    if(!subjetcuts) continue;
    if(toptagger.GetTopTaggerOutput(*topjet)>-1){
      n_toptags++;
      syncTopJets.push_back(*topjet);
    }
  }
  for( auto higgsJet = input.selectedBoostedJets.begin() ; higgsJet != input.selectedBoostedJets.end(); ++higgsJet ){
    // pt and eta requirements on higgs jet
    if( !(higgsJet->fatjet.pt() > 200. && abs(higgsJet->fatjet.eta()) < 2) ) continue;
    bool overlapping=false;
    for(auto tj=syncTopJets.begin(); tj!=syncTopJets.end(); tj++){
      if(BoostedUtils::DeltaR(tj->fatjet,higgsJet->fatjet)<1.5){
	overlapping=true;
	break;
      }
    }
    if(overlapping) continue;
    std::vector<pat::Jet> filterjets = higgsJet->filterjets;
    int subjettags=0;
    for(auto j=filterjets.begin(); j!=filterjets.end(); j++ ){
      if(j->pt()<20 || fabs(j->eta())>2.4) continue;
      if(BoostedUtils::PassesCSV(*j)){
	subjettags++;
      }
    }
    if(subjettags>=2) n_higgstags++;

  }

  MET=input.correctedMET.pt();

  ttHFCategory=input.genTopEvt.GetTTxIdFromProducer();

  out << boost::format("%6d %8d %10d   %6.2f %+4.2f %+4.2f   %6.2f %6.2f %6.2f %6.2f   %+7.3f %+7.3f %+7.3f %+7.3f   %+7.3f   %2d  %2d   %2d   %2d  %2d\n")%
	 run% lumi% event%
	 lep1_pt% lep1_eta% lep1_phi%
	 jet1_pt% jet2_pt% jet3_pt% jet4_pt%
	 jet1_CSVv2% jet2_CSVv2% jet3_CSVv2% jet4_CSVv2%
	 MET%
	 n_jets% n_btags%
	 ttHFCategory%
	 n_toptags% n_higgstags;

}
void Synchronizer::DumpSyncExe2HeaderBTagSys(std::ostream &out){
  out <<"run,lumi,event,is_e,is_mu,is_ee,is_emu,is_mumu,n_jets,n_btags,lep1_pt,lep1_iso,lep1_pdgId,lep2_pt,lep2_iso,lep2_pdgId,jet1_pt,jet2_pt,jet1_CSVv2,jet2_CSVv2,jet1_JecSF,jet1_JecSF_up,jet1_JecSF_down,MET_pt,MET_phi,mll,ttHFCategory,PUWeight,bWeight,triggerSF,lepIDSF,lepISOSF,Q2_upup,Q2_downdown,pdf_up,pdf_down,CSVLFup,CSVHFdown,CSVCErr1down\n";
}
void Synchronizer::DumpSyncExe2Header(std::ostream &out){
  out <<"run,lumi,event,is_e,is_mu,is_ee,is_emu,is_mumu,n_jets,n_btags,lep1_pt,lep1_iso,lep1_pdgId,lep2_pt,lep2_iso,lep2_pdgId,jet1_pt,jet2_pt,jet1_CSVv2,jet2_CSVv2,jet1_JecSF,jet1_JecSF_up,jet1_JecSF_down,MET_pt,MET_phi,mll,ttHFCategory,PUWeight,bWeight,triggerSF,lepIDSF,lepISOSF,Q2_upup,Q2_downdown,pdf_up,pdf_down\n";
}

void Synchronizer::DumpSyncExe2(const InputCollections& input,
				const InputCollections& input_DL,
				/*const InputCollections& input_JESUP,
				const InputCollections& input_DL_JESUP,
				const InputCollections& input_JESDOWN,
				const InputCollections& input_DL_JESDOWN,*/
				MiniAODHelper& helper,
				std::ostream &out,Cutflow& cutflowSL,
				Cutflow& cutflowDL,
				int dataset_flag){

  bool runOverData=false;
  bool is_SL=true;
  bool is_DL=true;
  std::string channel="both";
  std::string channel_DL="all";

  if(input.sampleType == SampleType::data) {
    runOverData=true;
    switch(dataset_flag) {
      case 1:
	      channel="el";
	      break;
      case 2:
	      channel="mu";
	      break;
      case 3:
	      channel_DL="elel";
	      break;
      case 4:
	      channel_DL="elmu";
	      break;
      case 5:
	      channel_DL="mumu";
	      break;
    }
  }
  //if(runOverData) {cout << "data" << endl;}
  //else if (!runOverData) {cout << "mc" << endl;}

  // Setup Selections
  // Single Lepton Selection
  vector<string> el_triggers;
  vector<string> mu_triggers;

  if(!runOverData) {
    //el_triggers.push_back("any");
    //mu_triggers.push_back("any");
    el_triggers.push_back("HLT_Ele27_eta2p1_WPTight_Gsf_v*");
    mu_triggers.push_back("HLT_IsoMu22_v*");
    mu_triggers.push_back("HLT_IsoTkMu22_v*");
  }
  else if(runOverData) {
    el_triggers.push_back("HLT_Ele27_eta2p1_WPTight_Gsf_v*");
    mu_triggers.push_back("HLT_IsoMu22_v*");
    mu_triggers.push_back("HLT_IsoTkMu22_v*");
    if(dataset_flag<3) {
      //cout << "is_DL set false" << endl;
      is_DL=false;
    }
    else if(dataset_flag>2) {
      //cout << "is_SL set false" << endl;
      is_SL=false;
    }
  }

  if(leptonSelections.size()==0 && ((!runOverData)||dataset_flag<3)){
    leptonSelections.push_back(new VertexSelection());
    leptonSelections.push_back(new LeptonSelection(el_triggers,mu_triggers,channel));
    leptonSelections.push_back(new JetTagSelection(4,2));

    cout << "SL Selection Step 0: VertexSelection" << endl;
    cout << "SL Selection Step 1: LeptonSelection" << endl;
    cout << "SL Selection Step 2: JetTagSelection" << endl;
  }
  if(!initializedCutflowsWithSelections){
    for(uint i=0; i<leptonSelections.size(); i++){
      leptonSelections[i]->InitCutflow(cutflowSL);
    }
  }

  // Dilepton Selection
  vector<string> elel_triggers;
  vector<string> mumu_triggers;
  vector<string> elmu_triggers;

  // MC triggers ->do not work yet

  if(!runOverData) {
    //elel_triggers.push_back("any");
    //mumu_triggers.push_back("any");
    //elmu_triggers.push_back("any");
    elel_triggers.push_back("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*");
    mumu_triggers.push_back("HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*");
    mumu_triggers.push_back("HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v*");
    elmu_triggers.push_back("HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*");
    elmu_triggers.push_back("HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v*");
  }
  // data triggers
  else {
    elel_triggers.push_back("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*");
    mumu_triggers.push_back("HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*");
    mumu_triggers.push_back("HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v*");
    elmu_triggers.push_back("HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*");
    elmu_triggers.push_back("HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v*");
  }
  

  if(dileptonSelections.size()==0 && ((!runOverData)||dataset_flag>2)){
    dileptonSelections.push_back(new VertexSelection());
    dileptonSelections.push_back(new DiLeptonSelection(elel_triggers,mumu_triggers,elmu_triggers,channel_DL));
    dileptonSelections.push_back(new DiLeptonMassSelection(20,99999,false,true));
    dileptonSelections.push_back(new DiLeptonMassSelection(76,106,true,false));
    dileptonSelections.push_back(new DiLeptonMETSelection(40,99999));
    dileptonSelections.push_back(new DiLeptonJetTagSelection(2,1));

    cout << "DL Selection Step 0: VertexSelection" << endl;
    cout << "DL Selection Step 1: DiLeptonSelection" << endl;
    cout << "DL Selection Step 2: DiLeptonMassSelection 20 GeV cut" << endl;
    cout << "DL Selection Step 3: DiLeptonMassSelection Z Veto" << endl;
    cout << "DL Selection Step 4: DiLeptonMETSelection" << endl;
    cout << "DL Selection Step 5: DiLeptonJetTagSelection" << endl;
  }

  //std::cout << "channel: " << channel << " channel_DL: " << channel_DL << endl;

  if(!initializedCutflowsWithSelections){
    for(uint i=0; i<dileptonSelections.size(); i++){
      dileptonSelections[i]->InitCutflow(cutflowDL);
    }
  }

  //dummy selection for dl flag
  if(dileptonSelection.size()==0){
    dileptonSelection.push_back(new DiLeptonSelection(elel_triggers,mumu_triggers,elmu_triggers));
  }
  if(!initializedCutflowsWithSelections){
    dummycutflow_DL.push_back(Cutflow());
    dummycutflow_DL.back().Init();
    dileptonSelection.back()->InitCutflow(dummycutflow_DL.back());
  }

  //dummy selection for Mll flag
  if(dileptonMllSelections.size()==0){
    dileptonMllSelections.push_back(new DiLeptonMassSelection(20,99999,false,true));
    dileptonMllSelections.push_back(new DiLeptonMassSelection(76,106,true,false));
  }
  if(!initializedCutflowsWithSelections){
    dummycutflow_Mll.push_back(Cutflow());
    dummycutflow_Mll.back().Init();
    for(uint i=0; i<dileptonMllSelections.size(); i++){
      dileptonMllSelections[i]->InitCutflow(dummycutflow_Mll.back());
    }
  }

  //dummy selection for MET flag
  if(dileptonMETSelection.size()==0){
    dileptonMETSelection.push_back(new DiLeptonMETSelection(40,99999));
  }
  if(!initializedCutflowsWithSelections){
    dummycutflow_MET.push_back(Cutflow());
    dummycutflow_MET.back().Init();
    dileptonMETSelection.back()->InitCutflow(dummycutflow_MET.back());
  }
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// Declare Variables //////////////////////////////////////

  bool BTagSystematics = false;

  /*bool is_ttjets=1;
  float xs_ttbar= 831.76 ;// in pb
  float xs_ttH=0.2918;
  float Ngen_ttjets=46400.;
  float Ngen_ttH=49894.;*/
  int run=input.eventInfo.run;
  int lumi=input.eventInfo.lumiBlock;
  long event=input.eventInfo.evt;




  float lep1_pt=-1;
  float lep1_eta=-1;
  float lep1_phi=-1;
  float lep1_iso=-1;
  float lep1_pdgId=0;
  float lep1_MVAID=-1;

  float lep2_pt=-1;
  float lep2_eta=-1;
  float lep2_phi=-1;
  float lep2_iso=-1;
  float lep2_pdgId=0;
  float lep2_MVAID=-1;

  float jet1_pt=-1;
  float jet2_pt=-1;
  float jet3_pt=-1;
  float jet4_pt=-1;

  float jet1_CSVv2=-1;
  float jet2_CSVv2=-1;
  float jet3_CSVv2=-1;
  float jet4_CSVv2=-1;

  float MET_pt=-1;
  float MET_phi=-1;

  int n_jets=0;
  int n_btags=0;
  //float mcweight=-1;
  float bWeight=-1;
  float triggerSF=-1;
  /*if(is_ttjets && !runOverData) {
    mcweight=xs_ttbar*1000*2.7/Ngen_ttjets;//because powheg pythia file->no negative event weights. Normalized to luminosity of 2.7 fb-1
  }
  else if(!is_ttjets && !runOverData) {
    mcweight=xs_ttH*1000*2.7/Ngen_ttH;
  }*/
  //float topweight=-1;
  //float lepSF=-1;
  float lepSFid=-1;
  float lepSFiso=-1;

  float puweight=-1;
  float q2upup=-1;
  float q2downdown=-1;
  float pdfup=-1;
  float pdfdown=-1;

  //CSV systematics
  float Weight_CSVLFup = -99.;
  //float Weight_CSVLFdown = -99.;
//  float Weight_CSVHFup = -99.;
  float Weight_CSVHFdown = -99.;
  //float Weight_CSVHFStats1up = -99.;
  //float Weight_CSVHFStats1down = -99.;
/*  float Weight_CSVLFStats1up = -99.;
  float Weight_CSVLFStats1down = -99.;
  float Weight_CSVHFStats2up = -99.;
  float Weight_CSVHFStats2down = -99.;
  float Weight_CSVLFStats2up = -99.;
  float Weight_CSVLFStats2down = -99.;
  float Weight_CSVCErr1up = -99.;*/
  float Weight_CSVCErr1down = -99.;
  //float Weight_CSVCErr2up = -99.;
  //float Weight_CSVCErr2down = -99.;


  int ttHFCategory=-1;

  float final_discriminant1=0;
  float final_discriminant2=0;

  int n_fatjets=0;
  float pt_fatjet_1=-1;
  float pt_fatjet_2=-1;

  float pt_nonW_1=0;
  float pt_nonW_2=0;
  float pt_W1_1=0;
  float pt_W1_2=0;
  float pt_W2_1=0;
  float pt_W2_2=0;
  float pt_top_1=0;
  float pt_top_2=0;
  float m_top_1=0;
  float m_top_2=0;

  float higgstag_fatjet_1=0;
  float higgstag_fatjet_2=0;
  float csv2_fatjet_1=0;
  float csv2_fatjet_2=0;

  bool dl_passed=false;
  bool mll_passed=false;
  bool met_passed=false;


  float mll=-1;

  int is_e=0;
  int is_mu=0;
  int is_ee=0;
  int is_emu=0;
  int is_mumu=0;


  float jet1_JecSF = 0;
  float jet1_JecSF_up = 0;
  float jet1_JecSF_down = 0;



  bool compare = false;
  /*if(event==3875954 || int(event)==3875954 || event==3897814 || int(event)==3897814) {
    cout << "####################################################### event " << event << " #############################" << endl;
    compare=true;
  }*/

  /*
  const int nEntries = 6;
  int comparisonList[] = {324990, 904458,2259390, 2844708,277215,1059298};

  for(int i = 0;i<nEntries;i++){
    if(event == comparisonList[i]){
      compare = true;
      break;
    }
  }
  */

  if(compare) std::cout << "Event: " << event << std::endl;

  cutflowSL.EventSurvivedStep("all",input.weights.at("Weight"));
  for(uint i=0; i<leptonSelections.size(); i++){
    //cout << "check SL selection " << i << endl;
    if(!leptonSelections[i]->IsSelected(input,cutflowSL)){
      if(compare) cout << "Event failed SL Selection at step " << i << endl;
	    is_SL=false;
	    break;
    }
  }

  cutflowDL.EventSurvivedStep("all",input_DL.weights.at("Weight"));
  for(uint i=0; i<dileptonSelections.size(); i++){
    //cout << "check DL selections " << i << endl;
    if(!dileptonSelections[i]->IsSelected(input_DL,cutflowDL)){
      if(compare) cout << "Event failed DL Selection at step " << i << endl;
	    is_DL=false;
	    break;
    }
  }

  if(compare) std::cout << "is_SL: " << is_SL  << "   is_DL: " << is_DL<< std::endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// calculate particle quantities /////////////////////////////////////////////

  if(is_SL) {
    for(std::vector<pat::Muon>::const_iterator iMuon = input.selectedMuonsLoose.begin(); iMuon != input.selectedMuonsLoose.end(); ++iMuon ){
      if(iMuon->pt()>lep1_pt){
	lep2_pt=lep1_pt;
	lep2_eta=lep1_eta;
	lep2_phi=lep1_phi;
	lep2_iso=lep1_iso;
	lep2_pdgId=lep1_pdgId;

	lep1_pt=iMuon->pt();
	lep1_eta=iMuon->eta();
	lep1_phi=iMuon->phi();
	lep1_iso=helper.GetMuonRelIso(*iMuon,coneSize::R04, corrType::deltaBeta);
	lep1_pdgId=iMuon->pdgId();
      }
      else if(iMuon->pt()>lep2_pt){
	lep2_pt=iMuon->pt();
	lep2_eta=iMuon->eta();
	lep2_phi=iMuon->phi();
	lep2_iso=helper.GetMuonRelIso(*iMuon,coneSize::R04, corrType::deltaBeta);
	lep2_pdgId=iMuon->pdgId();
      }
    }
    for(std::vector<pat::Electron>::const_iterator iEle = input.selectedElectronsLoose.begin(); iEle != input.selectedElectronsLoose.end(); ++iEle ){
      if(iEle->pt()>lep1_pt){
	lep2_pt=lep1_pt;
	lep2_eta=lep1_eta;
	lep2_phi=lep1_phi;
	lep2_iso=lep1_iso;
	lep2_pdgId=lep1_pdgId;
	lep2_MVAID=lep1_MVAID;

	lep1_pt=iEle->pt();
	lep1_eta=iEle->eta();
	lep1_phi=iEle->phi();
	lep1_iso=helper.GetElectronRelIso(*iEle, coneSize::R03, corrType::rhoEA,effAreaType::spring15);
	lep1_pdgId=iEle->pdgId();
	lep1_MVAID=iEle->userFloat("mvaValue");
      }
      else if(iEle->pt()>lep2_pt){
	lep2_pt=iEle->pt();
	lep2_eta=iEle->eta();
	lep2_phi=iEle->phi();
	lep2_iso=helper.GetElectronRelIso(*iEle, coneSize::R03, corrType::rhoEA,effAreaType::spring15);
	lep2_pdgId=iEle->pdgId();
	lep2_MVAID=iEle->userFloat("mvaValue");
      }
    }
  }
  if(is_DL) {
    for(std::vector<pat::Muon>::const_iterator iMuon = input.selectedMuonsLoose.begin(); iMuon != input.selectedMuonsLoose.end(); ++iMuon ){
      if(iMuon->pt()>lep1_pt){
	lep2_pt=lep1_pt;
	lep2_eta=lep1_eta;
	lep2_phi=lep1_phi;
	lep2_iso=lep1_iso;
	lep2_pdgId=lep1_pdgId;

	lep1_pt=iMuon->pt();
	lep1_eta=iMuon->eta();
	lep1_phi=iMuon->phi();
	lep1_iso=helper.GetMuonRelIso(*iMuon,coneSize::R04, corrType::deltaBeta);
	lep1_pdgId=iMuon->pdgId();
      }
      else if(iMuon->pt()>lep2_pt){
	lep2_pt=iMuon->pt();
	lep2_eta=iMuon->eta();
	lep2_phi=iMuon->phi();
	lep2_iso=helper.GetMuonRelIso(*iMuon,coneSize::R04, corrType::deltaBeta);
	lep2_pdgId=iMuon->pdgId();
      }
    }
    for(std::vector<pat::Electron>::const_iterator iEle = input.selectedElectronsLoose.begin(); iEle != input.selectedElectronsLoose.end(); ++iEle ){
      if(iEle->pt()>lep1_pt){
	lep2_pt=lep1_pt;
	lep2_eta=lep1_eta;
	lep2_phi=lep1_phi;
	lep2_iso=lep1_iso;
	lep2_pdgId=lep1_pdgId;
	lep2_MVAID=lep1_MVAID;

	lep1_pt=iEle->pt();
	lep1_eta=iEle->eta();
	lep1_phi=iEle->phi();
	lep1_iso=helper.GetElectronRelIso(*iEle, coneSize::R03, corrType::rhoEA,effAreaType::spring15);
	lep1_pdgId=iEle->pdgId();
	lep1_MVAID=iEle->userFloat("mvaValue");
      }
      else if(iEle->pt()>lep2_pt){
	lep2_pt=iEle->pt();
	lep2_eta=iEle->eta();
	lep2_phi=iEle->phi();
	lep2_iso=helper.GetElectronRelIso(*iEle, coneSize::R03, corrType::rhoEA,effAreaType::spring15);
	lep2_pdgId=iEle->pdgId();
	lep2_MVAID=iEle->userFloat("mvaValue");
      }
    }
  }

  if(is_SL){
    if(abs(lep1_pdgId)==11){
      is_e=1;
    }
    if(abs(lep1_pdgId)==13){
      is_mu=1;
    }
  }
  if(is_DL){
    if(abs(lep1_pdgId)==11){
      if(abs(lep2_pdgId)==13){
        is_emu=1;
      }
      if(abs(lep2_pdgId)==11){
        is_ee=1;
      }
    }
    if(abs(lep1_pdgId)==13){
      if(abs(lep2_pdgId)==11){
        is_emu=1;
      }
      if(abs(lep2_pdgId)==13){
        is_mumu=1;
      }
    }
  }

  //i hate scram unsused variable error !!!!11111 +1
  if(compare){
    std::cout<<"MVAIDs "<<lep1_MVAID<<" "<<lep2_MVAID<<std::endl;
  }


  if(is_DL){
    if(input_DL.selectedJetsLooseDL.size()>0){
      jet1_pt=input_DL.selectedJetsLooseDL.at(0).pt();
      jet1_CSVv2=MiniAODHelper::GetJetCSV(input_DL.selectedJetsLooseDL.at(0));
      bool jetmatched = false;
      for( auto rawJet: input_DL.rawJets){
	if( BoostedUtils::DeltaR(rawJet.p4(),input_DL.selectedJetsLooseDL.at(0).p4()) < 0.01 ){
	    //double jet1_JES = helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, input.genJets, sysType::NA,true,false) ;
	    double jet1_JES = 1.0;
	  //float jet1_JER = helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, sysType::NA,false,true) ;
	  


	    //	  double JESup =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, input.genJets, sysType::JESup,true,false)/jet1_JES ;
	     double JESup =  1.0 ;
	  //float JERup =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, sysType::JERup,false,true) ;
	    //double JESdown =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, input.genJets, sysType::JESdown,true,false)/jet1_JES;
	     double JESdown =   1.0;
	  //float JERdown =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, sysType::JERdown,false,true);

	  jet1_JecSF = jet1_JES;
	  jet1_JecSF_up = JESup;
	  jet1_JecSF_down = JESdown;


	  jetmatched = true;
	}
      }
      if ( !jetmatched ){
	jet1_JecSF = -1;
	jet1_JecSF_up = -1;
	jet1_JecSF_down = -1;
      }
    }

    if(input_DL.selectedJetsLooseDL.size()>1){
      jet2_pt=input_DL.selectedJetsLooseDL.at(1).pt();
      jet2_CSVv2=MiniAODHelper::GetJetCSV(input_DL.selectedJetsLooseDL.at(1));
    }

    if(input_DL.selectedJetsLooseDL.size()>2){
      jet3_pt=input_DL.selectedJetsLooseDL.at(2).pt();
      jet3_CSVv2=MiniAODHelper::GetJetCSV(input_DL.selectedJetsLooseDL.at(2));
    }

    if(input_DL.selectedJetsLooseDL.size()>3){
      jet4_pt=input_DL.selectedJetsLooseDL.at(3).pt();
      jet4_CSVv2=MiniAODHelper::GetJetCSV(input_DL.selectedJetsLooseDL.at(3));
    }
    n_jets=int(input_DL.selectedJetsLooseDL.size());
    for(auto jet=input_DL.selectedJetsLooseDL.begin();jet!=input_DL.selectedJetsLooseDL.end(); jet++){
      if(helper.PassesCSV(*jet,'M')) n_btags++;
    }
  }
  else if(is_SL) {
    if(input.selectedJets.size()>0){
      jet1_pt=input.selectedJets.at(0).pt();
      jet1_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(0));
      bool jetmatched = false;
      for( auto rawJet: input.rawJets){
	if( BoostedUtils::DeltaR(rawJet.p4(),input.selectedJets.at(0).p4()) < 0.01 ){
	    //double jet1_JES = helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, input.genJets, sysType::NA,true,false) ;
	    double jet1_JES = 1.0;
	  //float jet1_JER = helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, sysType::NA,false,true) ;
	  


	  //double JESup =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, input.genJets, sysType::JESup,true,false)/jet1_JES ;
	  double JESup =   1.0;
	  //float JERup =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, sysType::JERup,false,true) ;
	  //double JESdown =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, input.genJets, sysType::JESdown,true,false)/jet1_JES;
	  double JESdown =  1.0;
	  //float JERdown =  helper.GetJetCorrectionFactor(rawJet,input.iEvent, input.iSetup, sysType::JERdown,false,true);

	  jet1_JecSF = jet1_JES;
	  jet1_JecSF_up = JESup;
	  jet1_JecSF_down = JESdown;

	  jetmatched = true;
	}
      }
      if ( !jetmatched ){
	jet1_JecSF = -1;
	jet1_JecSF_up = -1;
	jet1_JecSF_down = -1;
      }
    }

    if(input.selectedJets.size()>1){
      jet2_pt=input.selectedJets.at(1).pt();
      jet2_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(1));
    }

    if(input.selectedJets.size()>2){
      jet3_pt=input.selectedJets.at(2).pt();
      jet3_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(2));
    }

    if(input.selectedJets.size()>3){
      jet4_pt=input.selectedJets.at(3).pt();
      jet4_CSVv2=MiniAODHelper::GetJetCSV(input.selectedJets.at(3));
    }
    n_jets=int(input.selectedJets.size());
    for(auto jet=input.selectedJets.begin();jet!=input.selectedJets.end(); jet++){
      if(helper.PassesCSV(*jet,'M')) n_btags++;
    }
  }
  
  /*if(event==3875954 || int(event)==3875954 || event==3897814 || int(event)==3897814) {
    for(size_t i=0;i<input.selectedJetsLooseDL.size();i++){
      cout << "############# Jet " << i << " ##############" << endl;
      cout << "Pt: " << input.selectedJetsLoose.at(i).pt() << endl;
      cout << "Eta: " << input.selectedJetsLoose.at(i).eta() << endl;
      cout << "JER? " << helper.jetdPtMatched(input.selectedJetsLoose.at(i)) << endl;
      //cout << "Jet CSV: " << MiniAODHelper::GetJetCSV(input.selectedJetsLooseDL.at(i)) << endl;
    }
  } */

  // get selection flags
  // dilepton
  dl_passed=dileptonSelection[0]->IsSelected(input_DL,dummycutflow_DL[0]);

  // dilepton mass
  mll_passed=dileptonMllSelections[0]->IsSelected(input_DL,dummycutflow_Mll[0]) && dileptonMllSelections[1]->IsSelected(input_DL,dummycutflow_Mll[0]);

  // MET
  met_passed=dileptonMETSelection[0]->IsSelected(input_DL,dummycutflow_MET[0]);

  if(compare) std::cout << "dl_passed: " << dl_passed << "   mll_passed: " << mll_passed << "   met_passed: " << met_passed << std::endl;


  //calculate mll
  if(dl_passed){
    math::XYZTLorentzVector vec1;
    math::XYZTLorentzVector vec2;
    bool calculateMll=false;
    if(input.selectedMuonsLoose.size()==2 && input.selectedElectronsLoose.size()==0){
      vec1=input.selectedMuonsLoose[0].p4();
      vec2=input.selectedMuonsLoose[1].p4();
      calculateMll=true;
    }
    else if(input.selectedMuonsLoose.size()==0 && input.selectedElectronsLoose.size()==2){
      vec1=input.selectedElectronsLoose[0].p4();
      vec2=input.selectedElectronsLoose[1].p4();
      calculateMll=true;
    }
    else if(input.selectedMuonsLoose.size()==1 && input.selectedElectronsLoose.size()==1){
      vec1=input.selectedMuonsLoose[0].p4();
      vec2=input.selectedElectronsLoose[0].p4();
      calculateMll=true;
    }
    else {
      std::cout<<"PROBLEM we have !=2 leptons in DiLeptonSelection"<<std::endl;
    }

    if(calculateMll){
      mll=(vec1+vec2).M();
    }
  }

  MET_pt=input.correctedMET.pt();
  MET_phi=input.correctedMET.phi();

  if(is_SL&&( (n_jets>=4&&n_btags>=3) || (n_jets>=6&&n_btags>=2))){
    final_discriminant1=bdt3.Evaluate(input.selectedMuons,input.selectedElectrons, input.selectedJets, input.selectedJetsLoose, input.correctedMET);
  }

  /////////////////////////////////////////////////////////////////////////// BOOSTED STUFF /////////////////////////////////////////////////////////////////////////////////////////////

  n_fatjets = int(input.selectedBoostedJets.size());
  if(input.selectedBoostedJets.size()>0){
    pt_fatjet_1=input.selectedBoostedJets.at(0).fatjet.pt();
    pt_nonW_1=input.selectedBoostedJets.at(0).nonW.pt();
    pt_W1_1=input.selectedBoostedJets.at(0).W1.pt();
    pt_W2_1=input.selectedBoostedJets.at(0).W2.pt();
    pt_top_1=input.selectedBoostedJets.at(0).topjet.mass();
    m_top_1=input.selectedBoostedJets.at(0).topjet.mass();
  }

  if(input.selectedBoostedJets.size()>1){
    pt_fatjet_2=input.selectedBoostedJets.at(1).fatjet.pt();
    pt_nonW_2=input.selectedBoostedJets.at(1).nonW.pt();
    pt_W1_2=input.selectedBoostedJets.at(1).W1.pt();
    pt_W2_2=input.selectedBoostedJets.at(1).W2.pt();
    pt_top_2=input.selectedBoostedJets.at(1).topjet.mass();
    m_top_2=input.selectedBoostedJets.at(1).topjet.mass();
  }

  if(input.selectedBoostedJets.size()>0){

    higgstag_fatjet_1 = input.selectedBoostedJets.at(0).fatjet.bDiscriminator("pfBoostedDoubleSecondaryVertexCA15BJetTags");

    if(input.selectedBoostedJets.at(0).filterjets.size()>1){
      std::vector<pat::Jet> filterjets = BoostedUtils::GetHiggsFilterJets(input.selectedBoostedJets.at(0));
      csv2_fatjet_1 = MiniAODHelper::GetJetCSV(filterjets.at(1));
    }
  }

  if(input.selectedBoostedJets.size()>1){

    higgstag_fatjet_2 = input.selectedBoostedJets.at(1).fatjet.bDiscriminator("pfBoostedDoubleSecondaryVertexCA15BJetTags");

    if(input.selectedBoostedJets.at(1).filterjets.size()>1){
      std::vector<pat::Jet> filterjets = BoostedUtils::GetHiggsFilterJets(input.selectedBoostedJets.at(1));
      csv2_fatjet_2 = MiniAODHelper::GetJetCSV(filterjets.at(1));
    }
  }

  if(is_DL && !runOverData){
    bWeight=input_DL.weightsDL.at("Weight_CSV");
    //if(is_ttjets) {
      //topweight=input_DL.weightsDL.at("Weight_TopPt");
    //}
    puweight=input_DL.weightsDL.at("Weight_PU");
    //mcweight = mcweight * input_DL.weightsDL.at("Weight_CT14nlo13100_nominal");
    Weight_CSVLFup = input_DL.weightsDL.at("Weight_CSVLFup");
    //Weight_CSVLFdown = input_DL.weightsDL.at("Weight_CSVLFdown");
    //Weight_CSVHFup = input_DL.weightsDL.at("Weight_CSVHFup");
    Weight_CSVHFdown = input_DL.weightsDL.at("Weight_CSVHFdown");
    /*Weight_CSVHFStats1up  = input_DL.weightsDL.at("Weight_CSVHFStats1up");
    Weight_CSVHFStats1down = input_DL.weightsDL.at("Weight_CSVHFStats1down");
    Weight_CSVLFStats1up = input_DL.weightsDL.at("Weight_CSVLFStats1up");
    Weight_CSVLFStats1down = input_DL.weightsDL.at("Weight_CSVLFStats1down");
    Weight_CSVHFStats2up = input_DL.weightsDL.at("Weight_CSVHFStats2up");
    Weight_CSVHFStats2down = input_DL.weightsDL.at("Weight_CSVHFStats2down");
    Weight_CSVLFStats2up = input_DL.weightsDL.at("Weight_CSVLFStats2up");
    Weight_CSVLFStats2down = input_DL.weightsDL.at("Weight_CSVLFStats2down");
    Weight_CSVCErr1up = input_DL.weightsDL.at("Weight_CSVCErr1up");*/
    Weight_CSVCErr1down = input_DL.weightsDL.at("Weight_CSVCErr1down");
    //Weight_CSVCErr2up = input_DL.weightsDL.at("Weight_CSVCErr2up");
  //  Weight_CSVCErr2down = input_DL.weightsDL.at("Weight_CSVCErr2down");

    q2upup=input_DL.weightsDL.at("Weight_muRupmuFup");
    q2downdown=input_DL.weightsDL.at("Weight_muRdownmuFdown");
    //pdfup=input_DL.weightsDL.at("Weight_NNPDFid260067");
    pdfup=input_DL.weightsDL.at("Weight_NNPDF30_nlo_as_0118260000_up");
    //pdfdown=input_DL.weightsDL.at("Weight_NNPDFid260005");
    pdfdown=input_DL.weightsDL.at("Weight_NNPDF30_nlo_as_0118260000_down");
    //lepSF=input_DL.weightsDL.at("Weight_LeptonSF");
    lepSFid=input_DL.weightsDL.at("Weight_ElectronSFID")*input_DL.weightsDL.at("Weight_MuonSFID");
    lepSFiso=input_DL.weightsDL.at("Weight_ElectronSFIso")*input_DL.weightsDL.at("Weight_MuonSFIso");
    if(is_ee) {triggerSF=input_DL.weightsDL.at("Weight_ElectronElectronTriggerSF");}
    if(is_emu) {triggerSF=input_DL.weightsDL.at("Weight_ElectronMuonTriggerSF");}
    if(is_mumu) {triggerSF=input_DL.weightsDL.at("Weight_MuonMuonTriggerSF");}
    
    

    ttHFCategory=input_DL.genTopEvt.GetTTxIdFromProducer();
  }
  else if(is_SL && !runOverData){
    bWeight=input.weights.at("Weight_CSV");
    //if(is_ttjets) {
    //  topweight=input.weights.at("Weight_TopPt");
    //}
    puweight=input.weights.at("Weight_PU");
    //mcweight = mcweight * input.weights.at("Weight_CT14nlo13100_nominal");
    Weight_CSVLFup = input.weights.at("Weight_CSVLFup");
  //  Weight_CSVLFdown = input.weights.at("Weight_CSVLFdown");
  //  Weight_CSVHFup = input.weights.at("Weight_CSVHFup");
    Weight_CSVHFdown = input.weights.at("Weight_CSVHFdown");
/*    Weight_CSVHFStats1up  = input.weights.at("Weight_CSVHFStats1up");
    Weight_CSVHFStats1down = input.weights.at("Weight_CSVHFStats1down");
    Weight_CSVLFStats1up = input.weights.at("Weight_CSVLFStats1up");
    Weight_CSVLFStats1down = input.weights.at("Weight_CSVLFStats1down");
    Weight_CSVHFStats2up = input.weights.at("Weight_CSVHFStats2up");
    Weight_CSVHFStats2down = input.weights.at("Weight_CSVHFStats2down");
    Weight_CSVLFStats2up = input.weights.at("Weight_CSVLFStats2up");
    Weight_CSVLFStats2down = input.weights.at("Weight_CSVLFStats2down");
    Weight_CSVCErr1up = input.weights.at("Weight_CSVCErr1up");*/
    Weight_CSVCErr1down = input.weights.at("Weight_CSVCErr1down");
    //Weight_CSVCErr2up = input.weights.at("Weight_CSVCErr2up");
    //Weight_CSVCErr2down = input.weights.at("Weight_CSVCErr2down");


    q2upup=input.weights.at("Weight_muRupmuFup");
    q2downdown=input.weights.at("Weight_muRdownmuFdown");

    //pdfup=input.weights.at("Weight_NNPDFid260067");
    pdfup=input.weights.at("Weight_NNPDF30_nlo_as_0118260000_up");
    //pdfdown=input.weights.at("Weight_NNPDFid260005");
    pdfdown=input.weights.at("Weight_NNPDF30_nlo_as_0118260000_down");

    //lepSF=input.weights.at("Weight_LeptonSF");
    lepSFid=input.weights.at("Weight_ElectronSFID")*input.weights.at("Weight_MuonSFID");
    lepSFiso=input.weights.at("Weight_ElectronSFIso")*input.weights.at("Weight_MuonSFIso");
    triggerSF=input.weights.at("Weight_MuonSFTrigger")*input.weights.at("Weight_ElectronSFTrigger");
    
    

    ttHFCategory=input.genTopEvt.GetTTxIdFromProducer();
  }

  if(compare) cout <<run<<","<<lumi<<","<<event<<","<<is_SL<<","<<is_DL<<","
	  <<lep1_pt<<","<<lep1_eta<<","<<lep1_phi<<","<<lep1_iso<<","<<lep1_pdgId<<","<<lep2_pt<<","<<lep2_eta<<","<<lep2_phi<<","<<lep2_iso<<","<<lep2_pdgId<<","<<mll<<","<<mll_passed<<","
	  <<jet1_pt<<","<<jet2_pt<<","<<jet3_pt<<","<<jet4_pt<<","
	  <<jet1_CSVv2<<","<<jet2_CSVv2<<","<<jet3_CSVv2<<","<<jet4_CSVv2<<","
	  <<MET_pt<<","<<MET_phi<<","<<met_passed<<","<<n_jets<<","<<n_btags<<","
	  <<bWeight<<","<<ttHFCategory<<","
	  <<final_discriminant1<<","<< final_discriminant2<<","
	  <<n_fatjets<<","<< pt_fatjet_1<<","<< pt_fatjet_2<<","
	  << pt_nonW_1<<","<< pt_nonW_2<<","
	  <<pt_W1_1<<","<< pt_W1_2<<","
	  <<pt_W2_1<<","<< pt_W2_2<<","
    <<pt_top_1<<","<< pt_top_2<<","
	  <<m_top_1<<","<< m_top_2<<","
    <<higgstag_fatjet_1<<","<< higgstag_fatjet_2 <<","
	  <<csv2_fatjet_1<<","<< csv2_fatjet_2 << "\n";
/* Pre 2016 spring sync output
  out <<run<<", "<<lumi<<","<<event<<","<<is_SL<<","<<is_DL<<","
	<<lep1_pt<<","<<lep1_eta<<","<<lep1_phi<<","<<lep1_iso<<","<<lep1_pdgId<<","<<lep2_pt<<","<<lep2_eta<<","<<lep2_phi<<","<<lep2_iso<<","<<lep2_pdgId<<","<<mll<<","<<mll_passed<<","
	<<jet1_pt<<","<<jet2_pt<<","<<jet3_pt<<","<<jet4_pt<<","
	<<jet1_CSVv2<<","<<jet2_CSVv2<<","<<jet3_CSVv2<<","<<jet4_CSVv2<<","
	<<MET_pt<<","<<MET_phi<<","<<met_passed<<","<<n_jets<<","<<n_btags<<","
	<<bWeight<<","<<ttHFCategory<<","
	<<final_discriminant1<<","<< final_discriminant2<<","
	<<n_fatjets<<","<< pt_fatjet_1<<","<< pt_fatjet_2<<","
	<< pt_nonW_1<<","<< pt_nonW_2<<","
	<<pt_W1_1<<","<< pt_W1_2<<","
	<<pt_W2_1<<","<< pt_W2_2<<","
  <<pt_top_1<<","<< pt_top_2<<","
	<<m_top_1<<","<< m_top_2<<","
  <<higgstag_fatjet_1<<","<< higgstag_fatjet_2 <<","
	<<csv2_fatjet_1<<","<< csv2_fatjet_2 << "\n";*/
//Sync spring 2016 output
  if(is_DL||is_SL) {
    if(BTagSystematics){

      out << boost::format("%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%.4f,%.4f,%i,%.4f,%.4f,%i,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%i,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n")%

	    run% lumi% event%
	    is_e% is_mu% is_ee% is_emu% is_mumu%
	    n_jets% n_btags%
	    lep1_pt% lep1_iso% lep1_pdgId%
	    lep2_pt% lep2_iso% lep2_pdgId%
	    jet1_pt% jet2_pt%
	    jet1_CSVv2% jet2_CSVv2%
	    jet1_JecSF% jet1_JecSF_up% jet1_JecSF_down%
	    MET_pt% MET_phi% mll%
	    ttHFCategory%
	    //mcweight%
	    puweight%
	    bWeight%
	    //topweight%
	    triggerSF%
	    lepSFid%
	    lepSFiso%
	    q2upup% q2downdown%
	    pdfup% pdfdown%
      Weight_CSVLFup% Weight_CSVHFdown%
      Weight_CSVCErr1down;
    }
    if(!BTagSystematics){

    out << boost::format("%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%.4f,%.4f,%i,%.4f,%.4f,%i,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%i,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n")%

	   run% lumi% event%
	   is_e% is_mu% is_ee% is_emu% is_mumu%
	   n_jets% n_btags%
	   lep1_pt% lep1_iso% lep1_pdgId%
	   lep2_pt% lep2_iso% lep2_pdgId%
	   jet1_pt% jet2_pt%
	   jet1_CSVv2% jet2_CSVv2%
	   jet1_JecSF% jet1_JecSF_up% jet1_JecSF_down%
	   MET_pt% MET_phi% mll%
	   ttHFCategory%
	   //mcweight%
	   puweight%
	   bWeight%
	   //topweight%
	   triggerSF%
	    lepSFid%
	    lepSFiso%
	    q2upup% q2downdown%
	    pdfup% pdfdown;
    }
    //cout << "event " << event << " written in csv" << endl;
  }
}


void Synchronizer::DumpSyncExe2(int nfile,const InputCollections& input, const InputCollections& input_jesup, const InputCollections& input_jesdown, const InputCollections& input_raw,const InputCollections& input_DL, const InputCollections& input_DL_jesup, const InputCollections& input_DL_jesdown, const InputCollections& input_DL_raw, MiniAODHelper& helper,int dataset_flag){
  DumpSyncExe2(input,input_DL,helper,*(dumpFiles2[nfile]),cutflowSL_nominal,cutflowDL_nominal,dataset_flag);
  //DumpSyncExe2(input_jesup,input_DL_jesup,helper,*(dumpFiles2_jesup[nfile]),cutflowSL_jesup,cutflowDL_jesup,1);
  //DumpSyncExe2(input_jesdown,input_DL_jesdown,helper,*(dumpFiles2_jesdown[nfile]),cutflowSL_jesdown,cutflowDL_jesdown,2);
  //DumpSyncExe2(input_raw,input_DL_raw,helper,*(dumpFiles2_raw[nfile]),cutflowSL_raw,cutflowDL_raw,3);
  initializedCutflowsWithSelections=true;
}

void Synchronizer::InitDumpSyncFile1(std::string filename){
    cutflowFile = new ofstream((filename+"-cutflow.log").c_str());
    dumpFiles1.push_back(new ofstream((filename+".txt").c_str()));
}

void Synchronizer::InitDumpSyncFile2(std::string filename, bool BTagSystematics){
    cutflowFile = new ofstream((filename+"-cutflow.log").c_str());
    dumpFiles2.push_back(new ofstream((filename+".csv").c_str()));
    //dumpFiles2_jesup.push_back(new ofstream((filename+"_JESup.csv").c_str()));
    //dumpFiles2_jesdown.push_back(new ofstream((filename+"_JESdown.csv").c_str()));
    //dumpFiles2_raw.push_back(new ofstream((filename+"_raw.csv").c_str()));
    if(BTagSystematics){
      DumpSyncExe2HeaderBTagSys(*(dumpFiles2.back()));
    }
    else{
      DumpSyncExe2Header(*(dumpFiles2.back()));
    }
    //DumpSyncExe2Header(*(dumpFiles2_jesup.back()));
    //DumpSyncExe2Header(*(dumpFiles2_jesdown.back()));
    //DumpSyncExe2Header(*(dumpFiles2_raw.back()));
}
