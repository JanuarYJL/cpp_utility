#ifndef utility_include_utility_database_otl_otl_ora11g_r2_cfg_h
#define utility_include_utility_database_otl_otl_ora11g_r2_cfg_h

// ORA11G_R2
#define OTL_STL
#define OTL_ORA11G_R2
#define OTL_ORA_UTF8
// connect pool
#define OTL_CONNECT_POOL_ON
#define OTL_CPP_11_ON
// env val
#define SET_ENV_VAL_ZHS16GBK putenv(const_cast<char*>("NLS_LANG=AMERICAN_AMERICA.ZHS16GBK"))
#define SET_ENV_VAL_AL32UTF8 putenv(const_cast<char*>("NLS_LANG=AMERICAN_AMERICA.AL32UTF8"))

#endif//!utility_include_utility_database_otl_otl_ora11g_r2_cfg_h