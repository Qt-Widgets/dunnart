/*
 * $Revision: 1.9 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2009-07-10 06:59:08 +1000 (Fri, 10 Jul 2009) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of basic functionality (incl. file and
 * directory handling)
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#include <ogdf/basic/basic.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/String.h>
#include <time.h>

// Windows includes
#ifdef OGDF_SYSTEM_WINDOWS
#include <direct.h>
#if defined(_MSC_VER) && defined(UNICODE)
#undef GetFileAttributes
#undef FindFirstFile
#undef FindNextFile
#define GetFileAttributes  GetFileAttributesA
#define FindFirstFile  FindFirstFileA
#define WIN32_FIND_DATA WIN32_FIND_DATAA
#define FindNextFile  FindNextFileA
#endif
#endif

#ifdef __BORLANDC__	
#define _chdir chdir
#endif

// Unix includes
#ifdef OGDF_SYSTEM_UNIX
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <fnmatch.h>

double OGDF_clk_tck = sysconf(_SC_CLK_TCK); //is long. but definig it here avoids casts...
#endif


#ifdef OGDF_DLL

BOOL APIENTRY DllMain(HANDLE hModule, 
	DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			ogdf::PoolMemoryAllocator::init();
			ogdf::System::init();
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			ogdf::PoolMemoryAllocator::cleanup();
			break;
    }
    return TRUE;
}

#else

namespace ogdf {

//static int variables are automatically initialized with 0
int Initialization::s_count;

Initialization::Initialization()
{
	if (s_count++ == 0) {
		ogdf::PoolMemoryAllocator::init();
		ogdf::System::init();
	}
}

Initialization::~Initialization()
{
	if (--s_count == 0) {
		ogdf::PoolMemoryAllocator::cleanup();
	}
}

} // namespace ogdf

#endif


namespace ogdf {

	// debug level (in debug build only)
#ifdef OGDF_DEBUG
	DebugLevel debugLevel;
#endif


double usedTime(double& T)
{
	double t = T;
#ifdef OGDF_SYSTEM_WINDOWS
	T = double(clock())/CLOCKS_PER_SEC;
#endif
#ifdef OGDF_SYSTEM_UNIX
	struct tms now;
	times (&now);
	T = now.tms_utime/OGDF_clk_tck;	
#endif
	return  T-t;
}


#ifdef __MINGW32__

// XXX How do we implement these for MinGW GCC?
bool isDirectory(const char *fname)
{
}

bool isFile(const char *fname)
{
}

void changeDir(const char *dirName)
{
       chdir(dirName);
}

void getEntriesAppend(const char *dirName,
       FileType t,
       List<String> &entries,
       const char *pattern)
{
}

#elif defined(OGDF_SYSTEM_WINDOWS)

bool isFile(const char *fileName)
{
	DWORD att = GetFileAttributes(fileName);

	if (att == 0xffffffff) return false;
	return (att & FILE_ATTRIBUTE_DIRECTORY) == 0;
}


bool isDirectory(const char *fileName)
{
	DWORD att = GetFileAttributes(fileName);

	if (att == 0xffffffff) return false;
	return (att & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


void changeDir(const char *dirName)
{
	_chdir(dirName);
}


void getEntriesAppend(const char *dirName,
		FileType t,
		List<String> &entries,
		const char *pattern)
{
	OGDF_ASSERT(isDirectory(dirName));

	String filePattern;
	filePattern.sprintf("%s\\%s", dirName, pattern);

	WIN32_FIND_DATA findData;
	HANDLE handle = FindFirstFile(filePattern.cstr(), &findData);

	if (handle != INVALID_HANDLE_VALUE)
	{
		do {
			DWORD isDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			if(isDir && (
				strcmp(findData.cFileName,".") == 0 ||
				strcmp(findData.cFileName,"..") == 0)
			)
				continue;

			if (t == ftEntry || (t == ftFile && !isDir) ||
				(t == ftDirectory && isDir))
			{
				entries.pushBack(findData.cFileName);
			}
		} while(FindNextFile(handle, &findData));

		FindClose(handle);
	}
}
#endif

#ifdef OGDF_SYSTEM_UNIX

bool isDirectory(const char *fname)
{
	struct stat stat_buf;
	
  	if (stat(fname,&stat_buf) != 0)
  		return false;
  	return (stat_buf.st_mode & S_IFMT) == S_IFDIR;
}

bool isFile(const char *fname)
{
	struct stat stat_buf;
	
  	if (stat(fname,&stat_buf) != 0)
  		return false;
  	return (stat_buf.st_mode & S_IFMT) == S_IFREG;
}

void changeDir(const char *dirName)
{
	chdir(dirName);
}

void getEntriesAppend(const char *dirName,
	FileType t,
	List<String> &entries,
	const char *pattern)
{
	OGDF_ASSERT(isDirectory(dirName));

 	DIR* dir_p = opendir(dirName);

 	dirent* dir_e;
 	while ( (dir_e = readdir(dir_p)) != NULL )
 	{
 		const char *fname = dir_e->d_name;
   		if (pattern != 0 && fnmatch(pattern,fname,0)) continue;
   		
   		String fullName;
   		fullName.sprintf("%s/%s", dirName, fname);
   		
   		bool isDir = isDirectory(fullName.cstr());
 			if(isDir && (
				strcmp(fname,".") == 0 ||
				strcmp(fname,"..") == 0)
			)
				continue;
   		
		if (t == ftEntry || (t == ftFile && !isDir) ||
			(t == ftDirectory && isDir))
		{
			entries.pushBack(fname);
		}
	}
  
 	closedir(dir_p);
}
#endif


void getEntries(const char *dirName,
		FileType t,
		List<String> &entries,
		const char *pattern)
{
	entries.clear();
	getEntriesAppend(dirName, t, entries, pattern);
}


void getFiles(const char *dirName,
	List<String> &files,
	const char *pattern)
{
	getEntries(dirName, ftFile, files, pattern);
}


void getSubdirs(const char *dirName,
	List<String> &subdirs,
	const char *pattern)
{
	getEntries(dirName, ftDirectory, subdirs, pattern);
}


void getEntries(const char *dirName,
	List<String> &entries,
	const char *pattern)
{
	getEntries(dirName, ftEntry, entries, pattern);
}


void getFilesAppend(const char *dirName,
	List<String> &files,
	const char *pattern)
{
	getEntriesAppend(dirName, ftFile, files, pattern);
}


void getSubdirsAppend(const char *dirName,
	List<String> &subdirs,
	const char *pattern)
{
	getEntriesAppend(dirName, ftDirectory, subdirs, pattern);
}


void getEntriesAppend(const char *dirName,
	List<String> &entries,
	const char *pattern)
{
	getEntriesAppend(dirName, ftEntry, entries, pattern);
}





} // end namespace ogdf
