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
#ifndef SdPfs_h
#define SdPfs_h

 /** SdPfs version YYYYMMDD */
#define SD_PFS_VERSION 20130313
//------------------------------------------------------------------------------
/** error if old IDE */
#if !defined(ARDUINO) || ARDUINO < 100
#error Arduino IDE must be 1.0 or greater
#endif  // ARDUINO < 100}
 //------------------------------------------------------------------------------
#include <SdFile.h>
#include <SdStream.h>
#include <ArduinoStream.h>
#include <MinimumSerial.h>
//------------------------------------------------------------------------------
/**
 * \class SdPfs
 * \brief Integration class for the %SdPfs library.
 */
class SdPfs {
 public:
  SdPfs() {}
  /**
   * Initialize an SdFat object.
   *
   * Initializes the SD card, SD volume, and root directory.
   *
   * \param[in] sckRateID value for SPI SCK rate. See Sd2Card::init().
   * \param[in] chipSelectPin SD chip select pin. See Sd2Card::init().
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool init(uint8_t sckRateID = SPI_FULL_SPEED,
    uint8_t chipSelectPin = SD_CHIP_SELECT_PIN) {
    return begin(chipSelectPin, sckRateID);
  }
  Sd2Card* card() {return &card_;}
  bool chdir(bool set_cwd = false);
  bool chdir(const char* path, bool set_cwd = false);
  bool exists(const char* name);
  bool begin(uint8_t chipSelectPin = SD_CHIP_SELECT_PIN,
    uint8_t sckRateID = SPI_FULL_SPEED);
  void ls(uint8_t flags = 0);
  void ls(Print* pr, uint8_t flags = 0);
  bool mkdir(const char* path, bool pFlag = true);
  bool remove(const char* path);
  bool rename(const char *oldPath, const char *newPath);
  bool rmdir(const char* path);

  SdVolume* vol() {return &vol_;}
  /** \return a pointer to the volume working directory. */
  SdBaseFile* vwd() {return &vwd_;}

  //----------------------------------------------------------------------------
  /**
   *  Set stdOut Print stream for messages.
   * \param[in] stream The new Print stream.
   */
  static void setStdOut(Print* stream) {stdOut_ = stream;}
  /** \return Print stream for messages. */
  static Print* stdOut() {return stdOut_;}

 private:
  Sd2Card card_;
  SdVolume vol_;
  SdBaseFile vwd_;
  static Print* stdOut_;


 #endif  // SdPfs_h