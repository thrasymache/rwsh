// Copyright (C) 2005, 2006, 2007 Samuel Newbold

std::string::size_type find_close_brace(const std::string& focus,
                                        std::string::size_type i);

// errors thrown by my_strtoi
class E_generic_t {};
class E_nan_t {};
class E_range_t {};

int my_strtoi(const std::string& src);

