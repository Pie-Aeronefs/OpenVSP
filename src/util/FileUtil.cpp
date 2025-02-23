//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "FileUtil.h"
#include "tinydir.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>    /* _NSGetExecutablePath */
#endif

#ifdef WIN32
#include <windows.h>
#include <Shlwapi.h>
#include <direct.h>
#else
#include <unistd.h>
#include <libgen.h>
#include <pwd.h>
#endif

#ifdef __FreeBSD__
#include <libgen.h>
#endif

vector< string > ScanFolder( const char* dir_path )
{
    vector< string > file_vec;
    tinydir_dir dir;

    //==== Open Path ====//
    if ( tinydir_open( &dir, dir_path ) == -1 )
    {
        tinydir_close( &dir );
        return file_vec;
    }

    //==== Load Files ====//
    while ( dir.has_next )
    {
        tinydir_file file;
        if ( tinydir_readfile( &dir, &file ) == -1 )
        {
        }
        else if ( file.is_dir )
        {
        }
        else
        {
            file_vec.emplace_back( file.name );
        }
        tinydir_next( &dir );
    }
    return file_vec;

}

int ScanFolder()
{
    tinydir_dir dir;
    if ( tinydir_open( &dir, "." ) == -1 )
    {
        perror( "Error opening file" );
        goto bail;
    }

    while ( dir.has_next )
    {
        tinydir_file file;
        if ( tinydir_readfile( &dir, &file ) == -1 )
        {
            perror( "Error getting file" );
            goto bail;
        }

        printf( "%s", file.name );
        if ( file.is_dir )
        {
            printf( "/" );
        }
        printf( "\n" );

        tinydir_next( &dir );
    }

bail:
    tinydir_close( &dir );
    return 0;
}


//DIR *dir;
//struct dirent *ent;
//if ((dir = opendir ("c:\\")) != NULL)
//{
//  /* print all the files and directories within directory */
//  while ((ent = readdir (dir)) != NULL)
//  {
//      printf ("%s\n", ent->d_name);
//  }
//  closedir (dir);
//}
//else
//{
//  /* could not open directory */
//  perror ("");
//  return EXIT_FAILURE;
//}
//return 0;

string PathToExe()
{

    int bufsize = 255;
    char *path = NULL;
    bool done = false;

#ifdef __FreeBSD__
    char exepath[PATH_MAX];

    ::realpath("/proc/curproc/file", exepath);

    return dirname(exepath);
#endif

// Pre-loop initialization.
#ifdef WIN32
    // Will contain exe path
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL)
    {
#endif

    while( !done )
    {
        path = (char*) realloc( path, bufsize * sizeof(char));

#ifdef WIN32
        DWORD size = GetModuleFileName(hModule, path, bufsize);
        if( size < bufsize )
        {
            char *cpath = (char*) malloc( 2 * bufsize * sizeof(char) );
            PathCanonicalize( cpath, path );
            if ( cpath )
            {
                free( path );
                path = cpath;
            }
            PathRemoveFileSpec( path );
            done = true;
        }
#else

#ifdef __APPLE__
        uint32_t size = bufsize;
        if (_NSGetExecutablePath(path, &size) == 0)
        {
            done = true;
        }
#else
        ssize_t size = readlink("/proc/self/exe", path, bufsize);
        if ( size < bufsize - 1 )
        {
            path[size] = '\0';
            done = true;
        }
#endif
        if ( done )
        {
            char* cpath = NULL;
            cpath = realpath( path, NULL );
            if ( cpath )
            {
                free( path );
                path = cpath;
            }

            char *ppath = (char*) malloc( 2 * bufsize * sizeof(char) );
            strcpy( ppath, path );
            ppath = dirname( ppath );
            if ( ppath )
            {
                free( path );
                path = ppath;
            }
        }
#endif

        bufsize *= 2;
    }  // while( !done )

// Handle pre-loop initialization failure.
#ifdef WIN32
    }
    else
    {
        path = (char*) realloc( path, bufsize * sizeof(char));
        path[0] = '\0';
    }
#endif

    string spath( path );

    free( path );

    return spath;
}

string PathToHome()
{
#ifdef WIN32
    char * homedir;
    homedir = getenv( "USERPROFILE" );

    if ( !homedir )
    {
        char * drive = getenv( "HOMEDRIVE" );
        char * path = getenv( "HOMEPATH" );
        if ( drive && path )
        {
            return string( drive ) + string( path );
        }
        return string();
    }
    else
    {
        return string( homedir );
    }
#else
    char * homedir;
    homedir = getenv( "HOME" );

    if ( !homedir )
    {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd)
        {
            homedir = pwd->pw_dir;
        }
    }

    if( homedir )
    {
        return string( homedir );
    }
    return string();
#endif
}

string PathToCWD()
{
    char buff[FILENAME_MAX];
#ifdef WIN32
    _getcwd( buff, FILENAME_MAX );
#else
    getcwd( buff, FILENAME_MAX );
#endif
    return string( buff );
}

bool CheckForFile( const string &path, const string &file )
{
    string pathfile = path + string( "/" ) + file;
    return FileExist( pathfile );
}

bool FileExist( const string & file )
{
    FILE *fp = NULL;

    fp = fopen( file.c_str(), "r" );

    if( fp )
    {
        fclose( fp );
        return true;
    }
    else
    {
        return false;
    }
}

// This is similar to basename() on linux and returns the last portion of the pathfile string
string GetFilename( const string &pathfile )
{
    /*  _splitpath is defined on windows only
    char drive[1000];       // Drive     : Output
    char dir[1000];         // Directory : Output
    char fname[1000];       // Filename  : Output
    char ext[1000];         // Extension : Output
    _splitpath(pathfile.c_str(),drive,dir,fname,ext);
    strcat(fname,ext);
    return fname;
    */

    string sepToken;
    sepToken = "/";

    // split headerstr into fieldnames
    std::vector<string>fileParts;
    int n_parts = 0;
    char * pch;
    char strbuf[1000];
    strcpy( strbuf, pathfile.c_str() );
    pch = strtok ( strbuf, sepToken.c_str() );
    while ( pch != NULL )
    {
        n_parts++;
        fileParts.emplace_back( pch );
        pch = strtok ( NULL, sepToken.c_str() );
    }

    return fileParts.back();

}
