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

SdBaseFile::SdBaseFile(const char* path, uint8_t oflag) {
  type_ = PFS_FILE_TYPE_CLOSED;
  open(path, oflag);
}

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

      if (i > n || c < 0X21 || c > 0X7E) { //solo caracteres imprimibles.
        DBG_FAIL_MACRO;
        goto fail;
      }
      // se guardan siempre en mayusculas
      name[i++] = c < 'a' || c > 'z' ?  c : c + ('A' - 'a');
    }
  }
  *ptr = str;
  return name[0] != ' ' && name[0] != 0;

 fail:
  return false;
}

bool SdBaseFile::exists(const char* name) {
  SdBaseFile file;
  return file.open(this, name, O_READ);
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

dir_t* SdBaseFile::readDirCache() {
  uint8_t i;
  // error if not directory
  if (!isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // index of entry in cache
  i = (curPosition_ >> 5) & 0XF;

  // use read to locate and cache block
  if (read() < 0) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // advance to next entry
  curPosition_ += 31;

  // return pointer to entry
  return vol_->cacheAddress()->dir + i;

 fail:
  return 0;
}

bool SdBaseFile::addCluster() {
  if (!vol_->allocContiguous(1, &curCluster_)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // if first cluster of file link to directory entry
  if (firstCluster_ == 0) {
    firstCluster_ = curCluster_;
    flags_ |= F_FILE_DIR_DIRTY;
  }
  return true;

 fail:
  return false;
}

bool SdBaseFile::openCachedEntry(uint8_t dirIndex, uint8_t oflag) {
  // location of entry in cache
  dir_t* p = &vol_->cacheAddress()->dir[dirIndex];

  // write or truncate is an error for a directory or read-only file
  if (p->attributes & (DIR_ATT_READ_ONLY | DIR_ATT_DIRECTORY)) {
    if (oflag & O_WRITE) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // remember location of directory entry on SD
  dirBlock_ = vol_->cacheBlockNumber();
  dirIndex_ = dirIndex;

  // copy first cluster number for directory fields
  firstCluster_ = p->firstCluster;

  // make sure it is a normal file or subdirectory
  if (DIR_IS_FILE(p)) {
    fileSize_ = p->fileSize;
    type_ = PFS_FILE_TYPE_NORMAL;
  } else if (DIR_IS_SUBDIR(p)) {
    if (!setDirSize()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    type_ = PFS_FILE_TYPE_SUBDIR;
  } else {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // save open flags for read/write
  flags_ = oflag & F_OFLAG;

  // set to start of file
  curCluster_ = 0;
  curPosition_ = 0;
  return true;

 fail:
  type_ = PFS_FILE_TYPE_CLOSED;
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
      #if !ENABLED_READ_ONLY      
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
      #endif
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
  type_ = PFS_FILE_TYPE_ROOT32;
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
int SdBaseFile::write(const void* buf, size_t nbyte) {
  // convert void* to uint8_t*  -  must be before goto statements
  const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);
  cache_t* pc;
  uint8_t cacheOption;
  // number of bytes left to write  -  must be before goto statements
  size_t nToWrite = nbyte;
  size_t n;
  // error if not a normal file or is read-only
  if (!isFile() || !(flags_ & O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }

  while (nToWrite) {
    uint8_t blockOfCluster = vol_->blockOfCluster(curPosition_);
    uint16_t blockOffset = curPosition_ & 0X1FF;
    if (blockOfCluster == 0 && blockOffset == 0) {
      // start of new cluster
      if (curCluster_ != 0) {
        uint32_t next;
        if (!vol_->pfsGet(curCluster_, &next)) {
          DBG_FAIL_MACRO;
          goto fail;
        }
        if (vol_->isEOC(next)) {
          // add cluster if at end of chain
          if (!addCluster()) {
            DBG_FAIL_MACRO;
            goto fail;
          }
        } else {
          curCluster_ = next;
        }
      } else {
        if (firstCluster_ == 0) {
          // allocate first cluster of file
          if (!addCluster()) {
            DBG_FAIL_MACRO;
            goto fail;
          }
        } else {
          curCluster_ = firstCluster_;
        }
      }
    }
    // block for data write
    uint32_t block = vol_->clusterStartBlock(curCluster_) + blockOfCluster;

    if (blockOffset != 0 || nToWrite < 512) {
      // partial block - must use cache
      // max space in block
      n = 512 - blockOffset;
      // lesser of space and amount to write
      if (n > nToWrite) n = nToWrite;

      if (blockOffset == 0 && curPosition_ >= fileSize_) {
        // start of new block don't need to read into cache
        cacheOption = SdVolume::CACHE_RESERVE_FOR_WRITE;
      } else {
        // rewrite part of block
        cacheOption = SdVolume::CACHE_FOR_WRITE;
        }
        pc = vol_->cacheFetch(block, cacheOption);
        if (!pc) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      uint8_t* dst = pc->data + blockOffset;
      memcpy(dst, src, n);
      if (512 == (n + blockOffset)) {
        if (!vol_->cacheWriteData()) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
    } else if (!USE_MULTI_BLOCK_SD_IO || nToWrite < 1024) {
      // use single block write command
      n = 512;
      if (vol_->cacheBlockNumber() == block) {
        vol_->cacheInvalidate();
      }
      if (!vol_->writeBlock(block, src)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    curPosition_ += n;
    src += n;
    nToWrite -= n;
  }
  if (curPosition_ > fileSize_) {
    // update fileSize and insure sync will update dir entry
    fileSize_ = curPosition_;
    flags_ |= F_FILE_DIR_DIRTY;
  }
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return nbyte;

 fail:
  return -1;
}

bool SdBaseFile::rmdir() {
  // must be open subdirectory
  if (!isSubDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  rewind();

  // make sure directory is empty
  while (curPosition_ < fileSize_) {
    dir_t* p = readDirCache();
    if (!p) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // done if past last used entry
    if (p->name[0] == DIR_NAME_FREE) break;
    // skip empty slot, '.' or '..'
    if (p->name[0] == DIR_NAME_DELETED || p->name[0] == '.') continue;
    // error not empty
    if (DIR_IS_FILE_OR_SUBDIR(p)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // convert empty directory to normal file for remove
  type_ = PFS_FILE_TYPE_NORMAL;
  flags_ |= O_WRITE;
  return remove();

 fail:
  return false;
}

cache_t* SdBaseFile::addDirCluster() {
  uint32_t block;
  cache_t* pc;
  // max folder size
  if (fileSize_/sizeof(dir_t) >= 0XFFFF) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!addCluster()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  block = vol_->clusterStartBlock(curCluster_);
  pc = vol_->cacheFetch(block, SdVolume::CACHE_RESERVE_FOR_WRITE);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  memset(pc, 0, 512);
  // zero rest of clusters
  for (uint8_t i = 1; i < vol_->blocksPerCluster_; i++) {
    if (!vol_->writeBlock(block + i, pc->data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // Increase directory file size by cluster size
  fileSize_ += 512UL*vol_->blocksPerCluster_;
  return pc;

 fail:
  return 0;
}

bool SdBaseFile::remove(SdBaseFile* dirFile, const char* path) {
  SdBaseFile file;
  if (!file.open(dirFile, path, O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return file.remove();

 fail:
  return false;
}

bool SdBaseFile::remove() {
  dir_t* d;
  // free any clusters - will fail if read-only or directory
  if (!truncate()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // cache directory entry
  d = cacheDirEntry(SdVolume::CACHE_FOR_WRITE);
  if (!d) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // mark entry deleted
  d->name[0] = DIR_NAME_DELETED;

  // set this file closed
  type_ = PFS_FILE_TYPE_CLOSED;

  // write entry to SD
  return vol_->cacheSync();
  return true;

 fail:
  return false;
}

bool SdBaseFile::mkdir(SdBaseFile* dir, const char dname[11]){
  if(!dir->isRoot()){
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

bool SdBaseFile::truncate() {
  uint32_t newPos;
  // error if not a normal file or read-only
  if (!isFile() || !(flags_ & O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }

  // fileSize and length are zero - nothing to do
  if (fileSize_ == 0) return true;

  // remember position for seek after truncation
  newPos = 0;

  // free all clusters
  if (!vol_->freeChain(firstCluster_)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  firstCluster_ = 0;
  
  fileSize_ = 0;

  // need to update directory entry
  flags_ |= F_FILE_DIR_DIRTY;

  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // set file to correct position
  return seekSet(0);

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

int SdBaseFile::peek() {
  PfsPos_t pos;
  getpos(&pos);
  int c = read();
  if (c >= 0) setpos(&pos);
  return c;
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

void SdBaseFile::dirName(const dir_t& dir, char* name) {
  uint8_t j = 0;
  for (uint8_t i = 0; i < 11; i++) {
    if (dir.name[i] == ' ')continue;
    if (i == 8) name[j++] = '.';
    name[j++] = dir.name[i];
  }
  name[j] = 0;
}

bool SdBaseFile::getFilename(char* name) {
  dir_t* p;
  if (!isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isRoot()) {
    name[0] = '/';
    name[1] = '\0';
    return true;
  }
  // cache entry
  p = cacheDirEntry(SdVolume::CACHE_FOR_READ);
  if (!p) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // format name
  dirName(*p, name);
  return true;

 fail:
  return false;
}

int16_t SdBaseFile::read() {
  uint8_t b;
  return read(&b, 1) == 1 ? b : -1;
}

int SdBaseFile::read(void* buf, size_t nbyte) {
  uint8_t blockOfCluster;
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
  uint16_t offset;
  size_t toRead;
  uint32_t block;  // raw device block number
  cache_t* pc;

  // error if not open or write only
  if (!isOpen() || !(flags_ & O_READ)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // max bytes left in file
  if (nbyte >= (fileSize_ - curPosition_)) {
    nbyte = fileSize_ - curPosition_;
  }
  // amount left to read
  toRead = nbyte;
  while (toRead > 0) {
    size_t n;
    offset = curPosition_ & 0X1FF;  // offset in block
    blockOfCluster = vol_->blockOfCluster(curPosition_);

    if (offset == 0 && blockOfCluster == 0) {
      // start of new cluster
      if (curPosition_ == 0) {
        // use first cluster in file
        curCluster_ = firstCluster_;
      } else {
        // get next cluster from FAT
        if (!vol_->pfsGet(curCluster_, &curCluster_)) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
    }
    block = vol_->clusterStartBlock(curCluster_) + blockOfCluster;
    
    if (offset != 0 || toRead < 512 || block == vol_->cacheBlockNumber()) {
      // amount to be read from current block
      n = 512 - offset;
      if (n > toRead) n = toRead;
      // read block to cache and copy data to caller
      pc = vol_->cacheFetch(block, SdVolume::CACHE_FOR_READ);
      if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      uint8_t* src = pc->data + offset;
      memcpy(dst, src, n);
    } else if (!USE_MULTI_BLOCK_SD_IO || toRead < 1024) {
      // read single block
      n = 512;
      if (!vol_->readBlock(block, dst)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    } else {
      uint8_t nb = toRead >> 9;

      n = 512*nb;
      if (vol_->cacheBlockNumber() <= block
        && block < (vol_->cacheBlockNumber() + nb)) {
        // flush cache if a block is in the cache
        if (!vol_->cacheSync()) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
      if (!vol_->sdCard()->readStart(block)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      for (uint8_t b = 0; b < nb; b++) {
        if (!vol_->sdCard()->readData(dst + b*512)) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
      if (!vol_->sdCard()->readStop()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    dst += n;
    curPosition_ += n;
    toRead -= n;
  }
  return nbyte;

 fail:
  return -1;
}

