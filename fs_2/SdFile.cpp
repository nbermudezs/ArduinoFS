#include <SdFile.h>

SdFile::SdFile(const char* path, uint8_t oflag) : SdBaseFile(path, oflag) {
}

int SdFile::write(const void* buf, size_t nbyte) {
  return SdBaseFile::write(buf, nbyte);
}

size_t SdFile::write(uint8_t b) {
  return SdBaseFile::write(&b, 1) == 1 ? 1 : 0;
}

int SdFile::write(const char* str) {
  return SdBaseFile::write(str, strlen(str));
}