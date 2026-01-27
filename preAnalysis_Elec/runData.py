import sys
import os
import re
JSAPSYS = os.getenv("JSAPSYS")
sys.path.append(JSAPSYS+"/Analysis/Utils")
import JPUtils


JPDataDir = os.getenv('JPDataDir')

#for RunNo in range(1200, 1346):
#for RunNo in [1282,1284,1343,1174,1150,1160,1274,1190,1280,1299,1332,1292,1091]:
for RunNo in range(1601, 1700+1):
#for RunNo in [1346, 1500]:
    if not JPUtils.IsGoodRun(RunNo):
        continue
    datadir = JPDataDir+"/run%08d"%(RunNo)
    #print(datadir)
    outputdir = JPDataDir+"/../02_PreAnalysis/run%08d"%(RunNo)
    if not os.path.exists(outputdir):
        os.mkdir(outputdir)
        #print(outputdir)
    
    logdir = "log/run%d"%(RunNo)
    if not os.path.exists(logdir):
        os.mkdir(logdir)
    
    nfiles = 0
    for filename in os.listdir(datadir):
        s1 = re.match("Jinping_1ton_Phy_\d{8}_00\d{6}(_)?(\d)*.root", filename)
        if s1!= None:
            nfiles += 1
    #print(nfiles);
    list1 = list(range(0, nfiles, 100))
    lastfiles = nfiles-list1[-1]
    if lastfiles<50 and len(list1)>1:
    #if nfiles%100 != 0:
        list1.pop()
    for i,run in enumerate(list1):
        start = run
        end = run+100-1
        if i==len(list1)-1:
            end = nfiles-1
        numlist = {"RunNo": RunNo, "start":start, "end":end}
        #cmd=  "bsub -o log/data_{RunNo}_{start}_{end}.log ./PreAnalysisData {RunNo} {start} {end} ".format(**numlist)+outputdir+"/PreAnalysis_{RunNo}_{start}_{end}.root".format(**numlist)
        cmd=  "bsub -o log/run{RunNo}/data_{RunNo}_{start}_{end}.log ./PreAnalysisData {RunNo} {start} {end} ".format(**numlist)+outputdir
        #print(cmd)
        os.system(cmd)
    
        

    #cmd = "bsub -o log/data{RunNo}.log ./PreAnalysisData {RunNo} /home/jinping/JinpingData/Jinping_1ton_Data/02_PreAnalysis/PreAnalysis_{RunNo}.root".format(**numlist)
    #print(cmd)
    #os.system(cmd)
