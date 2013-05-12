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

#ifdef __AVR__
	#include <avr/pgmspace.h>
#endif  // __AVR__
#include <Arduino.h>
#include <SdPfsConfig.h>
#include <SdVolume.h>

// use the gnu style oflag in open()
/** open() oflag for reading */
uint8_t const O_READ = 0X01;
/** open() oflag - same as O_IN */
uint8_t const O_RDONLY = O_READ;
/** open() oflag for write */
uint8_t const O_WRITE = 0X02;
/** open() oflag - same as O_WRITE */
uint8_t const O_WRONLY = O_WRITE;
/** open() oflag for reading and writing */
uint8_t const O_RDWR = (O_READ | O_WRITE);
/** open() oflag mask for access modes */
uint8_t const O_ACCMODE = (O_READ | O_WRITE);
/** The file offset shall be set to the end of the file prior to each write. */
uint8_t const O_APPEND = 0X04;
/** synchronous writes - call sync() after each write */
uint8_t const O_SYNC = 0X08;
/** truncate the file to zero length */
uint8_t const O_TRUNC = 0X10;
/** set the initial position at the end of the file */
uint8_t const O_AT_END = 0X20;
/** create the file if nonexistent */
uint8_t const O_CREAT = 0X40;
/** If O_CREAT and O_EXCL are set, open() shall fail if the file exists */
uint8_t const O_EXCL = 0X80;

/** ls() flag to print file size */
uint8_t const LS_SIZE = 2;
/** ls() flag for recursive list of subdirectories */
uint8_t const LS_R = 4;

// values for type_
/** This file has not been opened. */
uint8_t const PFS_FILE_TYPE_CLOSED = 0;
/** A normal file */
uint8_t const PFS_FILE_TYPE_NORMAL = 1;
/** A FAT12 or FAT16 root directory */
uint8_t const PFS_FILE_TYPE_ROOT_FIXED = 2;
/** A FAT32 root directory */
uint8_t const PFS_FILE_TYPE_ROOT32 = 3;
/** A subdirectory file*/
uint8_t const PFS_FILE_TYPE_SUBDIR = 4;
/** Test value for directory type */
uint8_t const PFS_FILE_TYPE_MIN_DIR = PFS_FILE_TYPE_ROOT_FIXED;

struct PfsPos_t {
  /** stream position */
  uint32_t position;
  /** cluster for position */
  uint32_t cluster;
  PfsPos_t() : position(0), cluster(0) {}
};

class SdBaseFile {
 public:
  /** Create an instance. */
  SdBaseFile() : writeError(false), type_(PFS_FILE_TYPE_CLOSED) {}
  SdBaseFile(const char* path, uint8_t oflag);
#if DESTRUCTOR_CLOSES_FILE
  ~SdBaseFile() {if(isOpen()) close();}
#endif  // DESTRUCTOR_CLOSES_FILE
  /**
   * writeError is set to true if an error occurs during a write().
   * Set writeError to false before calling print() and/or write() and check
   * for true after calls to print() and/or write().
   */
  bool writeError;
  /** \return value of writeError */
  bool getWriteError() {return writeError;}
  /** Set writeError to zero */
  void clearWriteError() {writeError = 0;}

  void getpos(PfsPos_t* pos);
  /** set position for streams
   * \param[out] pos struct with value for new position
   */
  void setpos(PfsPos_t* pos);

  uint32_t available() {return fileSize() - curPosition();}
  /** \return The current cluster number for a file or directory. */
  uint32_t curCluster() const {return curCluster_;}
  /** \return The current position for a file or directory. */
  uint32_t curPosition() const {return curPosition_;}
  /** \return The total number of bytes in a file or directory. */
  uint32_t fileSize() const {return fileSize_;}
  /** \return Current working directory */
  static SdBaseFile* cwd() {return cwd_;}

private:
  SdVolume* vol_;           // volume where file is located
  uint32_t  curCluster_;    // cluster for current file position
  uint32_t  curPosition_;   // current file position in bytes from beginning
};

 #endif  //SdBaseFile_h