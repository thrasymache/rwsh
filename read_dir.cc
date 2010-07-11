// The template used to open a directory, and copy its entries into the 
// container specified by dest.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

template<class Out> int read_dir(const std::string& filename, Out dest) {
  std::string file = filename + '/';
  struct stat sb;
  if (stat(file.c_str(), &sb) || (sb.st_mode & S_IFMT) != S_IFDIR) return 1;
  DIR *src = opendir(file.c_str());
  if (!src) {
    Argv error_argv;
    error_argv.push_back("rwsh.unreadable_dir");
    error_argv.push_back(file);
    executable_map.run(error_argv);
    return 2;}
  struct dirent entry;
  struct dirent *result = &entry;
  while (!readdir_r(src, &entry, &result) && result) *dest++ = entry.d_name;
  return closedir(src);}

