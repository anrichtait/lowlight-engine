/**
  \file G3D-base.lib/source/FileSystem.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/FileSystem.h"
#include "G3D-base/System.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/fileutils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "zip.h"
#include <regex>
#include <cstring>
#include "G3D-base/g3dfnmatch.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"

#ifdef G3D_WINDOWS
    // Needed for _getcwd
#   include <direct.h>

    // Needed for _findfirst
#   include <io.h>

#    define stat64 _stat64
#else
#   include <dirent.h>
#   include <fnmatch.h>
#   include <unistd.h>
#   define _getcwd getcwd
#   define _stat stat
#   define stricmp strcasecmp 
#   define strnicmp strncasecmp 
#endif

#ifdef G3D_OSX
#    define stat64 stat
#endif 

namespace G3D {

static FileSystem* common = nullptr;

std::recursive_mutex FileSystem::s_mutex;

FileSystem& FileSystem::instance() {
    init();
    return *common;
}


void FileSystem::init() {
    if (common == nullptr) {
        common = new FileSystem();
    }
}


void FileSystem::cleanup() {
    if (common != nullptr) {
        delete common;
        common = nullptr;
    }
}


bool FileSystem::registerPasswordProtectedZip(const String& zipFile, const String& password) {
	const String& canonical = FilePath::canonicalize(zipFile);
	if (exists(canonical) && isZipfile(canonical)) {
		instance().m_canonicalToPassword.set(canonical, password);
		return true;
	}
	return false;

}

bool FileSystem::isPasswordProtected(const String & zipFile, String & password)
{
	const String& canonical = FilePath::canonicalize(zipFile);
	if (instance().m_canonicalToPassword.containsKey(canonical)) { // check that key is in table
			instance().m_canonicalToPassword.get(canonical, password);
			return true;
	}

	return false;
}


FileSystem::FileSystem() : m_cacheLifetime(10) {}

/////////////////////////////////////////////////////////////

bool FileSystem::Dir::contains(const String& f, bool caseSensitive) const {
    
    for (int i = 0; i < nodeArray.size(); ++i) {
        if (caseSensitive) {
            if (f == nodeArray[i].name) {
                return true;
            }
        } else if (stricmp(f.c_str(), nodeArray[i].name.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

    
void FileSystem::Dir::computeZipListing(const String& zipfile, const String& _pathInsideZipfile) {
    const String& pathInsideZipfile = FilePath::canonicalize(_pathInsideZipfile);
    const String& filename = FilePath::canonicalize(FilePath::removeTrailingSlash(zipfile));
    struct zip* z = zip_open(filename.c_str(), ZIP_CHECKCONS, nullptr);
    debugAssertM(z, format("Could not open zipfile '%s'", filename.c_str()));

    int count = zip_get_num_files( z );
    Set<String> alreadyAdded;
    for (int i = 0; i < count; ++i) {
        struct zip_stat info;
        zip_stat_init( &info );    // TODO: Docs unclear if zip_stat_init is required.
        zip_stat_index( z, i, ZIP_FL_NOCASE, &info );
        
        // Fully-qualified name of a file inside zipfile
        String name = FilePath::canonicalize(info.name);

        if (beginsWith(name, pathInsideZipfile)) {
            // We found something inside the directory we were looking for,
            // so the directory itself must exist
            exists = true;

            // For building the cached directory listing, extract only elements that do not contain 
            // additional subdirectories.

            size_t start = pathInsideZipfile.size();
            if ((name.length() > start) && isSlash(name[start])) {
                ++start;
            }

            size_t end = findSlash(name, start);
            if (end == String::npos) {
                // There are no more slashes; add this name
                name = name.substr(start);
                if (alreadyAdded.insert(name)) {
                    Entry& e = nodeArray.next();
                    e.name = name;
                    e.type = FILE_TYPE;
                }
            } else {
                // There are more slashes, indicating that this is a directory
                name = name.substr(start, end);
                if (alreadyAdded.insert(name)) {
                    Entry& e = nodeArray.next();                    
                    e.name = FilePath::removeTrailingSlash(name);
                    e.type = DIR_TYPE;
                }
            }
        }
    }
    
    zip_close(z);
    z = nullptr;
}


FileSystem::Dir& FileSystem::getContents(const String& path, bool forceUpdate) {
    const String& key = 
#   if defined(G3D_WINDOWS)
        FilePath::canonicalize(FilePath::removeTrailingSlash(toLower(FilePath::canonicalize(resolve(path)))));
#   else
        FilePath::canonicalize(FilePath::removeTrailingSlash(FilePath::canonicalize(resolve(path))));
#   endif
    
    RealTime now = System::time();
    Dir& dir = m_cache.getCreate(key);

    if ((now > dir.lastChecked + cacheLifetime()) || forceUpdate) {
        dir = Dir();

        // Out of date: update
        dir.lastChecked = now;
#       ifdef G3D_WINDOWS
            // On windows, we have to use GetFileAttributes (http://msdn.microsoft.com/en-us/library/aa364944(v=vs.85).aspx) instead of 
            // stat in order to work with network shares
            const DWORD st = GetFileAttributesA(key.c_str());
            const bool exists = (st != INVALID_FILE_ATTRIBUTES);
            const bool isDirectory = (st & FILE_ATTRIBUTE_DIRECTORY) != 0;
#       else
            struct _stat st;
            const bool exists = _stat(key.c_str(), &st) != -1;
            const bool isDirectory = (st.st_mode & S_IFDIR) != 0;
#       endif

        // Does this path exist on the real filesystem?
        if (exists && isDirectory) {

            // Is this path actually a directory?
            if (isDirectory) {
                dir.exists = true;
                // Update contents
#               ifdef G3D_WINDOWS
                    const String& filespec = FilePath::concat(key, "*");
                    struct _finddata_t fileinfo;
                    intptr_t handle = _findfirst(filespec.c_str(), &fileinfo);
                    debugAssert(handle != -1);
                    int result = 0;
                    do {
                        if ((strcmp(fileinfo.name, ".") != 0) && (strcmp(fileinfo.name, "..") != 0)) {
                            Entry& e = dir.nodeArray.next();
                            e.name = fileinfo.name;
                            if ((fileinfo.attrib & _A_SUBDIR) != 0) {
                                e.type = DIR_TYPE;
                            } else {
                                e.type = FILE_TYPE;
                            }
                        }

                        result = _findnext(handle, &fileinfo);
                    } while (result == 0);
                    _findclose(handle);

#               else
                    DIR* listing = opendir(key.c_str());
                    debugAssertM(listing, "opendir failed on '" + key + "'");
                    struct dirent* entry = readdir(listing);
                    while (entry != nullptr) {
                        if ((strcmp(entry->d_name, "..") != 0) && (strcmp(entry->d_name, ".") != 0)) {
                            Entry& e = dir.nodeArray.next();
                            e.name = entry->d_name;

#                           ifdef _DIRENT_HAVE_D_TYPE
                                // Not all POSIX systems support this field
                                // http://www.delorie.com/gnu/docs/glibc/libc_270.html
                                switch (entry->d_type) {
                                case DT_DIR:
                                    e.type = DIR_TYPE;
                                    break;
                                    
                                case DT_REG:
                                    e.type = FILE_TYPE;
                                    break;
                                    
                                case DT_UNKNOWN:
                                default:
                                    e.type = UNKNOWN;
                                    break;
                                }
#                           endif
                        }
                        entry = readdir(listing);
                    }
                    closedir(listing);
                    listing = nullptr;
                    entry = nullptr;
#               endif
            }

        } else {
            String zip, internalFile;

            if (exists && isZipfile(path)) {
                // This is a zipfile; get its root
                dir.isZipfile = true;                
                dir.computeZipListing(path, "");

            } else if (inZipfile(path, zip, internalFile)) {

                // There is a zipfile somewhere in the path.  Does
                // the rest of the path exist inside the zipfile?
                dir.inZipfile = true;
                dir.computeZipListing(zip, internalFile);
            }
        }        
    }

    return dir;
}


bool FileSystem::_inZipfile(const String& _path, String& z, String& internalFile) {
    String path = FilePath::canonicalize(FilePath::expandEnvironmentVariables(_path));
    

    // Collapse "/X/../" subpaths to "/". This won't work correctly with
    // symlinks, but it greatly simplifies the checking below.
    for (size_t i = path.find("/../"); i > 0 && i != String::npos; i = path.find("/../", i)) {
        // Search backwards for the previous path element to delete it
        const size_t j = path.rfind("/", i - 1);
        if (path.substr(j + 1, 3) != "../") {
            path = path.substr(0, j + 1) + path.substr(i + 4);
            i = j;
        }
        else i++;
    }

    // Look at all sub-paths. For each, ask if it is a zipfile.
    size_t current = 0;
    current = findSlash(path, current);

    while (current != String::npos) {

        // Look forward for the next slash
        const size_t s = findSlash(path, current + 1);

        if (s == String::npos) {
            // No more slashes
            return false;
        }

        z = path.substr(0, s); 
        if (_isFile(z)) {
            if (_isZipfile(z)) {
                internalFile = path.substr(s + 1);
                return true;
            }
        }
        
        current = s;
    }

    z = "";
    internalFile = "";
    return false;
}


bool FileSystem::_isZipfile(const String& _filename) {

    const String& filename = FilePath::canonicalize(FilePath::expandEnvironmentVariables(_filename));
    
    // Windows drive
    if ((filename.length() == 2) && (filename[1] == ':')) {
        return false;
    }

    FILE* f = fopen(FilePath::removeTrailingSlash(filename).c_str(), "r");
    if (f == nullptr) {
        return false;
    }

    uint8 header[4];
    if (fread(header, 1, 4, f) != 4) {
        fclose(f);
        return false;
    }
    
    const uint8 zipHeader[4] = {0x50, 0x4b, 0x03, 0x04};
    for (int i = 0; i < 4; ++i) {
        if (header[i] != zipHeader[i]) {
            fclose(f);
            return false;
        }
    }
    
    fclose(f);
    return true;
}


FILE* FileSystem::_fopen(const char* _filename, const char* mode) {
    const String& filename = FilePath::canonicalize(FilePath::expandEnvironmentVariables(_filename));

    for (const char* m = mode; *m != '\0'; ++m) {
        if (*m == 'w') {
            // Purge the cache entry for the parent of this directory
            _clearCache(FilePath::parent(_resolve(filename)));
            break;
        }
    }

    markFileUsed(filename);
    return ::fopen(filename.c_str(), mode);
}


void FileSystem::_clearCache(const String& _path) {
    const String& path = FilePath::expandEnvironmentVariables(_path);

    if ((path == "") || FilePath::isRoot(path)) {
        m_cache.clear();
    } else {
        Array<String> keys;
        m_cache.getKeys(keys);

        const String& prefix = 
#           ifdef G3D_WINDOWS
                toLower(FilePath::canonicalize(FilePath::removeTrailingSlash(_resolve(path))));
#           else
                FilePath::canonicalize(FilePath::removeTrailingSlash(_resolve(path)));
#           endif
        const String& prefixSlash = prefix + "/";

        for (int k = 0; k < keys.size(); ++k) {
            const String& key = keys[k];
            if ((key == prefix) || beginsWith(key, prefixSlash)) {
                m_cache.remove(keys[k]);
            }
        }
    }
}


void FileSystem::_setCacheLifetime(float t) {
    m_cacheLifetime = t;
}


void FileSystem::_createDirectory(const String& _dir) {
    const String& dir = FilePath::expandEnvironmentVariables(_dir);

    if (dir == "") {
        return;
    }
    
    String d = _resolve(dir);
    
    // Add a trailing / if there isn't one.
    switch (d[d.size() - 1]) {
    case '/':
    case '\\':
        break;

    default:
        d += "/";
    }

    // If it already exists, do nothing
    if (_exists(FilePath::removeTrailingSlash(d))) {
        return;
    }

    // Parse the name apart
    String root, base, ext;
    Array<String> path;

    String lead;
    FilePath::parse(d, root, path, base, ext);
    debugAssert(base == "");
    debugAssert(ext == "");

    // Begin with an extra period so "c:\" becomes "c:\.\" after
    // appending a path and "c:" becomes "c:.\", not root: "c:\"
    String p = root + ".";

    // Create any intermediate that doesn't exist
    for (int i = 0; i < path.size(); ++i) {
        p += "/" + path[i];
        if (! _exists(p)) {
            // Windows only requires one argument to mkdir,
            // where as unix also requires the permissions.
#           ifndef G3D_WINDOWS
                mkdir(p.c_str(), 0777);
#           else
                _mkdir(p.c_str());
#           endif
        }
    }

    _clearCache(FilePath::parent(FilePath::removeTrailingSlash(d)));
}


int FileSystem::_rename(const String& _source, const String& _dest) {
    const String& source = FilePath::expandEnvironmentVariables(_source);
    const String& dest   = FilePath::expandEnvironmentVariables(_dest);
    
    _clearCache(FilePath::parent(_resolve(source)));
    markFileUsed(source);
    const int i = ::rename(source.c_str(), dest.c_str());
    _clearCache(FilePath::parent(_resolve(dest)));
    markFileUsed(dest);
    return i;
}


void FileSystem::_copyDir(const String& _source, const String& _dest) {
    const String& source    = FilePath::removeTrailingSlash(FilePath::expandEnvironmentVariables(_source));
    //const String& dest        = FilePath::expandEnvironmentVariables(_dest);
    const bool forceUpdate = true;
    const Dir& entry = getContents(source, forceUpdate);
    
    for (int i = 0; i < entry.nodeArray.size(); ++i) {
        const String& sourceChild = _source + entry.nodeArray[i].name;
        const String& destChild = _dest + entry.nodeArray[i].name;
        if (entry.nodeArray[i].type == FileSystem::Type::FILE_TYPE) {
            // Workaround for directories in zip files, 
            // which improperly are assigned as FILE_TYPE
            // TODO: Fix need for workaround
            if (! endsWith(sourceChild, "/")) {
                _copyFile(sourceChild, destChild);
            }
        } else if (entry.nodeArray[i].type == FileSystem::Type::DIR_TYPE) {
            _createDirectory(destChild);
            _copyDir(sourceChild + "/", destChild + "/");
        } else {
            alwaysAssertM(false, "Unknown file type");
        }
    }
}


void FileSystem::_copyFile(const String& _source, const String& _dest) {
    const String& source = FilePath::expandEnvironmentVariables(_source);
    const String& dest = FilePath::expandEnvironmentVariables(_dest);
#   ifdef G3D_WINDOWS
        // TODO: handle case where srcPath is in a zipfile
        if (inZipfile(source)) {
            // Read it all in, then dump it out
            BinaryInput  in(source, G3D_LITTLE_ENDIAN);
            BinaryOutput out(dest, G3D_LITTLE_ENDIAN);
            out.writeBytes(in.getCArray(), in.size());
            out.commit(false);
        } else {
            CopyFileA(source.c_str(), dest.c_str(), FALSE);
            _clearCache(FilePath::parent(_resolve(dest)));
        }
#   else
        // Read it all in, then dump it out
        BinaryInput  in(source, G3D_LITTLE_ENDIAN);
        BinaryOutput out(dest, G3D_LITTLE_ENDIAN);
        out.writeBytes(in.getCArray(), in.size());
        out.commit(false);
#   endif        
}


bool FileSystem::_exists(const String& _f, bool trustCache, bool caseSensitive) {
    const String& f = FilePath::expandEnvironmentVariables(_f);

    if (FilePath::isRoot(f)) {
#       ifdef G3D_WINDOWS
            const String& winname = toLower(f.substr(0, 1)) + ":\\";
            return _drives().contains(winname);
#       else
            return true;
#       endif
    }

    const String& path = FilePath::removeTrailingSlash(f);
    const String& parentPath = FilePath::parent(path);

    const bool forceUpdate = ! trustCache;
    const Dir& entry = getContents(parentPath, forceUpdate);

    if (FilePath::containsWildcards(f)) {
        if (! entry.exists) {
            // The directory didn't exist, so neither do its contents
            return false;
        } 

        const String& pattern = FilePath::baseExt(path);

        // See if any element of entry matches the wild card
        for (int i = 0; i < entry.nodeArray.size(); ++i) {
            if (FilePath::matches(entry.nodeArray[i].name, pattern, caseSensitive)) {
                return true;
            }
        }

        // Could not find a match
        return false;

    } else {
        return entry.exists && entry.contains(FilePath::baseExt(path), caseSensitive);
    }
}


bool FileSystem::_isDirectory(const String& _filename, bool expand) {
    const String& filename = expand ? FilePath::expandEnvironmentVariables(_filename) : _filename;
    // TODO: work with zipfiles and cache
    struct _stat st;
    const bool exists = _stat(FilePath::removeTrailingSlash(filename).c_str(), &st) != -1;
    return exists && ((st.st_mode & S_IFDIR) != 0);
}

bool FileSystem::_isFile(const String& _filename) {
    const String& filename = FilePath::expandEnvironmentVariables(_filename);
    // TODO: work with zipfiles and cache
    struct _stat st;
    const bool exists = _stat(FilePath::removeTrailingSlash(filename).c_str(), &st) != -1;
    return exists && ((st.st_mode & S_IFREG) != 0);
}

void FileSystem::_removeFile(const String& path) {
    alwaysAssertM(! inZipfile(path), "Cannot invoke removeFile() on files inside zipfiles.");
    Array<String> files;
    getFiles(path, files, true);

    for (int i = 0; i < files.size(); ++i) {
        const String& filename = files[i];
        int retval = ::remove(filename.c_str());
        (void)retval;
    }
    
    // Remove from cache
    _clearCache(FilePath::parent(path));
}

String FileSystem::_NFDStandardizeFilename(const String& _path, const String& _cwd) {
    const String& filename = FilePath::canonicalize(_resolve(_path, _cwd));
    Array<String> sections = stringSplit(filename, '/');
    Array<size_t> removalIndices;

    for (size_t i = 0; i < (size_t)sections.size(); ++i) {
        if (sections[i] == ".") {
            removalIndices.append(i);
        }

        else if (sections[i] == "..") {
            removalIndices.append(i);
            debugAssertM(i != 0, "Absolute path cannot begin with  \"..\"");
            removalIndices.append(i - 1);
        }
    }

    for (size_t i = 0; i < (size_t)removalIndices.size(); ++i) {
        #ifdef _MSC_VER
#       pragma warning(disable: 4244) // conversion from 'G3D::uint64' to 'int', possible loss of data
        #endif
        sections.remove(removalIndices[static_cast<int>(i)]);
        #ifdef _MSC_VER
#       pragma warning(default: 4244)
        #endif
    }

    const char slash
#   ifdef G3D_WINDOWS
        = '\\';
#   else
        = '/';
#   endif
    return stringJoin(sections, slash);
}

String FileSystem::_resolve(const String& _filename, const String& _cwd) {
    const String& filename = FilePath::expandEnvironmentVariables(_filename);
    const String& cwd = FilePath::expandEnvironmentVariables(_cwd);

    if (filename.size() >= 1) {
        if (isSlash(filename[0])) {
            // Already resolved
            return filename;
        } else {

            #ifdef G3D_WINDOWS
                if ((filename.size() >= 2) && (filename[1] == ':')) {
                    // There is a drive spec on the front.
                    if ((filename.size() >= 3) && isSlash(filename[2])) {
                        // Already fully qualified
                        return filename;
                    } else {
                        // The drive spec is relative to the
                        // working directory on that drive.
                        debugAssertM(false, "Files of the form d:path are"
                                     " not supported (use a fully qualified"
                                     " name).");
                        return filename;
                    }
                }
            #endif
        }
    }

    // Prepend the working directory.
    return FilePath::concat(cwd, filename);
}


String FileSystem::_currentDirectory() {
    static const int N = 2048;
    char buffer[N];

    (void)_getcwd(buffer, N);
    return String(buffer);
}


static Set<String> s_filesUsed;
static std::atomic<bool> s_markFileUsedEnabled(true);

void FileSystem::markFileUsed(const String& filename) {
    if (s_markFileUsedEnabled) {
        std::lock_guard<std::recursive_mutex> guard(s_mutex);
        debugAssert(filename != "c:");
        s_filesUsed.insert(filename);
    }
}

void FileSystem::setMarkFileUsedEnabled(bool enabled) {
    s_markFileUsedEnabled = enabled;
}

const Set<String>& FileSystem::usedFiles() {
    return s_filesUsed;
}

void FileSystem::clearUsedFiles() {
    s_filesUsed.clear();
}

bool FileSystem::_isNewer(const String& _src, const String& _dst) {
    const String& src = FilePath::expandEnvironmentVariables(_src);
    const String& dst = FilePath::expandEnvironmentVariables(_dst);

    // TODO: work with cache and zipfiles
    struct _stat sts;
    bool sexists = _stat(src.c_str(), &sts) != -1;

    struct _stat dts;
    bool dexists = _stat(dst.c_str(), &dts) != -1;

    return sexists && ((! dexists) || (sts.st_mtime > dts.st_mtime));
}


int64 FileSystem::_size(const String& _filename) {
    const String& filename = FilePath::canonicalize(FilePath::expandEnvironmentVariables(_filename));

    struct stat64 st;
    int result = stat64(filename.c_str(), &st);
    
    if (result == -1) {
        String zip, contents;
        if (zipfileExists(filename, zip, contents)) {
            int64 requiredMem;
            
            struct zip *z = zip_open( zip.c_str(), ZIP_CHECKCONS, nullptr );
            debugAssertM(z != nullptr, zip + ": zip open failed.");
            {
                struct zip_stat info;
                zip_stat_init( &info );    // Docs unclear if zip_stat_init is required.
                int success = zip_stat( z, contents.c_str(), ZIP_FL_NOCASE, &info );
                (void) success;
                debugAssertM(success == 0, zip + ": " + contents + ": zip stat failed.");
                requiredMem = info.size;
            }
            zip_close(z);
            return requiredMem;
        } else {
            return -1;
        }
    }
    
    return st.st_size;
}


void FileSystem::listHelper
(const String&  shortSpec,
 const String&  parentPath, 
 Array<String>& result, 
 const ListSettings& settings) {

    Dir& dir = getContents(parentPath, false);

    if (! dir.exists) {
        return;
    }

    for (int i = 0; i < dir.nodeArray.size(); ++i) {
        Entry& entry = dir.nodeArray[i];
        // See if it matches the spec
        if (FilePath::matches(entry.name, shortSpec, settings.caseSensitive)) {

            if ((entry.type == UNKNOWN) && 
                (! (settings.files && settings.directories) ||
                 settings.recursive)) {
                // Update the type: it is unknown and we'll need to branch onit below
                entry.type = isDirectory(FilePath::concat(parentPath, entry.name), false) ? DIR_TYPE : FILE_TYPE;
            }
            
            if ((settings.files && settings.directories) ||
                (settings.files && (entry.type == FILE_TYPE)) ||
                (settings.directories && (entry.type == DIR_TYPE))) {
                
                if (settings.includeParentPath) {
                    result.append(FilePath::concat(parentPath, entry.name));
                } else {
                    result.append(entry.name);
                }
            }
        } // match
  
        if (settings.recursive) {
            if (entry.type == UNKNOWN) {
                entry.type = isDirectory(FilePath::concat(parentPath, entry.name), false) ? DIR_TYPE : FILE_TYPE;
            }

            if (entry.type == DIR_TYPE) {
                listHelper(shortSpec, FilePath::concat(parentPath, entry.name), result, settings);
            }
        }
    } // for
}


void FileSystem::_list(const String& _spec, Array<String>& result, const ListSettings& settings) {
    const String& spec = FilePath::expandEnvironmentVariables(_spec);

    const String& shortSpec = FilePath::baseExt(spec);
    const String& parentPath = FilePath::parent(spec);

    listHelper(shortSpec, parentPath, result, settings);
}



#ifdef G3D_WINDOWS
const Array<String>& FileSystem::_drives() {
    if (m_winDrive.length() == 0) {
        // See http://msdn.microsoft.com/en-us/library/aa364975(VS.85).aspx
        static const size_t bufSize = 5000;
        char bufData[bufSize];
        GetLogicalDriveStringsA(bufSize, bufData);

        // Drive list is a series of nullptr-terminated strings, itself terminated with a nullptr.
        for (size_t i = 0; bufData[i] != '\0'; ++i) {
            const char* thisString = bufData + i;
            m_winDrive.append(toLower(thisString));
            i += strlen(thisString);
        }
    }

    return m_winDrive;
}
#endif


String FileSystem::tempFilename(const String& basePath) {
#   ifdef G3D_WINDOWS
        char name[2048];
        GetTempFileNameA(basePath.c_str(), "tmp", 0, name);
        return name;
#   elif defined(G3D_OSX)
        // We can't use mkdtemp because the caller expects an actual
        // filename in basePath and may modify the name before creating the file.
        //
        // For example, when saving a temporary backup before copying
        // over the real save name.
        //
        // We can't use mkstemp because the caller wants a name that can
        // be modified, and may not create the file right away. mkstemp
        // returns a file descriptor instead of a name.
        const String s = FilePath::concat(basePath, "tmpXXXXXX");
        char name[2048];
        strcpy(name, s.c_str());
        mktemp(name);
        return String(name);
#   else
        // Work around clang/gcc warning. Unfortunately, the
        // OS won't know to delete this file.
        String s = FilePath::concat(basePath, "tmp");
        for (int i = 0; i < 6; ++i) {
            s += '0' + (rand() % 10);
        }
        return s;        
#   endif
}

/////////////////////////////////////////////////////////////////////

bool FilePath::isRoot(const String& f) {
#   ifdef G3D_WINDOWS
        if (f.length() < 2) {
            return false;
        }

        if (f[1] == ':') {
            if (f.length() == 2) {
                // e.g.,   "x:"
                return true;
            } else if ((f.length() == 3) && isSlash(f[2])) {
                // e.g.,   "x:\"
                return true;
            }
        }

        // Windows shares are considered roots, but only if this does not include a path inside the share
        if (isSlash(f[0]) && isSlash(f[1])) {
            size_t i = f.find("/", 3);
            size_t j = f.find("\\", 3);

            if (i == String::npos) {
                i = j;
            }

            // e.g., "\\foo\", "\\foo"
            return ((i == String::npos) || (i == f.length() - 1));
        }
#   else
        if (f == "/") {
            return true;
        }
#   endif

    return false;
}


String FilePath::removeTrailingSlash(const String& f) {
    bool removeSlash = ((endsWith(f, "/") || endsWith(f, "\\"))) && ! isRoot(f);

    return removeSlash ? f.substr(0, f.length() - 1) : f;
}


String FilePath::concat(const String& dirname, const String& file) {
    // Ensure that the directory ends in a slash
    if (! dirname.empty() && 
        ! isSlash(dirname[dirname.size() - 1]) &&
        (dirname[dirname.size() - 1] != ':')) {
        return dirname + '/' + file;
    } else {
        return dirname + file;
    }
}


String FilePath::ext(const String& filename) {
    size_t i = filename.rfind(".");
    if (i != String::npos) {
        return filename.substr(i + 1, filename.length() - i);
    } else {
        return "";
    }
}


String FilePath::baseExt(const String& filename) {
    size_t i = findLastSlash(filename);

#   ifdef G3D_WINDOWS
        size_t j = filename.rfind(":");
        if ((i == String::npos) && (j != String::npos)) {
            i = j;
        }
#   endif

    if (i == String::npos) {
        return filename;
    } else {
        return filename.substr(i + 1, filename.length() - i);
    }
}


String FilePath::base(const String& path) {
    String filename = baseExt(path);
    size_t i = filename.rfind(".");
    if (i == String::npos) {
        // No extension
        return filename;
    } else {
        return filename.substr(0, i);
    }
}


String FilePath::parent(const String& path) {    
    size_t i = findLastSlash(removeTrailingSlash(path));

#   ifdef G3D_WINDOWS
        size_t j = path.rfind(":");
        if ((i == String::npos) && (j != String::npos)) {
            i = j;
        }
#   endif

    if (i == String::npos) {
        return "";
    } else {
        return path.substr(0, i + 1);
    }
}


bool FilePath::containsWildcards(const String& filename) {
    return (filename.find('*') != String::npos) || (filename.find('?') != String::npos);
}


bool FilePath::matches(const String& path, const String& pattern, bool caseSensitive) {
    int flags = FNM_PERIOD | FNM_NOESCAPE | FNM_PATHNAME;
    if (!  caseSensitive) {
        flags |= FNM_CASEFOLD;
    }
    return g3dfnmatch(pattern.c_str(), path.c_str(), flags) == 0;
}


static int fixslash(int c) {
    return (c == '\\') ? '/' : c;
}


String FilePath::canonicalize(String x) {
    std::transform(x.begin(), x.end(), x.begin(), fixslash);
    return x;
}

void FilePath::parse
(const String&     filename,
 String&           root,
 Array<String>&    path,
 String&           base,
 String&           ext) {

    String f = filename;

    root = "";
    path.clear();
    base = "";
    ext = "";

    if (f == "") {
        // Empty filename
        return;
    }

    // See if there is a root/drive spec.
    if ((f.size() >= 2) && (f[1] == ':')) {
        
        if ((f.size() > 2) && isSlash(f[2])) {
        
            // e.g.  c:\foo
            root = f.substr(0, 3);
            f = f.substr(3, f.size() - 3);
        
        } else {
        
            // e.g.  c:foo
            root = f.substr(2);
            f = f.substr(2, f.size() - 2);

        }

    } else if ((f.size() >= 2) && isSlash(f[0]) && isSlash(f[1])) {
        
        // e.g. //foo
        root = f.substr(0, 2);
        f = f.substr(2, f.size() - 2);

    } else if (isSlash(f[0])) {
        
        root = f.substr(0, 1);
        f = f.substr(1, f.size() - 1);

    }

    // Pull the extension off
    {
        // Find the period
        size_t i = f.rfind('.');

        if (i != String::npos) {
            // Make sure it is after the last slash!
        size_t j = findLastSlash(f);
            if ((j == String::npos) || (i > j)) {
                ext = f.substr(i + 1, f.size() - i - 1);
                f = f.substr(0, i);
            }
        }
    }

    // Pull the basename off
    {
        // Find the last slash
        size_t i = findLastSlash(f);
        
        if (i == String::npos) {
            
            // There is no slash; the basename is the whole thing
            base = f;
            f    = "";

        } else if ((i != String::npos) && (i < f.size() - 1)) {
            
            base = f.substr(i + 1, f.size() - i - 1);
            f    = f.substr(0, i);

        }
    }

    // Parse what remains into path.
    size_t prev, cur = 0;

    while (cur < f.size()) {
        prev = cur;
        
        // Allow either slash
        size_t i = f.find('/', prev + 1);
        size_t j = f.find('\\', prev + 1);
        if (i == String::npos) {
            i = f.size();
        }

        if (j == String::npos) {
            j = f.size();
        }

        cur = min(i, j);

        if (cur == String::npos) {
            cur = f.size();
        }

        path.append(f.substr(prev, cur - prev));
        ++cur;
    }
}


String FilePath::mangle(const String& filename) {
    String outputFilename;
    for (size_t i = 0; i < filename.size(); ++i) {
        switch (filename[i]) {
        case '(':
            outputFilename += "_9";
            break;
        case ')':
            outputFilename += "_0";
            break;
        case ':':
            outputFilename += "_c";
            break;
        case ';':
            outputFilename += "_l";
            break;
        case ' ':
            outputFilename += "__";
            break;
        case '"':
            outputFilename += "_y";
            break;
        case '\'':
            outputFilename += "_z";
            break;
        case '/':
            outputFilename += "_s";
            break;
        case '\\':
            outputFilename += "_b";
            break;
        case '.':
            outputFilename += "_p";
            break;
        case '*':
            outputFilename += "_a";
            break;
        case '?':
            outputFilename += "_q";
            break;
        case '_':
            outputFilename += "_u";
            break;
        default:
            outputFilename += filename[i];
            break;
        }
    }
    return outputFilename;
}

String FilePath::unMangle(const String& mangledFilename) {
    String outputFilename;
    for (size_t i = 0; i < mangledFilename.size() - 1; ++i) {
        char current = mangledFilename[i];
        if (mangledFilename[i] == '_' && (i < mangledFilename.size() - 1)) {
            switch (mangledFilename[i + 1]) {
            case 'c':
                current = ':';
                ++i;
                break;
            case 'l':
                current = ';';
                ++i;
                break;
            case '_':
                current = ' ';
                ++i;
                break;
            case 'y':
                current = '"';
                ++i;
                break;
            case 'z':
                current = '\'';
                ++i;
                break;
            case 's':
                current = '/';
                ++i;
                break;
            case 'b':
                current = '\\';
                ++i;
                break;
            case 'p':
                current = '.';
                ++i;
                break;
            case 'a':
                current = '*';
                ++i;
                break;
            case 'q':
                current = '?';
                ++i;
                break;
            case 'u':
                current = '_';
                ++i;
                break;
            case '9':
                current = '(';
                ++i;
                break;
            case '0':
                current = ')';
                ++i;
                break;
            default:
                break;
            }
        }
        outputFilename += current;
    }
    return outputFilename;
}


String FilePath::expandEnvironmentVariables(const String& path) {
    // Search for pattern
    size_t end = path.find_first_of('$', 0);
    if (end == String::npos) {
        // Pattern does not exist
        return path;
    }

    size_t start = 0;
    String result;
    while (end != String::npos) {
        const String& before = path.substr(start, end - start);
        result += before;
        start = end + 1;
        String var;
        if (path[start] == '(') {
            // Search for close paren
            end = path.find_first_of(')', start + 1);
            if (end == String::npos) {
                throw String("Missing close paren in environment variable in \"") + path + "\"";
            }
            var = path.substr(start + 1, end - start - 1);
        } else {
            // Search for slash or end of string
            end = path.find_first_of('/', start);
            size_t i = path.find_first_of('\\', start);
            if ((size_t(i) != String::npos) && ((end == String::npos) || (size_t(i) < end))) {
                end = i;
            }
            if (end == String::npos) {
                // If the varible goes to the end of the string, it is the rest of the string
                end = path.size();
            } else {
                --end;
            }
            var = path.substr(start, end - start + 1);
        }

        if (! var.empty()) {
            const char* value = getenv(var.c_str());

            if (value == nullptr) {
                throw (String("System environment variable \"") + var + "\" not defined when expanding file path \"" + path + "\"");
            } else {
                result += value;
            }
        } else {
            // We just parsed an "empty" variable, which was probably a default share on Windows, e.g.,
            // "\\mycomputer\c$", and not a variable name.
            result += "$";            
        }

        start = end + 1;
        end = path.find_first_of('$', start);
    }

    // Paste on the remainder of the source path
    if (start < path.size()) {
        result += path.substr(start);
    }

    return result;
}


/** Generate a filename based on the provided hint */
String FilePath::makeLegalFilename(const String& f, size_t maxLength) {
    const static String LEGAL = "-+=_.%";
    String tentative;

    for (size_t i = 0; i < G3D::min(maxLength, f.size()); ++i) {
        const char c = f[i];
        if (isLetter(c) || isDigit(c) || (LEGAL.find(c) != String::npos)) {
            tentative += c;
        } else if ((tentative.length() == 0) || (tentative[tentative.length() - 1] != '_')) {
            // Avoid annoying repeated "_" characters resulting from legalification
            tentative += "_";
        }
    }

    return tentative;
}

bool FilePath::isLegalFilename(const String& f, size_t maxLength) {
    for (size_t i = 0; i < G3D::min(maxLength, f.size()); ++i) {
        const char c = f[i];
        if (isLetter(c) || isDigit(c) || (c == '-') || (c == '+') || (c == '=') || (c == '_') || (c == '.') || (c == '%') || (c == '~')) {
            continue;
        }
        return false;
    }
    return true;
}

}
