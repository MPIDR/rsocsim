#include <Rcpp.h>
using namespace Rcpp;

#include "./src/events.cpp"
//#include "src\random.c"

// [[Rcpp::export]]
int startSocsimWithFile(String supfile, String seed, String compatibility_mode){
  Rcpp::Rcout << "Start socsim" << std::endl;
  char *args[] = {
    (char*)"zerothArgument",
    (char*) supfile.get_cstring(),
    (char*) seed.get_cstring(),
    (char*) compatibility_mode.get_cstring(),
    NULL
  };
  main1(2, args);
  Rcpp::Rcout << "Socsim Done."  << std::endl;
  
  return 0;
  
}


// You can include R code blocks in C++ files processed with sourceCpp
// (useful for testing and development). The R code will be automatically 
// run after the compilation.
//

// /*** R
// timesTwo(42)
// */
