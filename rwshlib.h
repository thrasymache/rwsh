// Copyright (C) 2005-2008 Samuel Newbold

class E_generic_t {};
class E_nan_t {};
class E_range_t {};

double my_strtod(const std::string& src);
float my_strtof(const std::string& src);
int my_strtoi(const std::string& src);
int my_strtoi(const std::string& src, int min, int max);
char my_strtoc(const std::string& src);
