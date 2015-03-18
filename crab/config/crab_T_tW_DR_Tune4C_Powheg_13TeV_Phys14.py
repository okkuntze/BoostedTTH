from WMCore.Configuration import Configuration
config = Configuration()

config.section_("General")
config.General.requestName = 'MC_Powheg_T_tW_DR_Tune4C_13TeV_PHYS14'
config.General.workArea = 'crab_projects'

config.section_("JobType")
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = '/nfs/dust/cms/user/shwillia/CMSSW_7_2_3/src/BoostedTTH/BoostedProducer/test/boostedProducer_plusTrees_cfg.py'
config.JobType.outputFiles = ['BoostedTTH_MiniAOD.root','BoostedTTH_Tree.root','BoostedTTH_Cutflow.txt']

config.section_("Data")
config.Data.inputDataset = '/T_tW-channel-DR_Tune4C_13TeV-CSA14-powheg-tauola/Phys14DR-PU20bx25_PHYS14_25_V1-v1/MINIAODSIM'
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.publication = True
#config.Data.totalUnits = 5
#config.Data.publishDbsUrl = 'phys03'
config.Data.publishDataName = 'BoostedTTH_MiniAOD'

config.section_("Site")
config.Site.storageSite = 'T2_DE_DESY'
