folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
supfile = "CousinDiversity.sup"
seed="33"

rsoc::run1simulationwithfile(folder,supfile,seed)
rsoc::run1simulationwithfile_inprocess(folder,supfile,seed)
rsoc::run1simulationwithfile_apply(folder,supfile,seed)

rsoc::run1simulationwithfile_clustercall(folder,supfile,seed)

