# Convert SOCSIM months to calendar years. 
asYr <- function(month, last_month, final_sim_year) {
  return(final_sim_year - trunc((last_month - month)/12))
}

## Returns simulation month corresponding to July of a given real calendar year
jul <- function(year, last_month, final_sim_year){
  return((last_month-5) - (final_sim_year - year)*12)
}

## Estimate yearly number of births any given year
yearly_birth_by_age_socsim <- function(df, year_range, age_breaks_fert) {
  
  last_month <- max(df$dob)
  
  out <- df %>% 
    left_join(df %>% select(mom = pid, mother_birth = birth_year), 
              by = "mom") %>% 
    select(birth_year, mother_birth) %>% 
    filter(birth_year %in% year_range) %>% 
    mutate(birth_year_factor = factor(birth_year, levels = year_range),
           mother_age = birth_year - mother_birth,
           mother_agegr_factor = cut(mother_age, breaks = age_breaks_fert, 
                                     include.lowest = F, right = F, ordered_results = T)) %>%
    filter(!is.na(mother_agegr_factor)) %>% 
    count(birth_year = birth_year_factor, mother_agegr = mother_agegr_factor) %>% 
    complete(birth_year, mother_agegr, fill = list(n = 0)) %>% 
    select(year = birth_year, agegr = mother_agegr, n) %>% 
    arrange(year, agegr)
  
  return(out)
  
}

#### Get Women Reproductive Age SOCSIM (Female mid-year population)
# Return women by age alive on the 1st of July of a given year, including right-censored

get_women_reproductive_age_socsim <- function(df, final_sim_year, year, age_breaks_fert) {
  
  last_month <- max(df$dob)
  df$census <- year
  
  out <- df %>% 
    mutate(dod2 = ifelse(dod == 0, 999999999, dod)) %>%
    filter(fem == 1 & dob < jul(year, last_month, final_sim_year) & dod2 >= jul(year, last_month, final_sim_year)) %>%
    mutate(age_at_census = trunc((jul(census, last_month, final_sim_year)-dob)/12),
           agegr_at_census = cut(age_at_census, breaks = age_breaks_fert, 
                                 include.lowest = F, right = F, ordered_results = T)) %>% 
    filter(!is.na(agegr_at_census)) %>% 
    count(agegr_at_census, census) %>% 
    complete(agegr_at_census, census, fill = list(n = 0)) %>% 
    select(year = census, agegr = agegr_at_census, n) %>% 
    arrange(year, agegr)
  
  return(out)
}

## Census SOCSIM (Mid-year population, 1st July)
# Returns population by sex and age alive on the 1st of July of a given year, including right-censored
census_socsim <- function(df, year, final_sim_year, age_levels_census) {
  
  last_month <- max(df$dob)
  df$census <- year 
  
  out <- df %>% 
    mutate(dod2 = ifelse(dod == 0, 999999999, dod)) %>%
    filter(dob < jul(year, last_month, final_sim_year) & dod2 >= jul(year, last_month, final_sim_year)) %>% 
    mutate(age_at_census = trunc((jul(census, last_month, final_sim_year)-dob)/12)) %>%
    count(sex, age_at_census, census) %>% 
    mutate(sex = factor(sex, levels = c("0","1")),
           age_at_census = factor(age_at_census, levels = age_levels_census)) %>%
    complete(sex, age_at_census, census, fill = list(n = 0))
  
  return(out)
}