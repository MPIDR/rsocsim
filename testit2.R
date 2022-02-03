folder = rsoc::create_simulation_folder()
supfile = "CousinDiversity.sup"
seed="33"

rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "inprocess")
rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "future")
rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "clustercall")

#rsoc::run1simulationwithfile_clustercall(folder,supfile,seed)

socsim_path = "D:\\downloads\\socsim1.exe"
rsoc::run1simulationwithfile_from_binary(folder,supfile,seed)
rsoc::run1simulationwithfile_from_binary(folder,supfile,seed,socsim_path)


basename(socsim_path)
dirname(socsim_path)
dirname(folder)

system.file("extdata", "SWEfert2022", package = "rsoc", mustWork = TRUE)
