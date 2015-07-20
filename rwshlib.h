// Copyright (C) 2005-2008 Samuel Newbold

class E_generic {};
class E_nan {};
class E_range {};

double my_strtod(const std::string& src);
float my_strtof(const std::string& src);
int my_strtoi(const std::string& src);
int my_strtoi(const std::string& src, int min, int max);
char my_strtoc(const std::string& src);
