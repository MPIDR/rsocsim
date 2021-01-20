#include <Rcpp.h>
using namespace Rcpp;

// This is a simple example of exporting a C++ function to R. You can
// source this function into an R session using the Rcpp::sourceCpp 
// function (or via the Source button on the editor toolbar). Learn
// more about Rcpp at:
//
//   http://www.rcpp.org/
//   http://adv-r.had.co.nz/Rcpp.html
//   http://gallery.rcpp.org/
//

// [[Rcpp::export]]
NumericVector timesTwo3(NumericVector x) {
  return x * 2;
}

#include "src\events.cpp"
//#include "src\random.c"


// [[Rcpp::export]]
void socsimMain(){
  Rcpp::Rcout << "thistestobj" << std::endl;
}

// [[Rcpp::export]]
void socsimMain2(){
  Rcpp::Rcout << "jabadabadujujujuku--------------1u! noch etwas frueher" << std::endl;
  char *args[] = {
    (char*)"zerothArgument",
    (char*)"sample.sup",
    (char*)"12345",
    NULL
  };
  main22(3, args);
  //main(3,["bla1","bla2","bla3"]);
}


// You can include R code blocks in C++ files processed with sourceCpp
// (useful for testing and development). The R code will be automatically 
// run after the compilation.
//

/*** R
timesTwo(42)
*/
