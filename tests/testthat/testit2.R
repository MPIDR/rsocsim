folder = rsocsim::create_simulation_folder()
folder
#folder = "C:/Users/tom/socsim/socsim_sim_2541/"
#folder= "C:\Users\tom\socsim\socsim_sim_2541"

supfile = rsocsim::create_sup_file(folder)
# supfile = "socsim.sup"
supfile

seed = 777
suffix="sufffi33f"

rsocsim::socsim(folder,supfile,seed,suffix=suffix)#,process_method = "inprocess")

opop <- rsocsim::read_opop(folder,supfile,seed,suffix)
opop
omar <- rsocsim::read_omar(folder,supfile,seed,suffix)
omar
#Obtain partial kinship network, with omar and opop already in R environment
pid <- c(1000,1001) #opop$pid[-3:-1]
pid

kin_network <- rsocsim::getKin(opop = opop, omar = omar, pid = pid, extra_kintypes = c("unclesaunts", "niblings"), kin_by_sex = TRUE)

kin_network

kin_network$ggparents
kin_network$gdaughters

rsocsim::socsim(folder,supfile,seed,process_method = "inprocess",suffix=suffix)
rsocsim::socsim(folder,supfile,seed,process_method = "future",suffix=suffix)

rsocsim::socsim(folder,supfile,seed,process_method = "clustercall",suffix=suffix)

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



# retrieve Kin:
# Try it out -----

opop <- read_opop("output_pop.opop")
omar <- read_omar("output_pop.omar")



# ISSUES -----
