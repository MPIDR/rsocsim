if (FALSE) {

folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
supfile = "CousinDiversity.sup"
seed="33"

rsocsim::run1simulationwithfile(folder,supfile,seed,process_method = "inprocess")
rsocsim::run1simulationwithfile(folder,supfile,seed,process_method = "future")
rsocsim::run1simulationwithfile(folder,supfile,seed,process_method = "clustercall")

#rsocsim::run1simulationwithfile_clustercall(folder,supfile,seed)

socsim_path = "D:\\downloads\\socsim1.exe"
rsocsim::run1simulationwithfile_from_binary(folder,supfile,seed)
rsocsim::run1simulationwithfile_from_binary(folder,supfile,seed,socsim_path)


rsocsim::get_asfr_socsim()

rsocsim::getKin()

basename(socsim_path)
dirname(socsim_path)
dirname(folder)

############################################




#################################
}