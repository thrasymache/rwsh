// Copyright (C) 2005-2019 Samuel Newbold

class E_generic {};
class E_nan {};
class E_not_an_integer {};
class E_range {};

double my_strtod(const std::string& src);
int my_strtoi(const std::string& src);
int my_strtoi(const std::string& src, int min, int max);
