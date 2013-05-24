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
#include <SdBaseFile.h>
#include <SdPfs.h>

bool SdBaseFile::seek(uint32_t pos, uint8_t option){
  if(option == SEEK_END_)
    return seekEnd(pos);
  else if(option == SEEK_CUR_)
    return seekCur(pos);
  else if(option == SEEK_BEG_)
    return seekSet(pos);
  else
    return false;
}

#define DBG_FAIL_MACRO  //  Serial.print(__FILE__);Serial.println(__LINE__)

SdBaseFile* SdBaseFile::cwd_ = 0;

void SdBaseFile::getpos(PfsPos_t* pos) {
  pos->position = curPosition_;
  pos->cluster = curCluster_;
}

void SdBaseFile::setpos(PfsPos_t* pos) {
  pos->position = curPosition_;
  pos->cluster = curCluster_;
}

bool SdBaseFile::sync() {
  #if ENABLED_READ_ONLY
  return true;
  #else
  // only allow open files and directories
  if (!isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (flags_ & F_FILE_DIR_DIRTY) {
    dir_t* d = cacheDirEntry(SdVolume::CACHE_FOR_WRITE);
    // check for deleted by another open file object
    if (!d || d->name[0] == DIR_NAME_DELETED) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // do not set filesize for dir files
    if (!isDir()) d->fileSize = fileSize_;
    // clear directory dirty
    flags_ &= ~F_FILE_DIR_DIRTY;
  }
  return vol_->cacheSync();

 fail:
  return false;
  #endif
}

dir_t* SdBaseFile::cacheDirEntry(uint8_t action) {
  cache_t* pc;
  pc = vol_->cacheFetch(dirBlock_, action);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return pc->dir + dirIndex_;

 fail:
  return 0;
}

bool SdBaseFile::open(const char* path, uint8_t oflag) {
  return open(cwd_, path, oflag);
}

bool SdBaseFile::open(SdBaseFile* dirFile, const char* path, uint8_t oflag) {
  char dname[11];
  SdBaseFile dir1, dir2;
  SdBaseFile *parent = dirFile;
  SdBaseFile *sub = &dir1;

  if (!dirFile) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (*path == '/') {
    while (*path == '/') path++;
    if (!dirFile->isRoot()) {
      if (!dir2.openRoot(dirFile->vol_)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      parent = &dir2;
    }
  }
  if(!make83Name(path, dname, &path)){
    DBG_FAIL_MACRO;
    goto fail;
  }
  while(!*path == '/') path++;
  /* just level 1 directories allowed*/
  if(!parent->isRoot() && *path !=0){
    DBG_FAIL_MACRO;
    goto fail;
  }
  return open(parent, dname, oflag);  

 fail:
  return false;
}

bool SdBaseFile::open(SdBaseFile* dirFile, const uint8_t dname[11], uint8_t oflag) {
  cache_t* pc;
  uint8_t i;

  bool emptyFound = false;
  bool fileFound = false;  
  dir_t* p;
  
  if(dirFile->isFile()) goto fail;

  vol_ = dirFile->vol_;

  //porque hay que buscar secuencialmente
  dirFile->rewind();

  while (dirFile->curPosition_ < dirFile->fileSize_) {
    i = (dirFile->curPosition_ >> 5) & 0XF;
    p = dirFile->readDirCache();
    if (!p) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if(p->name[0] == DIR_NAME_FREE || p->name[0] == DIR_NAME_DELETED){
      if (!emptyFound) {
        emptyFound = true;
        dirBlock_ = vol_->cacheBlockNumber();
        dirIndex_ = i;        
      }
      // done if no entries follow
      if (p->name[0] == DIR_NAME_FREE) break;
    }else{
      if(memcmp(p->name, dname, 11)==0){
        fileFound = true;
        break;
      }
    }
  }
  
  if(!fileFound){
    if((oflag & O_CREAT) || (oflag & O_WRITE)){
      #if ENABLED_READ_ONLY
      DBG_FAIL_MACRO;
      goto fail;
      #endif
      if(emptyFound){
        p = cacheDirEntry(SdVolume::CACHE_FOR_WRITE);
        i = dirIndex_;
      }else{
        pc = dirFile->addDirCluster();
        if (!pc) {
          DBG_FAIL_MACRO;
          goto fail;
        }
        i = 0;
        p = pc->dir;
        memset(p, 0, sizeof(dir_t));
        memcpy(p->name, dname, 11);
      }
    }    
  }  
  return openCachedEntry(i, oflag);  

 fail:
  return false;
}

bool SdBaseFile::openRoot(SdVolume* vol) {
  if (isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  vol_ = vol;
  type_ = PFS_FILE_TYPE_ROOT_FIXED;
  firstCluster_ = vol->rootDirStart();
  if (!setDirSize()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  flags_ = O_RDONLY;

  dirBlock_ = firstCluster_;
  dirIndex_ = 0;

  curCluster_ = 0;
  curPosition_ = 0;  
  return true;

 fail:
  return false;
}

bool SdBaseFile::openNext(SdBaseFile* dirFile, uint8_t oflag){
  if (!dirFile || isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  vol_ = dirFile->vol_;

  dir_t* p;
  uint8_t i;

  do{
    if(!(p = dirFile->readDirCache())){
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (p->name[0] == DIR_NAME_FREE) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (p->name[0] == DIR_NAME_DELETED || p->name[0] == '.') {
      continue;
    }
    i = (dirFile->curPosition_ >> 5) & 0XF;
    if (DIR_IS_FILE_OR_SUBDIR(p))
      return openCachedEntry(i, oflag);
  }while(true);
  
 fail:
  return false;
}

bool SdBaseFile::close() {
  bool res = sync();
  type_ = PFS_FILE_TYPE_CLOSED;
  return res;
}

#if !ENABLED_READ_ONLY
bool mkdir(SdBaseFile* dir, const char dname[11]){
  if(!dir.isRoot()){
    DBG_FAIL_MACRO;
    goto fail;
  }
  char new_dir[11];
  if (!make83Name(dname, new_dir, &dname)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!open(dir, dname, O_CREAT | O_RDWR)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  type_ = PFS_FILE_TYPE_SUBDIR;
  flags_ = O_READ;
  //ya es un directorio, ahora hay que agregar la referencia (dir_t) para . y ..

  if (!addDirCluster()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  dir_t* p;
  dir_t dots;

  if((p = cacheDirEntry(SdVolume::CACHE_FOR_WRITE)) == 0){
    DBG_FAIL_MACRO;
    goto fail;
  }
  p->attributes = DIR_ATT_DIRECTORY;
  
  memcpy(&dots, p, sizeof(dots));
  memcpy(&dots.name, ".", 11);
  dots.firstCluster = 0;

  return true;
 fail:
  return false;
}
#endif

bool SdBaseFile::seekSet(uint32_t pos) {
  uint32_t nCur;
  uint32_t nNew;

  if (!isOpen() || pos > fileSize_) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (type_ == PFS_FILE_TYPE_ROOT_FIXED) {
    curPosition_ = pos;
    goto done;
  }
  if (pos == 0) {
    curCluster_ = 0;
    curPosition_ = 0;
    goto done;
  }else{
	  curCluster_ = pos/512L;
	  curPosition_ = pos % 512L;
  }
 done:
  return true;

 fail:
  return false;
}



bool SdBaseFile::setDirSize() {
  uint16_t s = 0;
  uint32_t cluster = firstCluster_;
  do {
    if (!vol_->pfsGet(cluster, &cluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    s += vol_->blocksPerCluster();
    // max size if a directory file is 4096 blocks
    if (s >= 4096) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } while (!vol_->isEOC(cluster));
  fileSize_ = 512L*s;
  return true;

 fail:
  return false;
}

bool SdBaseFile::make83Name(const char* str, char* name, const char** ptr) {
  uint8_t c;
  uint8_t n = 7;  
  uint8_t i = 0;

  while (i < 11) name[i++] = 0;

  i = 0;
  while (*str != '\0' && *str != '/') {
    c = *str++;
    if (c == '.') {
      while(i < 8) name[i++] = ' ';
      if (n == 10) {
        // only one dot allowed
        DBG_FAIL_MACRO;
        goto fail;
      }
      n = 10; 
      i = 8;   
    } else {

#ifdef __AVR__
      // store chars in flash
      PGM_P p = PSTR("|<>^+=?/[];,*\"\\");
      uint8_t b;
      while ((b = pgm_read_byte(p++))) if (b == c) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#else  // __AVR__
      // store chars in RAM
      if (strchr("|<>^+=?/[];,*\"\\", c)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#endif  // __AVR__

      if (i > n || c < 0X21 || c > 0X7E) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      // convert lower to upper
      name[i++] = c < 'a' || c > 'z' ?  c : c + ('A' - 'a');
    }
  }
  *ptr = str;
  return name[0] != ' ' && name[0] != 0;

 fail:
  return false;
}