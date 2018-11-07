// The template used to open a directory, and copy its entries into the
// container specified by dest.
//
// Copyright (C) 2005-2018 Samuel Newbold

template<class Out> int read_dir(const std::string& filename, Out dest) {
  std::string file = filename + '/';
  DIR *src = opendir(file.c_str());
  if (!src) throw Exception(Argm::Unreadable_dir, file, errno);
  struct dirent *result = nullptr;
  while (result = readdir(src)) *dest++ = result->d_name;
  return closedir(src);}

