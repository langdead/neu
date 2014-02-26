/*================================= Neu =================================
 
 Copyright (c) 2013-2014, Andrometa LLC
 All rights reserved.
 
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
 
 =======================================================================*/

#include <neu/NPLParser.h>

#include "NPLParser_.h"

using namespace std;
using namespace neu;

namespace{
  
  class Global{
  public:
    Global(){
      nameMap_("for") = true;
      nameMap_("if") = true;
      nameMap_("else") = true;
      nameMap_("while") = true;
      nameMap_("return") = true;
      nameMap_("throw") = true;
      nameMap_("true") = true;
      nameMap_("false") = true;
      nameMap_("break") = true;
      nameMap_("continue") = true;
      nameMap_("new") = true;
      nameMap_("create") = true;
      nameMap_("delete") = true;
      nameMap_("this") = true;
      nameMap_("try") = true;
      nameMap_("catch") = true;
      
      typeMap_("bool") = nml("[size:1]");
      typeMap_("char") = nml("[size:1]");
      typeMap_("uchar") = nml("[size:1]");
      typeMap_("short") = nml("[size:2]");
      typeMap_("ushort") = nml("[size:2]");
      typeMap_("int") = nml("[size:4]");
      typeMap_("uint") = nml("[size:4]");
      typeMap_("long") = nml("[size:8]");
      typeMap_("ulong") = nml("[size:8]");
      typeMap_("float") = nml("[size:4]");
      typeMap_("double") = nml("[size:8]");
      typeMap_("var") = nml("[size:9]");
    }
    
    bool isReservedName(const nstr& name) const{
      return nameMap_.hasKey(name);
    }
    
    const nvar& getType(const nstr& t){
      return typeMap_.get(t, none);
    }
    
    nvar nameMap_;
    nvar typeMap_;
  };
  
  Global _global;
  
} // end namespace

const nvar& NPLParser_::getType(const nstr& t){
  return _global.getType(t);
}

NPLParser::NPLParser(){
  x_ = new NPLParser_(this);
}

NPLParser::~NPLParser(){
  delete x_;
}

nvar NPLParser::parse(const nstr& code, nvar* tags){
  return x_->parse(code, tags);
}

nvar NPLParser::parseFile(const nstr& path, nvar* tags){
  return x_->parseFile(path, tags);
}

bool NPLParser::isReservedName(const nstr& name){
  return _global.isReservedName(name);
}

void NPLParser::setErrorStream(ostream& estr){
  x_->setErrorStream(estr);
}

void NPLParser::setMetadata(bool flag){
  x_->setMetadata(flag);
}

nvar NPLParser::parseType(const nstr& t){
  nstr tn = t;
  
  bool ptr;
  
  if(t.endsWith("*")){
    ptr = true;
    tn = t.substr(0, t.length() - 1);
  }
  else{
    ptr = false;
  }
  
  size_t len = 0;
  for(size_t i = 0; i < tn.length(); ++i){
    if(nstr::isDigit(tn[i])){
      len = atoi(tn.substr(i, tn.length() - i).c_str());
      tn = tn.substr(0, i);
      break;
    }
  }
  
  nvar ti = _global.getType(tn);
  
  if(ti == none){
    return none;
  }
  
  nvar ret = nfunc(tn.uppercase());

  ret.merge(ti);
  
  if(ptr){
    ret("ptr") = true;
  }
  
  if(len > 0){
    ret("len") = len;
  }
  
  return ret;
}
