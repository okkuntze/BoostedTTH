import FWCore.ParameterSet.Config as cms
isData=True

# input
process = cms.Process("p")
process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring('root://xrootd-cms.infn.it//store/data/Run2016B/SingleElectron/MINIAOD/PromptReco-v2/000/274/998/00000/009D45F8-5E33-E611-AA50-02163E011EA5.root')
                            #fileNames = cms.untracked.vstring('file:/pnfs/desy.de/cms/tier2/store/mc/RunIISpring15DR74/TT_TuneCUETP8M1_13TeV-powheg-pythia8/MINIAODSIM/Asympt25ns_MCRUN2_74_V9-v2/00000/0AB045B5-BB0C-E511-81FD-0025905A60B8.root')
)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

# messages
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 10
process.MessageLogger.suppressWarning = cms.untracked.vstring('ecalLaserCorrFilter','manystripclus53X','toomanystripclus53X')
process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(False) )
process.options.allowUnscheduled = cms.untracked.bool(True)

# Load the producer for MVA IDs
process.load("RecoEgamma.ElectronIdentification.ElectronMVAValueMapProducer_cfi")

# select loose lepton collections
process.load('BoostedTTH.Producers.SelectedLeptonProducers_cfi')

#do projections
process.pfCHS = cms.EDFilter("CandPtrSelector", src = cms.InputTag("packedPFCandidates"), cut = cms.string("fromPV"))
process.pfNoMuonCHS =  cms.EDProducer("CandPtrProjector", src = cms.InputTag("pfCHS"), veto = cms.InputTag("SelectedMuonProducer:selectedLeptonsDL"))
process.pfNoElectronsCHS = cms.EDProducer("CandPtrProjector", src = cms.InputTag("pfNoMuonCHS"), veto =  cms.InputTag("SelectedElectronProducer:selectedLeptonsDL"))

# make patJets
process.load("PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff")
process.load("Configuration.EventContent.EventContent_cff")
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_dataRun2_Prompt_ICHEP16JEC_v0')

# make Boosted Jets
process.load('BoostedTTH.BoostedProducer.BoostedJetProducer_cfi')

# make PAT Jets from Boosted Jets
from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection

# HTT topjet
addJetCollection(
    process,
    labelName = 'HTTTopJetsPF',
    postfix="",
    jetSource = cms.InputTag('HTTTopJetProducer',''),
    algo = 'CA',
    rParam = 1.5,
    getJetMCFlavour = False,
    genJetCollection = None,
    jetCorrections = None,
    btagDiscriminators = None
)

# 3 HTT subjets
addJetCollection(
    process,
    labelName = 'HTTSubjetsPF',
    postfix="",
    jetSource = cms.InputTag('HTTTopJetProducer','subjets'),
    pfCandidates = cms.InputTag('packedPFCandidates'),
    pvSource = cms.InputTag('offlineSlimmedPrimaryVertices'),
    svSource = cms.InputTag('slimmedSecondaryVertices'),
    elSource = cms.InputTag('slimmedElectrons'),
    muSource = cms.InputTag('slimmedMuons'),
    algo = 'CA',
    rParam = 1.5,
    getJetMCFlavour = False,
    genJetCollection = None,
    jetCorrections = None,
    btagDiscriminators = [ 'pfCombinedInclusiveSecondaryVertexV2BJetTags','pfCombinedMVAV2BJetTags' ],
    explicitJTA = True,
    svClustering = True,
    fatJets=cms.InputTag('ca15PFJetsCHS'),
    groomedFatJets=cms.InputTag('HTTTopJetProducer')
)

# fatjet
addJetCollection(
    process,
    labelName = 'FatJetsPF',
    postfix = "",
    jetSource = cms.InputTag('ca15PFJetsCHS'),
    pfCandidates = cms.InputTag('packedPFCandidates'),
    pvSource = cms.InputTag('offlineSlimmedPrimaryVertices'),
    svSource = cms.InputTag('slimmedSecondaryVertices'),
    elSource = cms.InputTag('slimmedElectrons'),
    muSource = cms.InputTag('slimmedMuons'),
    algo = 'CA',
    rParam = 1.5,
    getJetMCFlavour = False,
    genJetCollection = None,
    jetCorrections = None,
    btagDiscriminators = ['pfBoostedDoubleSecondaryVertexCA15BJetTags']
)

