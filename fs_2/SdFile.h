#include <SdBaseFile.h>
#ifndef SdFile_h
#define SdFile_h

class SdFile : public SdBaseFile , public Print {
 public: 
  SdFile() {}
  SdFile(const char* name, uint8_t oflag);
#if DESTRUCTOR_CLOSES_FILE
  ~SdFile() {}
#endif  // DESTRUCTOR_CLOSES_FILE

  size_t write(uint8_t b);
  int write(const char* str);
  int write(const void* buf, size_t nbyte);
};
#endif  // SdFile_h
