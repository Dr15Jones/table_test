import ROOT 

theFile = ROOT.TFile("test_branch.root")

events = theFile.Get("Events")
entries = events.GetEntriesFast()

for i in range(entries):
    events.GetEntry(i)
    for eta in events.Jets_eta:
        print eta
