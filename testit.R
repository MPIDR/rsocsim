folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
supfile = "CousinDiversity.sup"
seed="33"

rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "inprocess")
rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "future")
rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "clustercall")

rsoc::run1simulationwithfile_clustercall(folder,supfile,seed)

