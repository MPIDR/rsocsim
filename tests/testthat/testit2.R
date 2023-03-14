folder = rsocsim::create_simulation_folder()
folder
#folder = "C:/Users/tom/socsim/socsim_sim_2541/"
#folder= "C:\Users\tom\socsim\socsim_sim_2541"

supfile = rsocsim::create_sup_file(folder)
supfile = "socsim.sup"
supfile

seed = 47

rsocsim::socsim(folder,supfile,seed,suffix="test1")#,process_method = "inprocess")


supfile = "CousinDiversity.sup"
seed="33"


rsocsim::simulation_time_to_years(4500, 1200, 1960)

rsocsim::get_supplement_content(folder, supfile)


rsocsim::socsim(folder,supfile,seed,process_method = "inprocess")
rsocsim::socsim(folder,supfile,seed,process_method = "future")

rsocsim::socsim(folder,supfile,seed,process_method = "clustercall")

#rsocsim::run1simulationwithfile_clustercall(folder,supfile,seed)

socsim_path = "D:\\downloads\\socsim1.exe"
rsocsim::run1simulationwithfile_from_binary(folder,supfile,seed)
rsocsim::run1simulationwithfile_from_binary(folder,supfile,seed,socsim_path)


basename(socsim_path)
dirname(socsim_path)
dirname(folder)

system.file("extdata", "SWEfert2022", package = "rsocsim", mustWork = TRUE)




Rcpp::compileAttributes()

 
devtools::build(binary = TRUE, args = c('--preclean'))


# Rcmd.exe INSTALL --preclean --no-multiarch --with-keep.source rsocsim

install.packages('rsocsim')

devtools::install(dependencies = TRUE, build = TRUE, args = c('--preclean'))
