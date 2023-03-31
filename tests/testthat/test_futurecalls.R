
library("rsocsim")

# create a new folder for all the files related to a simulation.
# this will be in your home- or user-directory:
folder = socsim::create_simulation_folder()

# create a new supervisory-file. supervisory-files tell socsim what
# to simulate. create_sup_file will create a very basic supervisory file
# and it copies some rate-files that will also be needed into the 
# simulation folder:
supfile = "socsim.sup" #rsocsim::create_sup_file(folder)

# Choose a random-number seed:
seed = 300

# Start the simulation:
rsocsim::socsim(folder,supfile,seed)


rsocsim::socsim(folder,supfile,seed,process_method = "future")

warnings()

print("jobida!")

rsocsim::socsim(folder,supfile,seed,process_method = "clustercall")


callr::r(print,c("hallo welt"))



processFile = function(filepath) {
  con = file(filepath, "r")
  while ( TRUE ) {
    line = readLines(con, n = 1)
    if ( length(line) == 0 ) {
      break
    }
    print(line)
  }
  
  close(con)
}
processFile(paste0(folder,supfile))




f1 <- future::future({
  #startSocsimWithFile(supfile,seed,compatibility_mode)
  Sys.sleep(10)
},seed=TRUE)
print("started!")
# start a loop and check whether the simulation in the future is finished.
# if it is not yet finished, read the output file and print the last line
# to the console
print("wait for simulation to finish")
while (!future::resolved(f1)) {
  Sys.sleep(0.333)
  processFile(supfile)
}











