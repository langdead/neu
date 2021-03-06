/*

      ___           ___           ___
     /\__\         /\  \         /\__\
    /::|  |       /::\  \       /:/  /
   /:|:|  |      /:/\:\  \     /:/  /
  /:/|:|  |__   /::\~\:\  \   /:/  /  ___
 /:/ |:| /\__\ /:/\:\ \:\__\ /:/__/  /\__\
 \/__|:|/:/  / \:\~\:\ \/__/ \:\  \ /:/  /
     |:/:/  /   \:\ \:\__\    \:\  /:/  /
     |::/  /     \:\ \/__/     \:\/:/  /
     /:/  /       \:\__\        \::/  /
     \/__/         \/__/         \/__/


The Neu Framework, Copyright (c) 2013-2015, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. If you find Neu
useful, please consider helping to support our work and the evolution
of Neu by making a donation via: http://donate.andrometa.net

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

#ifndef NEU_N_SYS_H
#define NEU_N_SYS_H

#include <neu/nvar.h>

namespace neu{
  
  class NSys{
  public:
    static nvar sysInfo();
    
    static nstr hostname();
    
    // e.g: /foo/bar/baz => baz
    static nstr basename(const nstr& path);
    
    // e.g: /foo/bar/baz => /foo/bar
    static nstr parentDir(const nstr& path);
    
    static bool makeDir(const nstr& path);
    
    static nstr tempPath();
    
    static nstr tempFilePath(const nstr& extension="");
    
    static long processId();
    
    static bool exists(const nstr& path);
    
    static nstr currentDir();
    
    static bool getEnv(const nstr& key, nstr& value);
    
    static bool setEnv(const nstr& key, const nstr& value, bool redef=true);
    
    static void setTimeZone(const nstr& zone);
    
    static bool rename(const nstr& sourcePath, const nstr& destPath);
    
    // e.g: /foo/bar/file.txt => txt
    static nstr fileExtension(const nstr& filePath);

    // e.g: /foo/bar/file.txt => file
    static nstr fileName(const nstr& filePath);

    // e.g: /foo/bar/some file.txt => /foo/bar/some\ file.txt
    static nstr normalizePath(const nstr& path);
    
    // e.g: /foo/bar/file.txt => /foo/bar
    // e.g: /foo/bar => /foo
    static nstr popPath(const nstr& path);
    
    static bool dirFiles(const nstr& dirPath, nvec& files);
    
    static nstr fileToStr(const nstr& path);
    
    // current time in fractional seconds since the UNIX epoch
    static double now();
    
    // sleep for a number of (fractional) seconds
    static void sleep(double dt);
   
    // e.g: some text with $(VAR) => some text with <the contents of env var>
    static void replaceEnvs(nstr& s);

  };
  
} // end namespace neu

#endif // NEU_N_SYS_H
