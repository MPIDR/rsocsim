folder = rsoc::create_simulation_folder()
folder
#folder = "C:/Users/tom/socsim/socsim_sim_3792/"
#folder= "C:/Users/tom/socsim/socsim_sim_6396/"

supfile = rsoc::create_sup_file(folder)
supfile

seed = 300

rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "inprocess")


supfile = "CousinDiversity.sup"
seed="33"


rsoc::simulation_time_to_years(4500, 1200, 1960)

rsoc::get_supplement_content(folder, supfile)


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




Rcpp::compileAttributes()

 
devtools::build(binary = TRUE, args = c('--preclean'))


# Rcmd.exe INSTALL --preclean --no-multiarch --with-keep.source rsoc

install.packages('rsoc')

devtools::install(dependencies = TRUE, build = TRUE, args = c('--preclean'))
