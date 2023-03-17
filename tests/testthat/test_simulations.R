folder = rsocsim::create_simulation_folder()
folder
print(paste0("folder: ", folder))

supfile = rsocsim::create_sup_file(folder)
print(supfile)

seed = 42
suffix="test3"

rsocsim::socsim(folder,supfile,seed,suffix,process_method = "inprocess")

omar = rsocsim::read_omar(folder,supfile,seed,suffix)
omar



opop = rsocsim::read_opop(folder,supfile,seed,suffix)
opop