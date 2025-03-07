folder = rsocsim::create_simulation_folder()
folder
print(paste0("folder: ", folder))

supfile = rsocsim::create_sup_file(folder)
print(supfile)

seed = 123345
suffix="test7"

rsocsim::socsim(
  folder,
  supfile,
  seed = seed,
  process_method = "inprocess",
  compatibility_mode = "1",
  suffix = suffix
  )

omar = rsocsim::read_omar(folder,supfile,seed,suffix=suffix)
omar

opop = rsocsim::read_opop(folder,supfile,seed,suffix)
opop

pid <- c("111", "10211", "10311")
kin_network <- getKin(opop = opop, omar = omar, pid = pid, 
                      extra_kintypes = c("unclesaunts", "niblings"), kin_by_sex = TRUE)

kin_network$nieces[[3]][1]

opop$pid[kin_network$nieces[[3]]]


####

fert_rates <- estimate_fertility_rates(
  opop=opop,
  final_sim_year=1966,
  year_min=1963,
  year_max=1965,
  year_group = 5,
  age_min_fert = 15,
  age_max_fert = 50,
  age_group = 5
)
 
