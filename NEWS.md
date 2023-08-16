# rsocsim 1.5.9
 * removed keyword register in load.cpp (See https://github.com/MPIDR/rsocsim/issues/11; it causes errors on Mac)

# rsocsim 1.5.8
 * renamed getKin() to retrieve_kin()
 * bugfixes

# rsocsim 1.5.7
 * bugfixes

# rsocsim 1.5.6
 * read_opop and read_omar
 * works on linux and windows, also with future and clustercall. Mac not tested, linux sometimes flaky

# rsocsim 1.5.5
 * compiles and runs on Linux again
 * reorganized contributed code from Liliana and Mallika
 * todo: tests for contributed code. how to use these functions?
 * todo: install and run on Mac

# rsocsim 1.5.4
 * Max age 200 years
 * new command for sup files: marriage_after_childbirth 1
   * allows simulations without marriage rates
   * mothers will marry immediately before childbirth if not married already

# rsocsim 1.5.3
 * compiles on Github now
 * use pkgdown to generate documentation-website
 