import FWCore.ParameterSet.Config as cms

SynchSelection = cms.PSet(
)

LeptonSelection = cms.PSet(
   #muonTriggers = cms.vstring("HLT_IsoMu24_eta2p1_v"),
  #electronTriggers = cms.vstring("HLT_Ele27_WP80_v")
   muonTriggers = cms.vstring("HLT_IsoMu24_eta2p1_IterTrk02_v1"),
   electronTriggers = cms.vstring("HLT_Ele27_eta2p1_WP85_Gsf_v1")
)

LeptonSelectionNoTrigger = cms.PSet(
    muonTriggers = cms.vstring("None"),
    electronTriggers = cms.vstring("None")
)

JetTagSelection = cms.PSet(
    minJets = cms.vint32(4,6),
    maxJets = cms.vint32(-1),
    minTags = cms.vint32(3,2),
    maxTags = cms.vint32(-1)
)
