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
 *Creditos: https://github.com/frasermac/SdPfslib
 */
 #include <SdPfs.h>
//------------------------------------------------------------------------------
/**
 * Initialize an SdPfs object.
 *
 * Initializes the SD card, SD volume, and root directory.
 *
 * \param[in] chipSelectPin SD chip select pin. See Sd2Card::init().
 * \param[in] sckRateID value for SPI SCK rate. See Sd2Card::init().
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdPfs::begin(uint8_t chipSelectPin, uint8_t sckRateID) {
  return card_.init(sckRateID, chipSelectPin) && vol_.init(&card_) && chdir(1);
}
//------------------------------------------------------------------------------
/** Change a volume's working directory to root
 *
 * Changes the volume's working directory to the SD's root directory.
 * Optionally set the current working directory to the volume's
 * working directory.
 *
 * \param[in] set_cwd Set the current working directory to this volume's
 *  working directory if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdPfs::chdir(bool set_cwd) {
  if (set_cwd) SdBaseFile::cwd_ = &vwd_;
  if (vwd_.isOpen()) vwd_.close();
  return vwd_.openRoot(&vol_);
}
//------------------------------------------------------------------------------
/** Change a volume's working directory
 *
 * Changes the volume working directory to the \a path subdirectory.
 * Optionally set the current working directory to the volume's
 * working directory.
 *
 * Example: If the volume's working directory is "/DIR", chdir("SUB")
 * will change the volume's working directory from "/DIR" to "/DIR/SUB".
 *
 * If path is "/", the volume's working directory will be changed to the
 * root directory
 *
 * \param[in] path The name of the subdirectory.
 *
 * \param[in] set_cwd Set the current working directory to this volume's
 *  working directory if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdPfs::chdir(const char *path, bool set_cwd) {
  SdBaseFile dir;
  if (path[0] == '/' && path[1] == '\0') return chdir(set_cwd);
  if (!dir.open(&vwd_, path, O_READ)) goto fail;
  if (!dir.isDir()) goto fail;
  vwd_ = dir;
  if (set_cwd) SdBaseFile::cwd_ = &vwd_;
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/** Set the current working directory to a volume's working directory.
 *
 * This is useful with multiple SD cards.
 *
 * The current working directory is changed to this volume's working directory.
 *
 * This is like the Windows/DOS \<drive letter>: command.
 */
void SdPfs::chvol() {
  SdBaseFile::cwd_ = &vwd_;
}
//------------------------------------------------------------------------------
/**
 * Test for the existence of a file.
 *
 * \param[in] name Name of the file to be tested for.
 *
 * \return true if the file exists else false.
 */
bool SdPfs::exists(const char* name) {
  return vwd_.exists(name);
}
#if USING_APP
//------------------------------------------------------------------------------
/** List the directory contents of the volume working directory to stdOut.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 * NO USAR!!!!! SOLO SERVIRA PARA LA APP Y HAY QUE REESCRIBIRLA PARA QUE REGRESE UN ARREGLO!!
 */
void SdPfs::ls(uint8_t flags) {
  vwd_.ls(stdOut_, flags);
}
//------------------------------------------------------------------------------
/** Rename a file or subdirectory.
 *
 * \param[in] oldPath Path name to the file or subdirectory to be renamed.
 *
 * \param[in] newPath New path name of the file or subdirectory.
 *
 * The \a newPath object must not exist before the rename call.
 *
 * The file to be renamed must not be open.  The directory entry may be
 * moved and file system corruption could occur if the file is accessed by
 * a file object that was opened before the rename() call.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdPfs::rename(const char *oldPath, const char *newPath) {
  SdBaseFile file;
  if (!file.open(oldPath, O_READ)) return false;
  return file.rename(&vwd_, newPath);
}
//------------------------------------------------------------------------------
/** List the directory contents of the volume working directory.
 *
 * \param[in] pr Print stream for list.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 */
void SdPfs::ls(Print* pr, uint8_t flags) {
  vwd_.ls(pr, flags);
}
#endif

#if ENABLED_READ_ONLY
//------------------------------------------------------------------------------
/** Make a subdirectory in the volume working directory.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
 *
 * \param[in] pFlag Create missing parent directories if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdPfs::mkdir(const char* path) {
  SdBaseFile sub;
  return sub.mkdir(&vwd_, path);
}
//------------------------------------------------------------------------------
/** Remove a file from the volume working directory.
*
* \param[in] path A path with a valid 8.3 DOS name for the file.
*
* \return The value one, true, is returned for success and
* the value zero, false, is returned for failure.
*/
bool SdPfs::remove(const char* path) {
  return SdBaseFile::remove(&vwd_, path);
}
//------------------------------------------------------------------------------
/** Remove a subdirectory from the volume's working directory.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
 *
 * The subdirectory file will be removed only if it is empty.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdPfs::rmdir(const char* path) {
  SdBaseFile sub;
  if (!sub.open(path, O_READ)) return false;
  return sub.rmdir();
}
#endif