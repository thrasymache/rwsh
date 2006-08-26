// The template used to open a directory, and copy its entries into the 
// container specified by dest.
//
// Copyright (C) 2005, 2006 Samuel Newbold

template<class Out> int read_dir(const std::string& filename, Out dest) {
  std::string file = filename + '/';
  struct stat sb;
  if (stat(file.c_str(), &sb) || (sb.st_mode & S_IFMT) != S_IFDIR) return 1;
  char buffer[sb.st_blksize];
  int src = open(file.c_str(), O_RDONLY);
  if (src < 0) {
    Argv_t error_argv;
    error_argv.push_back("rwsh.unreadable_dir");
    error_argv.push_back(file);
    executable_map[error_argv](error_argv);
    return 2;}
  long basep;
  while (int length = getdirentries(src, buffer, sb.st_blksize, &basep)) {
    char* end = buffer + length;
    struct dirent* focus;
    for (char* i=buffer; i<end; i+=focus->d_reclen) {
      focus = (struct dirent*) i;
      *dest++ = focus->d_name;}}
  close(src);
  return 0;}

