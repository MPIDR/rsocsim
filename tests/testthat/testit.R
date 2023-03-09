

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

basename(socsim_path)
dirname(socsim_path)
dirname(folder)

############################################


usethis::use_github_action("pkgdown")

usethis::use_pkgdown_github_pages()

devtools::document()

pkgdown::build_site()


#################################