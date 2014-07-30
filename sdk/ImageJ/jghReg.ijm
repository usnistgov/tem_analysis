run("Image Sequence...",
    "open=/home/hagedorn/Vis/CNST/tem_analysis/tools/gitRepository/centroid-qt/ui/atomCorrelation/00100.TEM1.atomCorr.tif     sort");

run("VerboseStackReg ", "transformation=[Rigid Body]");
run("Quit");
