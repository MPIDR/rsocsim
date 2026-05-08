rsocsim_test_console_output_enabled <- function() {
  value <- Sys.getenv("RSOCSIM_TEST_CONSOLE_OUTPUT", "")
  if (!nzchar(value)) {
    return(FALSE)
  }

  tolower(value) %in% c("1", "true", "yes")
}

options(rsocsim.console_output = rsocsim_test_console_output_enabled())