# 2 SF subjets
addJetCollection(
    process,
    labelName = 'SFSubjetsPF',
    postfix = "",
    jetSource = cms.InputTag('SFJetProducer','sub'),
    pfCandidates = cms.InputTag('packedPFCandidates'),
    pvSource = cms.InputTag('offlineSlimmedPrimaryVertices'),
    svSource = cms.InputTag('slimmedSecondaryVertices'),
    elSource = cms.InputTag('slimmedElectrons'),
    muSource = cms.InputTag('slimmedMuons'),
    algo = 'CA',
    rParam = 1.5,
    getJetMCFlavour = False,
    genJetCollection = None,
    jetCorrections = None,
    btagDiscriminators = [ 'pfCombinedInclusiveSecondaryVertexV2BJetTags','pfCombinedMVAV2BJetTags' ],
    explicitJTA = True,
    svClustering = True,
    fatJets=cms.InputTag('ca15PFJetsCHS'),
    groomedFatJets=cms.InputTag('SFJetProducer','subcomp')
)

# SF filtered subjets
addJetCollection(
    process,
    labelName = 'SFFilterjetsPF',
    postfix = "",
    jetSource = cms.InputTag('SFJetProducer','filter'),
    pfCandidates = cms.InputTag('packedPFCandidates'),
    pvSource = cms.InputTag('offlineSlimmedPrimaryVertices'),
    svSource = cms.InputTag('slimmedSecondaryVertices'),
    elSource = cms.InputTag('slimmedElectrons'),
    muSource = cms.InputTag('slimmedMuons'),
    algo = 'CA',
    rParam = 1.5,
    getJetMCFlavour = False,
    genJetCollection = None,
    jetCorrections = None,
    btagDiscriminators = [ 'pfCombinedInclusiveSecondaryVertexV2BJetTags','pfCombinedMVAV2BJetTags' ],
    explicitJTA = True,
    svClustering = True,
    fatJets = cms.InputTag('ca15PFJetsCHS'),
    groomedFatJets = cms.InputTag('SFJetProducer','filtercomp')
)

# pruned subjets
addJetCollection(
    process,
    labelName = 'PrunedSubjetsPF',
    postfix = "",
    jetSource = cms.InputTag('ca15PFPrunedJetsCHS','subjets'),
    pfCandidates = cms.InputTag('packedPFCandidates'),
    pvSource = cms.InputTag('offlineSlimmedPrimaryVertices'),
    svSource = cms.InputTag('slimmedSecondaryVertices'),
    elSource = cms.InputTag('slimmedElectrons'),
    muSource = cms.InputTag('slimmedMuons'),
    algo = 'CA',
    rParam = 1.5,
    getJetMCFlavour = False,
    genJetCollection = None,
    jetCorrections = None,
    btagDiscriminators = [ 'pfCombinedInclusiveSecondaryVertexV2BJetTags','pfCombinedMVAV2BJetTags' ],
    explicitJTA = True,
    svClustering = True,
    fatJets=cms.InputTag('ca15PFJetsCHS'),
    groomedFatJets=cms.InputTag('ca15PFPrunedJetsCHS')
)

# softdrop subjets
addJetCollection(
    process,
    labelName = 'SDSubjetsPF',
    postfix = "",
    jetSource = cms.InputTag('ca15PFSoftdropJetsCHS','subjets'),
    pfCandidates = cms.InputTag('packedPFCandidates'),
    pvSource = cms.InputTag('offlineSlimmedPrimaryVertices'),
    svSource = cms.InputTag('slimmedSecondaryVertices'),
    elSource = cms.InputTag('slimmedElectrons'),
    muSource = cms.InputTag('slimmedMuons'),
    algo = 'CA',
    rParam = 1.5,
    getJetMCFlavour = False,
    genJetCollection = None,
    jetCorrections = None,
    btagDiscriminators = [ 'pfCombinedInclusiveSecondaryVertexV2BJetTags','pfCombinedMVAV2BJetTags' ],
    explicitJTA = True,
    svClustering = True,
    fatJets=cms.InputTag('ca15PFJetsCHS'),
    groomedFatJets=cms.InputTag('ca15PFSoftdropJetsCHS')
)


