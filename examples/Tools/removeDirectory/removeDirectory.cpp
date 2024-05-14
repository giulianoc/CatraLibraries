/*
 Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

#include "FileIO.h"
#include "StringUtils.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int iArgc, char *pArgv[])

{

  string suffix = ".tar.gz";
  string source = "/var/catramms/storage/MMSWorkingAreaRepository/Staging/"
                  "1_7441941_virtualVOD_2024_05_08_00_45_03_0042/"
                  "7441941_liveRecorderVirtualVOD.tar.gz";
  cout << "aaa: " << StringUtils::endWith(source, suffix) << endl;
  const char *pPathName;
  Boolean_t bIsDirectoryExisting;

  if (iArgc != 2) {
    std::cerr << "Usage: " << pArgv[0] << " <directory>" << std::endl;

    return 1;
  }

  pPathName = pArgv[1];

  cout << "isDirectoryExisting " << pPathName << std::endl;
  if (FileIO::isDirectoryExisting(pPathName, &bIsDirectoryExisting) !=
      errNoError) {
    Error err = ToolsErrors(__FILE__, __LINE__,
                            TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);
    std::cerr << (const char *)err << std::endl;

    return 1;
  }

  cout << "isDirectoryExisting " << bIsDirectoryExisting << std::endl;
  // if (!bIsDirectoryExisting)
  // {
  // 	std:: cerr << "The " << pPathName << " file does not exist." << std::
  // endl;

  // 	return 1;
  // }

  cout << "removeDirectory " << pPathName << std::endl;
  Boolean_t bRemoveRecursively = true;
  if (FileIO::removeDirectory(pPathName, bRemoveRecursively) != errNoError) {
    Error err =
        ToolsErrors(__FILE__, __LINE__, TOOLS_FILEIO_REMOVEDIRECTORY_FAILED);
    std::cerr << (const char *)err << std::endl;

    return 1;
  }

  cout << "removeDirectory finished" << std::endl;

  return 0;
}
