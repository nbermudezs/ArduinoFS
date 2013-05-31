/* Arduino PFS Library
 * Copyright (C) 2013 by Enrique Urbina, Moises Martinez and Néstor Bermúdez
 *
 * This file is part of the Arduino PFS Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino PFS Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/*
 *Creditos: https://github.com/frasermac/sdfatlib
 */
#ifndef SdBaseFile_h
#define SdBaseFile_h

#include <Arduino.h>
#include <SdPfsConfig.h>
#include <SdVolume.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#ifndef PGM_P
#define PGM_P const char*
#endif  // PGM_P
#ifndef PSTR
#define PSTR(x) (x)
#endif  // PSTR
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#endif  // pgm_read_byte
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif  // pgm_read_word
#ifndef PROGMEM
#define PROGMEM const
#endif  // PROGMEM
#endif  // __AVR__

struct PfsPos_t {
  uint32_t position;
  uint32_t cluster;
  PfsPos_t() : position(0), cluster(0) {}
};

class SdBaseFile {
 public:
  SdBaseFile() : type_(PFS_FILE_TYPE_CLOSED) {}
  SdBaseFile(const char* path, uint8_t oflag); //ya
#if DESTRUCTOR_CLOSES_FILE
  ~SdBaseFile() {if(isOpen()) close();} //ya
#endif  // DESTRUCTOR_CLOSES_FILE

  void getpos(PfsPos_t* pos); //ya
  void setpos(PfsPos_t* pos); //ya

  uint32_t available() {return fileSize() - curPosition();}  //ya

  uint32_t curCluster() const {return curCluster_;} //ya
  uint32_t curPosition() const {return curPosition_;} //ya

  static SdBaseFile* cwd() {return SdBaseFile::cwd_;}   //ya

  uint32_t fileSize() const {return fileSize_;} //ya
  bool getFilename(char* name); //ya
  bool exists(const char* name);

  bool isDir() const {return type_ >= PFS_FILE_TYPE_MIN_DIR;} //ya
  bool isFile() const {return type_ == PFS_FILE_TYPE_NORMAL;} //ya
  bool isOpen() const {return type_ != PFS_FILE_TYPE_CLOSED;} //ya
  bool isSubDir() const {return type_ == PFS_FILE_TYPE_SUBDIR;} //ya
  bool isRoot() const {return type_ == PFS_FILE_TYPE_ROOT32;} //ya
  
  bool open(const char* path, uint8_t oflag = O_READ); //ya
  bool open(SdBaseFile* dirFile, const char* path, uint8_t oflag); //ya
  bool open(SdBaseFile* dirFile, const uint8_t dname[11], uint8_t oflag); //ya
  bool openRoot(SdVolume* vol); //ya
  bool openNext(SdBaseFile* dirFile, uint8_t oflag); //ya
  bool close(); //ya

  #if !ENABLED_READ_ONLY
  bool mkdir(SdBaseFile* dir, const char dname[11]); //ya
  cache_t* addDirCluster(); //ya
  static bool remove(SdBaseFile* dirFile, const char* path); //ya
  bool remove(); //ya
  bool truncate(); //ya
  bool rmdir(); //ya
  int write(const void* buf, size_t nbyte);
  #endif  

  static void dirName(const dir_t& dir, char* name);

  int16_t read(); //ya
  int read(void* buf, size_t nbyte); //ya
  int peek(); //ya  
  
  bool seek(uint32_t pos, uint8_t option); //ya
  void rewind() {seekSet(0);} //ya
  
  bool sync(); //ya

private:
  friend class SdPfs;       // allow SdPfs to set cwd_  

  SdVolume* vol_;           // volume where file is located
  uint32_t  curCluster_;    // cluster for current file position
  uint32_t  curPosition_;   // current file position in bytes from beginning}
  uint32_t  fileSize_;      // file size in bytes  
  uint32_t  dirBlock_;      // block for this files directory entry
  uint32_t  firstCluster_;  // first cluster of file
  
  static SdBaseFile* cwd_;  // global pointer to cwd dir

  bool seekSet(uint32_t pos); //ya
  bool seekEnd(int32_t offset = 0) {return seekSet(fileSize_ + offset);} //ya
  bool seekCur(int32_t offset) {return seekSet(curPosition_ + offset); } //ya

  /** Cache related*/
  bool openCachedEntry(uint8_t cacheIndex, uint8_t oflags); //ya
  dir_t* readDirCache(); //ya
  dir_t* cacheDirEntry(uint8_t action); //ya
  bool addCluster(); //ya


  static bool make83Name(const char* str, char* name, const char** ptr); //ya
  bool setDirSize(); //ya

  // bits defined in flags_
  // should be 0X0F
  static uint8_t const F_OFLAG = O_ACCMODE;
  // sync of directory entry required
  static uint8_t const F_FILE_DIR_DIRTY = 0X80;

  uint8_t   flags_;         // See above for definition of flags_ bits
  uint8_t   fstate_;        // error and eof indicator
  uint8_t   type_;          // type of file see above for values
  uint8_t   dirIndex_;      // index of directory entry in dirBlock
};

 #endif  //SdBaseFile_h