#adjust MC matching for all Boosted Jet collections
if not isData:
#HTT topjets
    process.patJetsHTTTopJetsPF.addGenJetMatch=False
    process.patJetPartonMatchHTTTopJetsPF.matched = "prunedGenParticles"
#HTT subjets
    process.patJetsHTTSubjetsPF.addGenJetMatch=False
    process.patJetPartonMatchHTTSubjetsPF.matched = "prunedGenParticles"
#SF fatjets
    process.patJetsSFFatJetsPF.addGenJetMatch=False
    process.patJetPartonMatchSFFatJetsPF.matched = "prunedGenParticles"
#SF subjets
    process.patJetsSFSubjetsPF.addGenJetMatch=False
    process.patJetPartonMatchSFSubjetsPF.matched = "prunedGenParticles"
#SF filterjets
    process.patJetsSFFilterjetsPF.addGenJetMatch=False
    process.patJetPartonMatchSFFilterjetsPF.matched = "prunedGenParticles"

    # all
    process.patJetPartons.particles = "prunedGenParticles"

#from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
    from RecoJets.JetProducers.ak4GenJets_cfi import ak4GenJets
#process.ak4PFJetsCHS = ak4PFJets.clone(src = 'pfNoElectronsCHS', doAreaFastjet = True)
    process.ak4GenJets = ak4GenJets.clone(src = 'packedGenParticles')

# match boosted reco Jets to pat jets
process.load('BoostedTTH.BoostedProducer.BoostedJetMatcher_cfi')

if not isData:
    # gen hadron matching for tt+X categorization
    process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
    process.load("BoostedTTH.BoostedProducer.genHadronMatching_cfi")

# skim
process.load("BoostedTTH.BoostedProducer.LeptonJetsSkim_cfi")

# execute in the right order
process.boosted_skimmed=cms.Path(process.electronMVAValueMapProducer
                                 *process.LeptonJetsSkim
                                 *process.SelectedElectronProducer
                                 *process.SelectedMuonProducer
                                 *process.ca15PFJetsCHS
                                 *process.HTTTopJetProducer
                                 *process.SFJetProducer
                                 *process.ca15PFPrunedJetsCHS
                                 *process.ca15PFSoftdropJetsCHS
                                 *process.ca15PFSoftdropJetsCHSforSubjettiness
				                         *process.ca15SoftdropSubjettiness
                                 *process.patJetsFatJetsPF
                                 *process.patJetsHTTTopJetsPF
                                 *process.patJetsHTTSubjetsPF
                                 *process.patJetsSFSubjetsPF
                                 *process.patJetsSFFilterjetsPF
                                 *process.patJetsPrunedSubjetsPF
                                 *process.patJetsSDSubjetsPF
                                 *process.BoostedJetMatcher)

# # execute in the right order
# process.boosted_skimmed=cms.Path(process.electronMVAValueMapProducer
#                                  *process.LeptonJetsSkim
#                                  *process.SelectedElectronProducer
#                                  *process.SelectedMuonProducer
#                                  *process.HTTTopJetProducer
#                                  *process.SFJetProducer
#                                  *process.patJetsHTTTopJetsPF
#                                  *process.patJetsHTTSubjetsPF
#                                  *process.patJetsSFFatJetsPF
#                                  *process.patJetsSFSubjetsPF
#                                  *process.patJetsSFFilterjetsPF
#                                  *process.BoostedJetMatcher)

process.OUT = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string('BoostedTTH_MiniAOD.root'),
    outputCommands = cms.untracked.vstring(['drop *','keep *_*_*_PAT','keep *_*_*_RECO','keep *_*_*_HLT*','keep *_*_*_SIM','keep *_*_*_LHE','keep *_*BoostedJetMatcher*_*_*','keep *_matchGen*Hadron_*_*', 'keep *_ak4GenJetsCustom_*_*', 'keep *_categorizeGenTtbar_*_*']),
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring("boosted_skimmed")
    )
)
process.endpath = cms.EndPath(process.OUT)
