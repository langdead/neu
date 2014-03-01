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

#ifndef NEU_N_VAR_H
#define NEU_N_VAR_H

#include <atomic>
#include <ostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <limits>
#include <functional>
#include <cstdarg>

#include <neu/nstr.h>
#include <neu/nrat.h>
#include <neu/nreal.h>
#include <neu/NVector.h>
#include <neu/NList.h>
#include <neu/NMap.h>
#include <neu/NMultimap.h>
#include <neu/NObjectBase.h>
#include <neu/NError.h>

namespace neu{
  
  class nvar;
  class NObject;
  
  typedef nvar (*NFunc)(void*, const nvar&);
  
  extern const class nvar none;
  extern const class nvar undef;
  
  template <class T>
  struct nvarLess{
    bool operator()(const T& x, const T& y) const{
      return x.less(y);
    }
  };
  
  typedef NVector<nvar> nvec;
  typedef NList<nvar> nlist;
  typedef NMap<nvar, nvar, nvarLess<nvar>> nmap;
  typedef NMultimap<nvar, nvar, nvarLess<nvar>> nmmap;
  
  extern const nvec _emptyVec;
  
  class nvar{
  public:
    typedef uint8_t Type;
    
    static const Type None =                     5;
    static const Type Undefined =                6;
    static const Type False =                    7;
    static const Type True =                     8;
    static const Type Integer =                  9;
    static const Type Rational =                10;
    static const Type Float =                   11;
    static const Type Real =                    12;
    static const Type Symbol =                  13;
    static const Type String =                  14;
    static const Type StringPointer =           15;
    static const Type Binary =                  16;
    static const Type RawPointer =              17;
    static const Type ObjectPointer =           18;
    static const Type LocalObject =             19;
    static const Type SharedObject =            20;
    static const Type Vector =                  21;
    static const Type List =                    22;
    static const Type Function =                23;
    static const Type HeadSequence =            24;
    static const Type Map =                     25;
    static const Type Multimap =                26;
    static const Type HeadMap =                 27;
    static const Type SequenceMap =             28;
    static const Type HeadSequenceMap =         29;
    static const Type Pointer =                 30;
    static const Type Reference =               31;
    
    class CFunction{
    public:
      CFunction(const char* f)
      : f(f),
      fp(0),
      m(0){
        
      }
      
      CFunction(const nstr& f)
      : f(f),
      fp(0),
      m(0){
        
      }
      
      CFunction(nstr&& f, size_t n)
      : f(std::move(f)),
      v(n),
      fp(0),
      m(0){
        
      }
      
      ~CFunction(){
        if(m){
          delete m;
        }
      }
      
      CFunction* clone() const{
        CFunction* c = new CFunction(f);
        c->v = v;
        c->fp = fp;
        c->m = m ? new nmap(*m) : 0;
        
        return c;
      }
      
      nstr f;
      nvec v;
      NFunc fp;
      nmap* m;
    };
    
    class CHeadSequence{
    public:
      CHeadSequence(){
        
      }
      
      CHeadSequence(nvar* h, nvar* s)
      : h(h),
      s(s){
        
      }
      
      CHeadSequence* clone(){
        CHeadSequence* c = new CHeadSequence(new nvar(*h), new nvar(*s));
        return c;
      }
      
      void dealloc(){
        delete h;
        delete s;
      }
      
      nvar* h;
      nvar* s;
    };
    
    class CHeadMap{
    public:
      CHeadMap(){
        
      }
      
      CHeadMap(nvar* h, nvar* m)
      : h(h),
      m(m){
        
      }
      
      CHeadMap* clone(){
        CHeadMap* c = new CHeadMap(new nvar(*h), new nvar(*m));
        return c;
      }
      
      void dealloc(){
        delete h;
        delete m;
      }
      
      nvar* h;
      nvar* m;
    };
    
    class CSequenceMap{
    public:
      CSequenceMap(){
        
      }
      
      CSequenceMap(nvar* s, nvar* m)
      : s(s),
      m(m){
        
      }
      
      CSequenceMap* clone(){
        CSequenceMap* c = new CSequenceMap(new nvar(*s), new nvar(*m));
        return c;
      }
      
      void dealloc(){
        delete s;
        delete m;
      }
      
      nvar* s;
      nvar* m;
    };
    
    class CHeadSequenceMap{
    public:
      CHeadSequenceMap(){
        
      }
      
      CHeadSequenceMap(nvar* h, nvar* s, nvar* m)
      : h(h),
      s(s),
      m(m){
        
      }
      
      CHeadSequenceMap* clone(){
        CHeadSequenceMap* c =
        new CHeadSequenceMap(new nvar(*h), new nvar(*s), new nvar(*m));
        
        return c;
      }
      
      void dealloc(){
        delete h;
        delete s;
        delete m;
      }
      
      nvar* h;
      nvar* s;
      nvar* m;
    };
    
    class CReference{
    public:
      CReference(nvar* v)
      : refCount_(1),
      v(v){
        
      }
      
      ~CReference(){
        delete v;
      }
      
      bool deref(){
        return std::atomic_fetch_sub<uint32_t>(&refCount_, 1) - 1 == 0;
      }
      
      void ref(){
        std::atomic_fetch_add<uint32_t>(&refCount_, 1);
      }
      
      nvar* v;
      
    private:
      std::atomic<uint32_t> refCount_;
    };
    
    union Head{
      int64_t i;
      nrat* r;
      double d;
      nreal* x;
      nstr* s;
      void* p;
      NObjectBase* o;
      nvec* v;
      nlist* l;
      CFunction* f;
      CHeadSequence* hs;
      nmap* m;
      nmmap* mm;
      CHeadMap* hm;
      CSequenceMap* sm;
      CHeadSequenceMap* hsm;
      nvar* vp;
      CReference* ref;
    };
    
    nvar(Type t, Head h)
    : t_(t),
    h_(h){
    
    }
    
    nvar()
    : t_(Undefined){
    
    }
    
    nvar(bool x)
    : t_(x ? True : False){
    
    }
    
    nvar(uint8_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(int8_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(uint16_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(int16_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(uint32_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(int32_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(uint64_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(int64_t x)
    : t_(Integer){
      h_.i = x;
      
    }
    
#if defined (__i386__) || defined (__APPLE__)
    
    nvar(long int x)
    : t_(Integer){
      h_.i = x;
      
    }
    
    nvar(unsigned long int x)
    : t_(Integer){
      h_.i = x;
      
    }
    
#endif
    
    nvar(double x)
    : t_(Float){
      h_.d = x;
      
    }
    
    nvar(float x)
    : t_(Float){
      h_.d = x;
      
    }
    
    nvar(nvar&& x)
    : t_(x.t_),
    h_(x.h_){
      
      x.t_ = Undefined;
    }
    
    nvar(const nvar& x)
    : t_(x.t_){
      switch(t_){
        case Rational:
          h_.r = new nrat(*x.h_.r);
          break;
        case Real:
          h_.x = new nreal(*x.h_.x);
          break;
        case String:
        case Binary:
        case Symbol:
          h_.s = new nstr(*x.h_.s);
          break;
        case LocalObject:
          h_.o = x.h_.o->clone();
          break;
        case SharedObject:
          h_.o = x.h_.o;
          h_.o->ref();
          break;
        case Vector:
          h_.v = new nvec(*x.h_.v);
          break;
        case List:
          h_.l = new nlist(*x.h_.l);
          break;
        case Function:
          h_.f = x.h_.f->clone();
          break;
        case HeadSequence:
          h_.hs = x.h_.hs->clone();
          break;
        case Map:
          h_.m = new nmap(*x.h_.m);
          break;
        case Multimap:
          h_.mm = new nmmap(*x.h_.mm);
          break;
        case HeadMap:
          h_.hm = x.h_.hm->clone();
          break;
        case SequenceMap:
          h_.sm = x.h_.sm->clone();
          break;
        case HeadSequenceMap:
          h_.hsm = x.h_.hsm->clone();
          break;
        case Reference:
          h_.ref = x.h_.ref;
          h_.ref->ref();
          break;
        default:
          h_.i = x.h_.i;
          break;
      }
    }
    
    struct CopyFlag{};
    
    static const CopyFlag* Copy;
    
    nvar(const nvar& x, const CopyFlag*);
    
    nvar copy() const{
      return nvar(*this, Copy);
    }
    
    struct SymbolFlag{};
    
    static const SymbolFlag* Sym;
    
    struct FunctionFlag{};
    
    static const FunctionFlag* Func;
    
    nvar(const nstr& str)
    : t_(String){
      h_.s = new nstr(str);
    }
    
    nvar(const std::string& str)
    : t_(String){
      h_.s = new nstr(str);
    }
    
    nvar(const char* str)
    : t_(String){
      h_.s = new nstr(str);
    }
    
    nvar(nstr* str)
    : t_(StringPointer){
      h_.s = str;
    }
    
    nvar(const char* str, const SymbolFlag*)
    : t_(Symbol){
      h_.s = new nstr(str);
    }
    
    nvar(const nstr& str, const SymbolFlag*)
    : t_(Symbol){
      h_.s = new nstr(str);
    }
    
    nvar(const std::string& str, const SymbolFlag*)
    : t_(Symbol){
      h_.s = new nstr(str);
    }
    
    nvar(const nstr& str, const FunctionFlag*)
    : t_(Function){
      h_.f = new CFunction(str);
    }
    
    nvar(const std::string& str, const FunctionFlag*)
    : t_(Function){
      h_.f = new CFunction(str);
    }
    
    nvar(const char* str, const FunctionFlag*)
    : t_(Function){
      h_.f = new CFunction(str);
    }
    
    nvar(NObjectBase* o)
    : t_(ObjectPointer){
      
      h_.o = o;
    }
    
    nvar(NObjectBase* o, Type t)
    : t_(t){
      switch(t_){
        case LocalObject:
        case ObjectPointer:
          h_.o = o;
          break;
        case SharedObject:
          h_.o = o;
          h_.o->ref();
          break;
        default:
          NERROR("invalid object type");
      }
    }
    
    nvar(void* p)
    : t_(RawPointer){
      h_.p = p;
    }
    
    nvar(const nrat& r){
      if(r.denominator() == 1){
        t_ = Integer;
        h_.i = r.numerator();
      }
      else{
        t_ = Rational;
        h_.r = new nrat(r);
      }
    }
    
    nvar(const nvec& v)
    : t_(Vector){
      h_.v = new nvec(v);
    }
    
    nvar(nvec&& v)
    : t_(Vector){
      h_.v = new nvec(std::move(v));
    }
    
    nvar(std::initializer_list<nvar> il)
    : t_(Vector){
      h_.v = new nvec(il);
    }
    
    nvar(const nlist& l)
    : t_(List){
      h_.l = new nlist(l);
    }
    
    nvar(nlist&& l)
    : t_(List){
      h_.l = new nlist(std::move(l));
    }
    
    nvar(nmap&& m)
    : t_(Map){
      h_.m = new nmap(std::move(m));
    }
    
    nvar(nmmap&& m)
    : t_(Multimap){
      h_.mm = new nmmap(std::move(m));
    }
    
    nvar(const nreal& x)
    : t_(Real){
      h_.x = new nreal(x);
    }
    
    nvar(nvar* v)
    : t_(Reference){
      
      assert(v->t_ != Reference && v->t_ != Pointer);
      
      h_.ref = new CReference(v);
    }
    
    struct PointerFlag{};
    
    static const PointerFlag* Ptr;
    
    nvar(nvar* v, const PointerFlag*)
    : t_(Pointer){
      
      assert(v->t_ != Reference && v->t_ != Pointer);
      
      h_.vp = v;
    }
    
    struct HeadSequenceFlag{};
    
    static const HeadSequenceFlag* HeadSequenceType;
    
    nvar(nvar&& h, nvar&& s, const HeadSequenceFlag*)
    : t_(HeadSequence){
      h_.hs = new CHeadSequence(new nvar(std::move(h)), new nvar(std::move(s)));
    }
    
    struct HeadMapFlag{};
    
    static const HeadMapFlag* HeadMapType;
    
    nvar(nvar&& h, nvar&& m, const HeadMapFlag*)
    : t_(HeadMap){
      h_.hm = new CHeadMap(new nvar(std::move(h)), new nvar(std::move(m)));
    }
    
    struct SequenceMapFlag{};
    
    static const SequenceMapFlag* SequenceMapType;
    
    nvar(nvar&& s, nvar&& m, const SequenceMapFlag*)
    : t_(SequenceMap){
      h_.sm = new CSequenceMap(new nvar(std::move(s)), new nvar(std::move(m)));
    }
    
    struct HeadSequenceMapFlag{};
    
    static const HeadSequenceMapFlag* HeadSequenceMapType;
    
    nvar(nvar&& h, nvar&& s, nvar&& m, const HeadSequenceMapFlag*)
    : t_(HeadSequenceMap){
      h_.hsm = new CHeadSequenceMap(new nvar(std::move(h)),
                                    new nvar(std::move(s)),
                                    new nvar(std::move(m)));
    }
    
    ~nvar(){
      switch(t_){
        case Rational:
          delete h_.r;
          break;
        case Real:
          delete h_.x;
          break;
        case String:
        case Binary:
        case Symbol:
          delete h_.s;
          break;
        case LocalObject:
          delete h_.o;
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          break;
        case Vector:
          delete h_.v;
          break;
        case List:
          delete h_.l;
          break;
        case Function:
          delete h_.f;
          break;
        case HeadSequence:
          h_.hs->dealloc();
          delete h_.hs;
          break;
        case Map:
          delete h_.m;
          break;
        case Multimap:
          delete h_.mm;
          break;
        case HeadMap:
          h_.hm->dealloc();
          delete h_.hm;
          break;
        case SequenceMap:
          h_.sm->dealloc();
          delete h_.sm;
          break;
        case HeadSequenceMap:
          h_.hsm->dealloc();
          delete h_.hsm;
          break;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          break;
      }
    }
    
    bool isFalse() const{
      return t_ == False;
    }
    
    bool isTrue() const{
      return t_ == True;
    }
    
    bool isDefined() const{
      switch(t_){
        case None:
        case Undefined:
          return false;
        case Vector:
        case List:
        case Map:
        case Multimap:
        case SequenceMap:
          return false;
        case HeadSequence:
          return h_.hs->h->isDefined();
        case HeadMap:
          return h_.hm->h->isDefined();
        case HeadSequenceMap:
          return h_.hsm->h->isDefined();
        case Reference:
          return h_.ref->v->isDefined();
        case Pointer:
          return h_.vp->isDefined();
        default:
          return true;
      }
    }
    
    bool some() const{
      return t_ != None;
    }
    
    bool toBool() const{
      switch(t_){
        case False:
          return false;
        case True:
          return true;
        case Integer:
          return h_.i != 0;
        case Float:
          return h_.d != 0.0;
        case Rational:
          return *h_.r != 0;
        case Real:
          return *h_.x != 0;
        case Symbol:
        case String:
        case Binary:
        case StringPointer:
          return !h_.s->empty();
        case RawPointer:
          return h_.p;
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o;
        case HeadSequence:
          return h_.hs->h->toBool();
        case HeadMap:
          return h_.hm->h->toBool();
        case HeadSequenceMap:
          return h_.hsm->h->toBool();
        case Reference:
          return h_.ref->v->toBool();
        case Pointer:
          return h_.vp;
        default:
          return false;
      }
    }
    
    typedef void (nvar::*BoolType_)() const;
    
    void boolStub_() const{}
    
    operator BoolType_() const {
      return toBool() ? &nvar::boolStub_ : 0;
    }
    
    const int64_t& asLong() const{
      switch(t_){
        case Integer:
          return h_.i;
        case HeadSequence:
          return h_.hs->h->asLong();
        case HeadMap:
          return h_.hm->h->asLong();
        case HeadSequenceMap:
          return h_.hsm->h->asLong();
        case Reference:
          return h_.ref->v->asLong();
        case Pointer:
          return h_.vp->asLong();
        default:
          NERROR("var does not hold a long");
      }
    }
    
    int64_t& asLong(){
      switch(t_){
        case Integer:
          return h_.i;
        case HeadSequence:
          return h_.hs->h->asLong();
        case HeadMap:
          return h_.hm->h->asLong();
        case HeadSequenceMap:
          return h_.hsm->h->asLong();
        case Reference:
          return h_.ref->v->asLong();
        case Pointer:
          return h_.vp->asLong();
        default:
          NERROR("var does not hold a long");
      }
    }
    
    int64_t toLong() const{
      switch(t_){
        case False:
          return 0;
        case True:
          return 1;
        case Integer:
          return h_.i;
        case Float:
          return static_cast<int64_t>(h_.d);
        case Rational:
          return h_.r->toLong();
        case Real:
          return h_.x->toLong();
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
          return atoll(h_.s->c_str());
        case HeadSequence:
          return h_.hs->h->toLong();
        case HeadMap:
          return h_.hm->h->toLong();
        case HeadSequenceMap:
          return h_.hsm->h->toLong();
        case Reference:
          return h_.ref->v->toLong();
        case Pointer:
          return h_.vp->toLong();
        default:
          NERROR("undefined operand");
      }
    }
    
    const double& asDouble() const{
      switch(t_){
        case Float:
          return h_.d;
        case HeadSequence:
          return h_.hs->h->asDouble();
        case HeadMap:
          return h_.hm->h->asDouble();
        case HeadSequenceMap:
          return h_.hsm->h->asDouble();
        case Reference:
          return h_.ref->v->asDouble();
        case Pointer:
          return h_.vp->asDouble();
        default:
          NERROR("var does not hold a double");
      }
    }
    
    double& asDouble(){
      switch(t_){
        case Float:
          return h_.d;
        case HeadSequence:
          return h_.hs->h->asDouble();
        case HeadMap:
          return h_.hm->h->asDouble();
        case HeadSequenceMap:
          return h_.hsm->h->asDouble();
        case Reference:
          return h_.ref->v->asDouble();
        case Pointer:
          return h_.vp->asDouble();
        default:
          NERROR("var does not hold a double");
      }
    }
    
    double toDouble() const{
      switch(t_){
        case False:
          return 0.0;
        case True:
          return 1.0;
        case Integer:
          return h_.i;
        case Float:
          return h_.d;
        case Rational:
          return h_.r->toDouble();
        case Real:
          return h_.x->toDouble();
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
          return atof(h_.s->c_str());
        case HeadSequence:
          return h_.hs->h->toDouble();
        case HeadMap:
          return h_.hm->h->toDouble();
        case HeadSequenceMap:
          return h_.hsm->h->toDouble();
        case Reference:
          return h_.ref->v->toDouble();
        case Pointer:
          return h_.vp->toDouble();
        default:
          NERROR("undefined operand");
      }
    }
    
    const nrat& rat() const{
      switch(t_){
        case Rational:
          return *h_.r;
        case HeadSequence:
          return h_.hs->h->rat();
        case HeadMap:
          return h_.hm->h->rat();
        case HeadSequenceMap:
          return h_.hsm->h->rat();
        case Reference:
          return h_.ref->v->rat();
        case Pointer:
          return h_.vp->rat();
        default:
          NERROR("var does not hold a rational");
      }
    }
    
    nrat& rat(){
      switch(t_){
        case Rational:
          return *h_.r;
        case HeadSequence:
          return h_.hs->h->rat();
        case HeadMap:
          return h_.hm->h->rat();
        case HeadSequenceMap:
          return h_.hsm->h->rat();
        case Reference:
          return h_.ref->v->rat();
        case Pointer:
          return h_.vp->rat();
        default:
          NERROR("var does not hold a rational");
      }
    }
    
    nrat toRat() const{
      switch(t_){
        case Integer:
          return h_.i;
        case Float:
          return nrat::fromDouble(h_.d);
        case Rational:
          return *h_.r;
        case Real:
          return h_.x->toRat();
        case HeadSequence:
          return h_.hs->h->toRat();
        case HeadMap:
          return h_.hm->h->toRat();
        case HeadSequenceMap:
          return h_.hsm->h->toRat();
        case Reference:
          return h_.ref->v->toRat();
        case Pointer:
          return h_.vp->toRat();
        default:
          NERROR("invalid operand");
      }
    }
    
    const nreal& real() const{
      switch(t_){
        case Real:
          return *h_.x;
        case HeadSequence:
          return h_.hs->h->real();
        case HeadMap:
          return h_.hm->h->real();
        case HeadSequenceMap:
          return h_.hsm->h->real();
        case Reference:
          return h_.ref->v->real();
        case Pointer:
          return h_.vp->real();
        default:
          NERROR("var does not hold a real");
      }
    }
    
    nreal& real(){
      switch(t_){
        case Real:
          return *h_.x;
        case HeadSequence:
          return h_.hs->h->real();
        case HeadMap:
          return h_.hm->h->real();
        case HeadSequenceMap:
          return h_.hsm->h->real();
        case Reference:
          return h_.ref->v->real();
        case Pointer:
          return h_.vp->real();
        default:
          NERROR("var does not hold a real");
      }
    }
    
    nreal toReal() const{
      switch(t_){
        case Integer:
          return h_.i;
        case Float:
          return h_.d;
        case Rational:
          return *h_.r;
        case Real:
          return *h_.x;
        case HeadSequence:
          return h_.hs->h->toReal();
        case HeadMap:
          return h_.hm->h->toReal();
        case HeadSequenceMap:
          return h_.hsm->h->toReal();
        case Reference:
          return h_.ref->v->toReal();
        case Pointer:
          return h_.vp->toReal();
        default:
          NERROR("invalid operand");
      }
    }
    
    const char* c_str() const{
      return str().c_str();
    }
    
    nvar& operator*() const{
      switch(t_){
        case Reference:
          return *h_.ref->v;
        case Pointer:
          return *h_.vp;
        default:
          return const_cast<nvar&>(*this);
      }
    }
    
    nstr& str(){
      switch(t_){
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
          return *h_.s;
        case Function:
          return h_.f->f;
        case HeadSequence:
          return h_.hs->h->str();
        case HeadMap:
          return h_.hm->h->str();
        case HeadSequenceMap:
          return h_.hsm->h->str();
        case Reference:
          return h_.ref->v->str();
        case Pointer:
          return h_.vp->str();
        default:
          NERROR("var does not hold a string");
      }
    }
    
    const nstr& str() const{
      switch(t_){
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
          return *h_.s;
        case Function:
          return h_.f->f;
        case HeadSequence:
          return h_.hs->h->str();
        case HeadMap:
          return h_.hm->h->str();
        case HeadSequenceMap:
          return h_.hsm->h->str();
        case Reference:
          return h_.ref->v->str();
        case Pointer:
          return h_.vp->str();
        default:
          NERROR("var does not hold a string");
      }
    }
    
    nvar sym() const{
      switch(t_){
        case Symbol:
          return *this;
        case Function:
          return nvar(h_.f->f, Sym);
        case HeadSequence:
          return h_.hs->h->sym();
        case HeadMap:
          return h_.hm->h->sym();
        case HeadSequenceMap:
          return h_.hsm->h->sym();
        case Reference:
          return h_.ref->v->sym();
        case Pointer:
          return h_.vp->sym();
        default:
          NERROR("var does not hold a symbol");
      }
    }
    
    NObjectBase* obj() const{
      switch(t_){
        case ObjectPointer:
        case LocalObject:
        case SharedObject:
          return h_.o;
        case HeadSequence:
          return h_.hs->h->obj();
        case HeadMap:
          return h_.hm->h->obj();
        case HeadSequenceMap:
          return h_.hsm->h->obj();
        case Reference:
          return h_.ref->v->obj();
        case Pointer:
          return h_.vp->obj();
        default:
          NERROR("var does not hold an object");
      }
    }
    
    template<typename T>
    T* ptr() const{
      switch(t_){
        case RawPointer:
          return static_cast<T*>(h_.p);
        case HeadSequence:
          return h_.hs->h->ptr<T>();
        case HeadMap:
          return h_.hm->h->ptr<T>();
        case HeadSequenceMap:
          return h_.hsm->h->ptr<T>();
        case Reference:
          return h_.ref->v->ptr<T>();
        case Pointer:
          return h_.vp->ptr<T>();
        default:
          NERROR("var does not hold a pointer");
      }
    }
    
    nvec& vec(){
      switch(t_){
        case Vector:
          return *h_.v;
        case HeadSequence:
          return h_.hs->s->vec();
        case SequenceMap:
          return h_.sm->s->vec();
        case HeadSequenceMap:
          return h_.hsm->s->vec();
        case Reference:
          return h_.ref->v->vec();
        case Pointer:
          return h_.vp->vec();
        default:
          NERROR("var does not hold a vector");
      }
    }
    
    const nvec& vec() const{
      switch(t_){
        case Vector:
          return *h_.v;
        case HeadSequence:
          return h_.hs->s->vec();
        case SequenceMap:
          return h_.sm->s->vec();
        case HeadSequenceMap:
          return h_.hsm->s->vec();
        case Reference:
          return h_.ref->v->vec();
        case Pointer:
          return h_.vp->vec();
        default:
          NERROR("var does not hold a vector");
      }
    }
    
    operator const nvec&() const{
      return vec();
    }
    
    operator nvec&(){
      return vec();
    }
    
    nlist& list(){
      switch(t_){
        case List:
          return *h_.l;
        case HeadSequence:
          return h_.hs->s->list();
        case SequenceMap:
          return h_.sm->s->list();
        case HeadSequenceMap:
          return h_.hsm->s->list();
        case Reference:
          return h_.ref->v->list();
        case Pointer:
          return h_.vp->list();
        default:
          NERROR("var does not hold a list");
      }
    }
    
    const nlist& list() const{
      switch(t_){
        case List:
          return *h_.l;
        case HeadSequence:
          return h_.hs->s->list();
        case SequenceMap:
          return h_.sm->s->list();
        case HeadSequenceMap:
          return h_.hsm->s->list();
        case Reference:
          return h_.ref->v->list();
        case Pointer:
          return h_.vp->list();
        default:
          NERROR("var does not hold a list");
      }
    }
    
    const nvar& anySequence() const{
      switch(t_){
        case Vector:
        case List:
          return *this;
        case HeadSequence:
          return h_.hs->s->anySequence();
        case SequenceMap:
          return h_.sm->s->anySequence();
        case HeadSequenceMap:
          return h_.hsm->s->anySequence();
        case Reference:
          return h_.ref->v->anySequence();
        case Pointer:
          return h_.vp->anySequence();
        default:
          NERROR("var does not hold a sequence");
      }
    }
    
    nvar& anySequence(){
      switch(t_){
        case Vector:
        case List:
          return *this;
        case HeadSequence:
          return h_.hs->s->anySequence();
        case SequenceMap:
          return h_.sm->s->anySequence();
        case HeadSequenceMap:
          return h_.hsm->s->anySequence();
        case Reference:
          return h_.ref->v->anySequence();
        case Pointer:
          return h_.vp->anySequence();
        default:
          NERROR("var does not hold a sequence");
      }
    }
    
    operator const nlist&() const{
      return list();
    }
    
    operator nlist&(){
      return list();
    }
    
    nmap& map(){
      switch(t_){
        case Map:
          return *h_.m;
        case HeadMap:
          return h_.hm->m->map();
        case SequenceMap:
          return h_.sm->m->map();
        case HeadSequenceMap:
          return h_.hsm->m->map();
        case Reference:
          return h_.ref->v->map();
        case Pointer:
          return h_.vp->map();
        default:
          NERROR("var does not hold a map");
      }
    }
    
    const nmap& map() const{
      switch(t_){
        case Map:
          return *h_.m;
        case HeadMap:
          return h_.hm->m->map();
        case SequenceMap:
          return h_.sm->m->map();
        case HeadSequenceMap:
          return h_.hsm->m->map();
        case Reference:
          return h_.ref->v->map();
        case Pointer:
          return h_.vp->map();
        default:
          NERROR("var does not hold a map");
      }
    }
    
    operator const nmap&() const{
      return map();
    }
    
    operator nmap&(){
      return map();
    }
    
    nmmap& multimap(){
      switch(t_){
        case Multimap:
          return *h_.mm;
        case HeadMap:
          return h_.hm->m->multimap();
        case SequenceMap:
          return h_.sm->m->multimap();
        case HeadSequenceMap:
          return h_.hsm->m->multimap();
        case Reference:
          return h_.ref->v->multimap();
        case Pointer:
          return h_.vp->multimap();
        default:
          NERROR("var does not hold a multimap");
      }
    }
    
    const nmmap& multimap() const{
      switch(t_){
        case Multimap:
          return *h_.mm;
        case HeadMap:
          return h_.hm->m->multimap();
        case SequenceMap:
          return h_.sm->m->multimap();
        case HeadSequenceMap:
          return h_.hsm->m->multimap();
        case Reference:
          return h_.ref->v->multimap();
        case Pointer:
          return h_.vp->multimap();
        default:
          NERROR("var does not hold a multimap");
      }
    }
    
    const nvar& anyMap() const{
      switch(t_){
        case Map:
        case Multimap:
          return *this;
        case HeadMap:
          return h_.hm->m->anyMap();
        case SequenceMap:
          return h_.sm->m->anyMap();
        case HeadSequenceMap:
          return h_.hsm->m->anyMap();
        case Reference:
          return h_.ref->v->anyMap();
        case Pointer:
          return h_.vp->anyMap();
        default:
          NERROR("var does not hold a map");
      }
    }
    
    nvar& anyMap(){
      switch(t_){
        case Map:
        case Multimap:
          return *this;
        case HeadMap:
          return h_.hm->m->anyMap();
        case SequenceMap:
          return h_.sm->m->anyMap();
        case HeadSequenceMap:
          return h_.hsm->m->anyMap();
        case Reference:
          return h_.ref->v->anyMap();
        case Pointer:
          return h_.vp->anyMap();
        default:
          NERROR("var does not hold a map");
      }
    }
    
    operator const nmmap&() const{
      return multimap();
    }
    
    operator nmmap&(){
      return multimap();
    }
    
    operator double() const{
      return toDouble();
    }
    
    operator uint8_t() const{
      return toLong();
    }
    
    operator int8_t() const{
      return toLong();
    }
    
    operator uint16_t() const{
      return toLong();
    }
    
    operator int16_t() const{
      return toLong();
    }
    
    operator uint32_t() const{
      return (uint32_t)toLong();
    }
    
    operator int32_t() const{
      return (int32_t)toLong();
    }
    
    operator uint64_t() const{
      return toLong();
    }
    
    operator int64_t() const{
      return toLong();
    }
    
#if defined (__i386__) || defined (__APPLE__)
    
    operator unsigned long int() const{
      return (unsigned long int)toLong();
    }
    
    operator long int() const{
      return (long int)toLong();
    }
    
#endif
    
    operator const char*() const{
      return c_str();
    }
    
    operator nstr&(){
      return str();
    }
    
    operator const nstr&() const{
      return str();
    }
    
    operator float() const{
      return toDouble();
    }
    
    operator bool() const{
      return toBool();
    }
    
    void pushBack(const nvar& x){
      switch(t_){
        case None:
        case Undefined:
          t_ = Vector;
          h_.v = new nvec(1, x);
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.v = new nvec(1, x);
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case Vector:
          h_.v->push_back(x);
          break;
        case List:
          h_.l->push_back(x);
          break;
        case Function:
          h_.f->v.push_back(x);
          break;
        case HeadSequence:
          h_.hs->s->pushBack(x);
          break;
        case Map:{
          Head hv;
          hv.v = new nvec(1, x);
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case Multimap:{
          Head hv;
          hv.v = new nvec(1, x);
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head hv;
          hv.v = new nvec(1, x);
          
          CHeadMap* hm = h_.hm;
          h_.hsm = new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
          delete hm;
          t_ = HeadSequenceMap;
          break;
        }
        case SequenceMap:
          h_.sm->s->pushBack(x);
          break;
        case HeadSequenceMap:
          h_.hsm->s->pushBack(x);
          break;
        case Reference:
          h_.ref->v->pushBack(x);
          break;
        case Pointer:
          h_.vp->pushBack(x);
          break;
      }
    }
    
    void pushBack(nvar&& x){
      switch(t_){
        case None:
        case Undefined:
          t_ = Vector;
          h_.v = new nvec;
          h_.v->emplace_back(std::move(x));
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.v = new nvec;
          h.v->emplace_back(std::move(x));
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case Vector:
          h_.v->emplace_back(std::move(x));
          break;
        case List:
          h_.l->emplace_back(std::move(x));
          break;
        case Function:
          h_.f->v.emplace_back(std::move(x));
          break;
        case HeadSequence:
          h_.hs->s->pushBack(std::move(x));
          break;
        case Map:{
          Head hv;
          hv.v = new nvec;
          hv.v->emplace_back(std::move(x));
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case Multimap:{
          Head hv;
          hv.v = new nvec;
          hv.v->emplace_back(std::move(x));
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head hv;
          hv.v = new nvec;
          hv.v->emplace_back(std::move(x));
          
          CHeadMap* hm = h_.hm;
          h_.hsm = new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
          delete hm;
          t_ = HeadSequenceMap;
          break;
        }
        case SequenceMap:
          h_.sm->s->pushBack(std::move(x));
          break;
        case HeadSequenceMap:
          h_.hsm->s->pushBack(std::move(x));
          break;
        case Reference:
          h_.ref->v->pushBack(std::move(x));
          break;
        case Pointer:
          h_.vp->pushBack(std::move(x));
          break;
      }
    }
    
    nvar& operator<<(const nvar& x){
      switch(t_){
        case None:
        case Undefined:
          t_ = Vector;
          h_.v = new nvec(1, x);
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.v = new nvec(1, x);
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case Vector:
          h_.v->push_back(x);
          break;
        case List:
          h_.l->push_back(x);
          break;
        case Function:
          h_.f->v.push_back(x);
          break;
        case HeadSequence:
          *h_.hs->s << x;
          break;
        case Map:{
          Head hv;
          hv.v = new nvec(1, x);
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case Multimap:{
          Head hv;
          hv.v = new nvec(1, x);
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head hv;
          hv.v = new nvec(1, x);
          
          CHeadMap* hm = h_.hm;
          h_.hsm = new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
          delete hm;
          t_ = HeadSequenceMap;
          break;
        }
        case SequenceMap:
          *h_.sm->s << x;
          break;
        case HeadSequenceMap:
          *h_.hsm->s << x;
          break;
        case Reference:
          *h_.ref->v << x;
          break;
        case Pointer:
          *h_.vp << x;
          break;
      }
      
      return *this;
    }
    
    nvar& operator<<(nvar&& x){
      switch(t_){
        case None:
        case Undefined:
          t_ = Vector;
          h_.v = new nvec;
          h_.v->emplace_back(std::move(x));
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.v = new nvec;
          h.v->emplace_back(std::move(x));
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case Vector:
          h_.v->emplace_back(std::move(x));
          break;
        case List:
          h_.l->emplace_back(std::move(x));
          break;
        case Function:
          h_.f->v.emplace_back(std::move(x));
          break;
        case HeadSequence:
          *h_.hs->s << x;
          break;
        case Map:{
          Head hv;
          hv.v = new nvec;
          hv.v->emplace_back(std::move(x));
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case Multimap:{
          Head hv;
          hv.v = new nvec;
          hv.v->emplace_back(std::move(x));
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head hv;
          hv.v = new nvec;
          hv.v->emplace_back(std::move(x));
          
          CHeadMap* hm = h_.hm;
          h_.hsm = new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
          delete hm;
          t_ = HeadSequenceMap;
          break;
        }
        case SequenceMap:
          *h_.sm->s << std::move(x);
          break;
        case HeadSequenceMap:
          *h_.hsm->s << std::move(x);
          break;
        case Reference:
          *h_.ref->v << std::move(x);
          break;
        case Pointer:
          *h_.vp << std::move(x);
          break;
      }
      
      return *this;
    }
    
    nvar& operator<<(int x){
      return operator<<(nvar(x));
    }
    
    nvar& operator<<(double x){
      return operator<<(nvar(x));
    }
    
    nvar& operator<<(long long x){
      return operator<<(nvar(x));
    }
    
    void pushFront(const nvar& x){
      switch(t_){
        case None:
        case Undefined:
          t_ = Vector;
          h_.v = new nvec(1, x);
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.v = new nvec(1, x);
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case Vector:
          h_.v->pushFront(x);
          break;
        case List:
          h_.l->push_front(x);
          break;
        case Function:
          h_.f->v.pushFront(x);
          break;
        case HeadSequence:
          h_.hs->s->pushFront(x);
          break;
        case Map:{
          Head hv;
          hv.v = new nvec(1, x);
          
          Head hm;
          hm.m = h_.m;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
          t_ = SequenceMap;
          break;
        }
        case Multimap:{
          Head hv;
          hv.v = new nvec(1, x);
          
          Head hm;
          hm.mm = h_.mm;
          
          h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head hv;
          hv.v = new nvec(1, x);
          
          CHeadMap* hm = h_.hm;
          h_.hsm = new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
          delete hm;
          t_ = HeadSequenceMap;
          break;
        }
        case SequenceMap:
          h_.sm->s->pushFront(x);
          break;
        case HeadSequenceMap:
          h_.hsm->s->pushFront(x);
          break;
        case Reference:
          h_.ref->v->pushFront(x);
          break;
        case Pointer:
          h_.vp->pushFront(x);
          break;
      }
    }
    
    void append(const nvar& x);
    
    nvar popBack(){
      switch(t_){
        case Vector:
          return h_.v->popBack();
        case List:
          return h_.l->popBack();
        case Function:
          return h_.f->v.popBack();
        case HeadSequence:
          return h_.hs->s->popBack();
        case SequenceMap:
          return h_.sm->s->popBack();
        case HeadSequenceMap:
          return h_.hsm->s->popBack();
        case Reference:
          return h_.ref->v->popBack();
        case Pointer:
          return h_.vp->popBack();
        default:
          NERROR("var does not hold a sequence");
      }
    }
    
    nvar popFront(){
      switch(t_){
        case Vector:
          return h_.v->popFront();
        case List:
          return h_.l->popFront();
        case Function:
          return h_.f->v.popFront();
        case HeadSequence:
          return h_.hs->s->popFront();
        case SequenceMap:
          return h_.sm->s->popFront();
        case HeadSequenceMap:
          return h_.hsm->s->popFront();
        case Reference:
          return h_.ref->v->popFront();
        case Pointer:
          return h_.vp->popFront();
        default:
          NERROR("var does not hold a sequence");
      }
    }
    
    nvar& add(const nvar& x){
      pushBack(x);
      return *this;
    }
    
    nvar& add(const nvar& k, const nvar& v){
      (*this)(k) = v;
      return *this;
    }
    
    Type fullType() const{
      return t_;
    }
    
    Type type() const{
      switch(t_){
        case HeadSequence:
          return h_.hs->h->type();
        case HeadMap:
          return h_.hm->h->type();
        case HeadSequenceMap:
          return h_.hsm->h->type();
        case Vector:
        case List:
        case Map:
        case Multimap:
        case SequenceMap:
          return Undefined;
        case Reference:
          return h_.ref->v->type();
        case Pointer:
          return h_.vp->type();
        default:
          return t_;
      }
    }
    
    bool normalize(){
      switch(t_){
        case Vector:
          if(h_.v->empty()){
            delete h_.v;
            t_ = Undefined;
            return true;
          }
          return false;
        case List:
          if(h_.l->empty()){
            delete h_.l;
            t_ = Undefined;
            return true;
          }
          return false;
        case HeadSequence:{
          nvar* s = h_.hs->s;
          if(s->normalize()){
            nvar* h = h_.hs->h;
            t_ = h->t_;
            h_ = h->h_;
            h->t_ = Undefined;
            delete h;
            delete s;
            return true;
          }
          return false;
        }
        case Function:
          if(h_.f->m && h_.f->m->empty()){
            delete h_.f->m;
            h_.f->m = 0;
            return true;
          }
          return false;
        case Map:
          if(h_.m->empty()){
            delete h_.m;
            t_ = Undefined;
            return true;
          }
          return false;
        case Multimap:
          if(h_.mm->empty()){
            delete h_.mm;
            t_ = Undefined;
            return true;
          }
          return false;
        case HeadMap:{
          nvar* m = h_.hm->m;
          if(m->normalize()){
            nvar* h = h_.hm->h;
            t_ = h->t_;
            h_ = h->h_;
            h->t_ = Undefined;
            delete h;
            delete m;
            return true;
          }
          return false;
        }
        case SequenceMap:{
          nvar* s = h_.sm->s;
          nvar* m = h_.sm->m;
          
          bool ns = s->normalize();
          bool nm = m->normalize();
          
          if(ns){
            delete s;
            delete h_.sm;
            if(nm){
              delete m;
              t_ = Undefined;
            }
            else{
              t_ = m->t_;
              h_ = m->h_;
              m->t_ = Undefined;
              delete m;
            }
            return true;
          }
          else if(nm){
            delete h_.sm;
            t_ = s->t_;
            h_ = s->h_;
            s->t_ = Undefined;
            delete s;
            return true;
          }
          return false;
        }
        case HeadSequenceMap:{
          nvar* h = h_.hsm->h;
          nvar* s = h_.hsm->s;
          nvar* m = h_.hsm->m;
          
          bool ns = s->normalize();
          bool nm = m->normalize();
          
          if(ns){
            delete s;
            delete h_.hsm;
            if(nm){
              delete m;
              t_ = h->t_;
              h_ = h->h_;
              h->t_ = Undefined;
              delete h;
            }
            else{
              t_ = HeadMap;
              h_.hm = new CHeadMap(h, m);
            }
            return true;
          }
          else if(nm){
            delete m;
            delete h_.hsm;
            t_ = HeadSequence;
            h_.hs = new CHeadSequence(h, s);
            return true;
          }
          return false;
        }
        case Reference:
          return h_.ref->v->normalize();
        case Pointer:
          return h_.vp->normalize();
        default:
          return false;
      }
    }
    
    bool isString() const{
      switch(t_){
        case String:
          return true;
        case HeadSequence:
          return h_.hs->h->isString();
        case HeadMap:
          return h_.hm->h->isString();
        case HeadSequenceMap:
          return h_.hsm->h->isString();
        case Reference:
          return h_.ref->v->isString();
        case Pointer:
          return h_.vp->isString();
        default:
          return false;
      }
    }
    
    bool isSymbol() const{
      switch(t_){
        case Symbol:
          return true;
        case HeadSequence:
          return h_.hs->h->isSymbol();
        case HeadMap:
          return h_.hm->h->isSymbol();
        case HeadSequenceMap:
          return h_.hsm->h->isSymbol();
        case Reference:
          return h_.ref->v->isSymbol();
        case Pointer:
          return h_.vp->isSymbol();
        default:
          return false;
      }
    }
    
    bool isSymbol(const nstr& s) const{
      switch(t_){
        case Symbol:
          return *h_.s == s;
        case HeadSequence:
          return h_.hs->h->isSymbol(s);
        case HeadMap:
          return h_.hm->h->isSymbol(s);
        case HeadSequenceMap:
          return h_.hsm->h->isSymbol(s);
        case Reference:
          return h_.ref->v->isSymbol(s);
        case Pointer:
          return h_.vp->isSymbol(s);
        default:
          return false;
      }
    }
    
    bool isFunction() const{
      switch(t_){
        case Function:
          return true;
        case HeadMap:
          return h_.hm->h->isFunction();
        case Reference:
          return h_.ref->v->isFunction();
        case Pointer:
          return h_.vp->isFunction();
        default:
          return false;
      }
    }
    
    bool isFunction(const nstr& f) const{
      switch(t_){
        case Function:
          return h_.f->f == f;
        case HeadMap:
          return h_.hm->h->isFunction(f);
        case Reference:
          return h_.ref->v->isFunction(f);
        case Pointer:
          return h_.vp->isFunction(f);
        default:
          return false;
      }
    }
    
    bool isFunction(const nstr& f, size_t arity) const{
      switch(t_){
        case Function:
          return h_.f->f == f && h_.f->v.size() == arity;
        case HeadMap:
          return h_.hm->h->isFunction(f, arity);
        case Reference:
          return h_.ref->v->isFunction(f, arity);
        case Pointer:
          return h_.vp->isFunction(f, arity);
        default:
          return false;
      }
    }

    bool isFunction(size_t minArity, size_t maxArity) const{
      switch(t_){
        case Function:{
          size_t s = h_.f->v.size();
          return s >= minArity && s <= maxArity;
        }
        case HeadMap:
          return h_.hm->h->isFunction(minArity, maxArity);
        case Reference:
          return h_.ref->v->isFunction(minArity, maxArity);
        case Pointer:
          return h_.vp->isFunction(minArity, maxArity);
        default:
          return false;
      }
    }
    
    bool isFunction(const nstr& f, size_t minArity, size_t maxArity) const{
      switch(t_){
        case Function:
          if(h_.f->f == f){
            size_t s = h_.f->v.size();
            return s >= minArity && s <= maxArity;
          }
          return false;
        case HeadMap:
          return h_.hm->h->isFunction(f, minArity, maxArity);
        case Reference:
          return h_.ref->v->isFunction(f, minArity, maxArity);
        case Pointer:
          return h_.vp->isFunction(f, minArity, maxArity);
        default:
          return false;
      }
    }
    
    bool isSymbolic() const{
      switch(t_){
        case Symbol:
        case Function:
          return true;
        case HeadSequence:
          return h_.hs->h->isSymbolic();
        case HeadMap:
          return h_.hm->h->isSymbolic();
        case HeadSequenceMap:
          return h_.hsm->h->isSymbolic();
        case Reference:
          return h_.ref->v->isSymbolic();
        case Pointer:
          return h_.vp->isSymbolic();
        default:
          return false;
      }
    }
    
    bool isNumeric() const{
      switch(t_){
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
          return true;
        case HeadSequence:
          return h_.hs->h->isNumeric();
        case HeadMap:
          return h_.hm->h->isNumeric();
        case HeadSequenceMap:
          return h_.hsm->h->isNumeric();
        case Reference:
          return h_.ref->v->isNumeric();
        case Pointer:
          return h_.vp->isNumeric();
        default:
          return false;
      }
    }
    
    bool isReference() const{
      return t_ == Reference;
    }
    
    bool isPointer() const{
      return t_ == Pointer;
    }
    
    bool isInteger() const{
      switch(t_){
        case Integer:
          return true;
        case HeadSequence:
          return h_.hs->h->isInteger();
        case HeadMap:
          return h_.hm->h->isInteger();
        case HeadSequenceMap:
          return h_.hsm->h->isInteger();
        case Reference:
          return h_.ref->v->isInteger();
        case Pointer:
          return h_.vp->isInteger();
        default:
          return false;
      }
    }
    
    int isRational() const{
      switch(t_){
        case Integer:
          return 1;
        case Rational:
          return 2;
        case HeadSequence:
          return h_.hs->h->isRational();
        case HeadMap:
          return h_.hm->h->isRational();
        case HeadSequenceMap:
          return h_.hsm->h->isRational();
        case Reference:
          return h_.ref->v->isRational();
        case Pointer:
          return h_.vp->isRational();
        default:
          return 0;
      }
    }
    
    bool isReal() const{
      switch(t_){
        case Real:
          return true;
        case HeadSequence:
          return h_.hs->h->isReal();
        case HeadMap:
          return h_.hm->h->isReal();
        case HeadSequenceMap:
          return h_.hsm->h->isReal();
        case Reference:
          return h_.ref->v->isReal();
        case Pointer:
          return h_.vp->isReal();
        default:
          return false;
      }
    }
    
    nvar head() const{
      switch(t_){
        case LocalObject:{
          Head h;
          h.o = h_.o->clone();
          return nvar(LocalObject, h);
        }
        case SharedObject:{
          Head h;
          h.o = h_.o;
          h.o->ref();
          return nvar(SharedObject, h);
        }
        case Vector:
        case List:
        case Function:
        case Map:
        case Multimap:
        case SequenceMap:
          return undef;
        case HeadSequence:
          return h_.hs->h->head();
        case HeadMap:
          return h_.hm->h->head();
        case HeadSequenceMap:
          return h_.hsm->h->head();
        case Reference:
          return h_.ref->v->head();
        case Pointer:
          return h_.vp->head();
        default:
          return nvar(t_, h_);
      }
    }
    
    void setHead(const nvar& x);
    
    void clearHead(){
      switch(t_){
        case HeadSequence:
          delete h_.hs->h;
          
          if(h_.hs->s->t_ == Vector){
            t_ = Vector;
            h_.v = h_.hs->s->h_.v;
          }
          else{
            t_ = List;
            h_.l = h_.hs->s->h_.l;
          }
          
          break;
        case HeadMap:
          delete h_.hs->h;
          
          if(h_.hm->m->t_ == Map){
            t_ = Map;
            h_.m = h_.hm->m->h_.m;
          }
          else{
            t_ = Multimap;
            h_.mm = h_.hs->s->h_.mm;
          }
          
          break;
        case HeadSequenceMap:
          delete h_.hsm->h;
          
          t_ = SequenceMap;
          h_.sm = new CSequenceMap(h_.hsm->s, h_.hsm->m);
          
          delete h_.hsm;
          
          break;
        case Rational:
          delete h_.r;
          t_ = Undefined;
          break;
        case Real:
          delete h_.x;
          t_ = Undefined;
          break;
        case String:
        case Binary:
        case Symbol:
          delete h_.s;
          t_ = Undefined;
          break;
        case LocalObject:
          delete h_.o;
          t_ = Undefined;
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          t_ = Undefined;
          break;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Undefined;
          break;
        default:
          t_ = Undefined;
          break;
      }
    }
    
    bool hasKey(const nvar& key) const{
      switch(t_){
        case Function:
          return h_.f->m && h_.f->m->hasKey(key);
        case Map:
          return h_.m->hasKey(key);
        case Multimap:
          return h_.mm->count(key) > 0;
        case HeadMap:
          return h_.hm->m->hasKey(key);
        case SequenceMap:
          return h_.sm->m->hasKey(key);
        case HeadSequenceMap:
          return h_.hsm->m->hasKey(key);
        case Reference:
          return h_.ref->v->hasKey(key);
        case Pointer:
          return h_.vp->hasKey(key);
        default:
          return false;
      }
    }
    
    bool hasKey(const nvar& key, Type t) const{
      switch(t_){
        case Function:
          if(h_.f->m){
            auto itr = h_.f->m->find(key);
            if(itr == h_.f->m->end()){
              return false;
            }
            return itr->second.t_ == t;
          }
          return false;
        case Map:{
          auto itr = h_.m->find(key);
          return itr->second.t_ == t;
        }
        case Multimap:{
          auto p = h_.mm->equal_range(key);
          auto itr = p.first;
          while(itr != p.second){
            if(itr->second.t_ == t){
              return true;
            }
            ++itr;
          }
          return false;
        }
        case HeadMap:
          return h_.hm->m->hasKey(key, t);
        case SequenceMap:
          return h_.sm->m->hasKey(key, t);
        case HeadSequenceMap:
          return h_.hsm->m->hasKey(key, t);
        case Reference:
          return h_.ref->v->hasKey(key, t);
        case Pointer:
          return h_.vp->hasKey(key, t);
        default:
          return false;
      }
    }
    
    size_t numKeys(const nvar& key){
      switch(t_){
        case Function:
          return h_.f->m && h_.f->m->hasKey(key) ? 1 : 0;
        case Map:
          return h_.m->hasKey(key) ? 1 : 0;
        case Multimap:
          return h_.mm->count(key);
        case HeadMap:
          return h_.hm->m->numKeys(key);
        case SequenceMap:
          return h_.sm->m->numKeys(key);
        case HeadSequenceMap:
          return h_.hsm->m->numKeys(key);
        case Reference:
          return h_.ref->v->numKeys(key);
        case Pointer:
          return h_.vp->numKeys(key);
        default:
          return 0;
      }
    }
    
    void insert(size_t pos, const nvar& x){
      switch(t_){
        case Vector:{
#ifndef NEU_FAST
          if(pos > h_.v->size()){
            NERROR("invalid position");
          }
#endif
          auto itr = h_.v->begin();
          advance(itr, pos);
          h_.v->insert(itr, x);
          break;
        }
        case List:{
#ifndef NEU_FAST
          if(pos > h_.l->size()){
            NERROR("invalid position");
          }
#endif
          auto itr = h_.l->begin();
          advance(itr, pos);
          h_.l->insert(itr, x);
          break;
        }
        case Function:{
          auto itr = h_.f->v.begin();
          advance(itr, pos);
          h_.f->v.insert(itr, x);
          break;
        }
        case HeadSequence:
          h_.hs->s->insert(pos, x);
          break;
        case SequenceMap:
          h_.sm->s->insert(pos, x);
          break;
        case HeadSequenceMap:
          h_.hsm->s->insert(pos, x);
          break;
        case Reference:
          h_.ref->v->insert(pos, x);
          break;
        case Pointer:
          h_.vp->insert(pos, x);
          break;
        default:
          NERROR("var does not hold a sequence");
      }
    }
    
    void clear(){
      switch(t_){
        case Vector:
          h_.v->clear();
          break;
        case List:
          h_.l->clear();
          break;
        case Function:
          h_.f->v.clear();
          if(h_.f->m){
            delete h_.f->m;
            h_.f->m = 0;
          }
          break;
        case HeadSequence:
          h_.hs->s->clear();
          break;
        case Map:
          h_.m->clear();
          break;
        case Multimap:
          h_.mm->clear();
          break;
        case HeadMap:
          h_.hm->m->clear();
          break;
        case SequenceMap:
          h_.sm->s->clear();
          h_.sm->m->clear();
          break;
        case HeadSequenceMap:
          h_.hsm->s->clear();
          h_.hsm->m->clear();
          break;
        case Reference:
          h_.ref->v->clear();
          break;
        case Pointer:
          h_.vp->clear();
          break;
        default:
          break;
      }
    }
    
    double inf(){
      return std::numeric_limits<double>::infinity();
    }
    
    double negInf(){
      return -std::numeric_limits<double>::infinity();
    }
    
    double nan(){
      return std::numeric_limits<double>::quiet_NaN();
    }
    
    double min(){
      return -std::numeric_limits<double>::max();
    }
    
    double max(){
      return std::numeric_limits<double>::max();
    }
    
    double epsilon(){
      return std::numeric_limits<double>::epsilon();
    }
    
    static const nvar& max(const nvar& a, const nvar& b){
      return a > b ? a : b;
    }
    
    static const nvar& min(const nvar& a, const nvar& b){
      return a < b ? a : b;
    }
    
    static void streamOutputVector_(std::ostream& ostr,
                                    const nvec& v,
                                    bool& first,
                                    bool concise){
      for(const nvar& vi : v){
        if(first){
          first = false;
        }
        else{
          ostr << ",";
        }
        vi.streamOutput_(ostr, concise);
      }
    }
    
    static void streamOutputList_(std::ostream& ostr,
                                  const nlist& l,
                                  bool& first,
                                  bool concise){
      for(const nvar& vi : l){
        if(first){
          first = false;
        }
        else{
          ostr << ",";
        }
        vi.streamOutput_(ostr, concise);
      }
    }
    
    static bool streamOutputMap_(std::ostream& ostr,
                                 const nmap& m,
                                 bool& first,
                                 bool concise){
      bool found = false;
      for(auto& itr : m){
        bool visible;
        const nvar& k = *itr.first;
        
        if(k.t_ == Symbol){
          const nstr& s = *k.h_.s;
          
          if(s.beginsWith("__")){
            visible = false;
          }
          else{
            visible = true;
          }
        }
        else{
          visible = true;
        }
        
        if(visible){
          if(first){
            first = false;
          }
          else{
            ostr << ", ";
          }
          
          k.streamOutput_(ostr, concise);
          ostr << ":";
          itr.second.streamOutput_(ostr, concise);
          
          found = true;
        }
      }
      
      return found;
    }
    
    static bool streamOutputMultimap_(std::ostream& ostr,
                                      const nmmap& m,
                                      bool& first,
                                      bool concise){
      bool found = false;
      for(auto& itr : m){
        bool visible;
        const nvar& k = *itr.first;
        
        if(k.t_ == Symbol){
          const nstr& s = *k.h_.s;
          
          if(s.beginsWith("__")){
            visible = false;
          }
          else{
            visible = true;
          }
        }
        else{
          visible = true;
        }
        
        if(visible){
          if(first){
            first = false;
          }
          else{
            ostr << ", ";
          }
          
          k.streamOutput_(ostr, concise);
          ostr << ":";
          itr.second.streamOutput_(ostr, concise);
          
          found = true;
        }
      }
      
      return found;
    }
    
    void streamOutput_(std::ostream& ostr, bool concise=true) const;
    
    nstr toStr(bool concise=true) const{
      std::stringstream ostr;
      
      if(!concise){
        ostr.precision(16);
      }
      
      streamOutput_(ostr, concise);
      
      return ostr.str();
    }
    
    static nvar fromStr(const nstr& str);
    
    size_t size() const{
      switch(t_){
        case Vector:
          return h_.v->size();
        case List:
          return h_.l->size();
        case Function:
          return h_.f->v.size();
        case HeadSequence:
          return h_.hs->s->size();
        case SequenceMap:
          return h_.sm->s->size();
        case HeadSequenceMap:
          return h_.hsm->s->size();
        case Reference:
          return h_.ref->v->size();
        case Pointer:
          return h_.vp->size();
        default:
          return 0;
      }
    }
    
    size_t numKeys() const{
      switch(t_){
        case Map:
          return h_.m->size();
        case Multimap:
          return h_.mm->size();
        case HeadMap:
          return h_.hm->m->size();
        case SequenceMap:
          return h_.sm->m->size();
        case HeadSequenceMap:
          return h_.hsm->m->size();
        case Reference:
          return h_.ref->v->size();
        case Pointer:
          return h_.vp->size();
        default:
          return 0;
      }
    }
    
    bool empty() const{
      switch(t_){
        case Vector:
          return h_.v->empty();
        case List:
          return h_.l->empty();
        case Function:
          return h_.f->v.empty();
        case HeadSequence:
          return h_.hs->s->empty();
        case Map:
          return h_.m->empty();
        case Multimap:
          return h_.mm->empty();
        case HeadMap:
          return h_.hm->m->empty();
        case SequenceMap:
          return h_.sm->s->empty();
        case HeadSequenceMap:
          return h_.hsm->s->empty();
        case Reference:
          return h_.ref->v->empty();
        case Pointer:
          return h_.vp->empty();
        default:
          return true;
      }
    }
    
    const nvar& back() const{
      switch(t_){
        case Vector:
          return h_.v->back();
        case List:
          return h_.l->back();
        case Function:
          return h_.f->v.back();
        case HeadSequence:
          return h_.hs->s->back();
        case SequenceMap:
          return h_.sm->s->back();
        case HeadSequenceMap:
          return h_.hsm->s->back();
        case Reference:
          return h_.ref->v->back();
        case Pointer:
          return h_.vp->back();
        default:
          NERROR("no sequence");
      }
    }
    
    nvar& back(){
      switch(t_){
        case Vector:
          return h_.v->back();
        case List:
          return h_.l->back();
        case Function:
          return h_.f->v.back();
        case HeadSequence:
          return h_.hs->s->back();
        case SequenceMap:
          return h_.sm->s->back();
        case HeadSequenceMap:
          return h_.hsm->s->back();
        case Reference:
          return h_.ref->v->back();
        case Pointer:
          return h_.vp->back();
        default:
          NERROR("no sequence");
      }
    }
    
    bool hasVector() const{
      switch(t_){
        case Vector:
          return true;
        case HeadSequence:
          return h_.hs->s->hasVector();
        case SequenceMap:
          return h_.sm->s->hasVector();
        case HeadSequenceMap:
          return h_.hsm->s->hasVector();
        case Reference:
          return h_.ref->v->hasVector();
        case Pointer:
          return h_.vp->hasVector();
        default:
          return false;
      }
    }
    
    bool hasList() const{
      switch(t_){
        case List:
          return true;
        case HeadSequence:
          return h_.hs->s->hasList();
        case SequenceMap:
          return h_.sm->s->hasList();
        case HeadSequenceMap:
          return h_.hsm->s->hasList();
        case Reference:
          return h_.ref->v->hasList();
        case Pointer:
          return h_.vp->hasList();
        default:
          return false;
      }
    }
    
    bool hasMap() const{
      switch(t_){
        case Function:
          return h_.f->m;
        case Map:
          return true;
        case HeadMap:
          return h_.hm->h->hasMap();
        case SequenceMap:
          return h_.sm->m->hasMap();
        case HeadSequenceMap:
          return h_.hsm->m->hasMap();
        case Reference:
          return h_.ref->v->hasMap();
        case Pointer:
          return h_.vp->hasMap();
        default:
          return false;
      }
    }
    
    bool hasMultimap() const{
      switch(t_){
        case Multimap:
          return true;
        case HeadMap:
          return h_.hm->m->hasMultimap();
        case SequenceMap:
          return h_.sm->m->hasMultimap();
        case HeadSequenceMap:
          return h_.hsm->m->hasMultimap();
        case Reference:
          return h_.ref->v->hasMultimap();
        case Pointer:
          return h_.vp->hasMultimap();
        default:
          return false;
      }
    }
    
    nvar& operator=(nvar&& x){
      switch(t_){
        case Rational:
          delete h_.r;
          break;
        case Real:
          delete h_.x;
          break;
        case String:
        case Binary:
        case Symbol:
          delete h_.s;
          break;
        case Vector:
          delete h_.v;
          break;
        case List:
          delete h_.l;
          break;
        case Function:
          delete h_.f;
          break;
        case Map:
          delete h_.m;
          break;
        case Multimap:
          delete h_.mm;
          break;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          break;
        case HeadSequence:
          h_.hs->dealloc();
          delete h_.hs;
          break;
        case HeadMap:
          h_.hm->dealloc();
          delete h_.hm;
          break;
        case LocalObject:
          delete h_.o;
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          break;
        case SequenceMap:
          h_.sm->dealloc();
          delete h_.sm;
          break;
        case HeadSequenceMap:
          h_.hsm->dealloc();
          delete h_.hsm;
          break;
      }
      
      t_ = x.t_;
      h_ = x.h_;
      x.t_ = Undefined;
      
      return *this;
    }
    
    nvar& operator=(int x){
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Float:
        case StringPointer:
        case RawPointer:
        case ObjectPointer:
        case Pointer:
          t_ = Integer;
          h_.i = x;
          return *this;
        case Integer:
          h_.i = x;
          return *this;
        case Rational:
          delete h_.r;
          t_ = Integer;
          h_.i = x;
          return *this;
        case Real:
          delete h_.x;
          t_ = Integer;
          h_.i = x;
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.s;
          t_ = Integer;
          h_.i = x;
          return *this;
        case Vector:
          delete h_.v;
          t_ = Integer;
          h_.i = x;
          return *this;
        case List:
          delete h_.l;
          t_ = Integer;
          h_.i = x;
          return *this;
        case Function:
          delete h_.f;
          t_ = Integer;
          h_.i = x;
          return *this;
        case Map:
          delete h_.m;
          t_ = Integer;
          h_.i = x;
          return *this;
        case Multimap:
          delete h_.mm;
          t_ = Integer;
          h_.i = x;
          return *this;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Integer;
          h_.i = x;
          return *this;
        case HeadSequence:
          h_.hs->dealloc();
          delete h_.hs;
          break;
        case HeadMap:
          h_.hm->dealloc();
          delete h_.hm;
          break;
        case LocalObject:
          delete h_.o;
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          break;
        case SequenceMap:
          h_.sm->dealloc();
          delete h_.sm;
          break;
        case HeadSequenceMap:
          h_.hsm->dealloc();
          delete h_.hsm;
          break;
      }
      
      t_ = Integer;
      h_.i = x;
      return *this;
    }
    
    nvar& operator=(bool x){
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Float:
        case StringPointer:
        case RawPointer:
        case ObjectPointer:
        case Pointer:
          t_ = x ? True : False;
          return *this;
        case Rational:
          delete h_.r;
          t_ = x ? True : False;
          return *this;
        case Real:
          delete h_.x;
          t_ = x ? True : False;
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.s;
          t_ = x ? True : False;
          return *this;
        case Vector:
          delete h_.v;
          t_ = x ? True : False;
          return *this;
        case List:
          delete h_.l;
          t_ = x ? True : False;
          return *this;
        case Function:
          delete h_.f;
          t_ = x ? True : False;
          return *this;
        case Map:
          delete h_.m;
          t_ = x ? True : False;
          return *this;
        case Multimap:
          delete h_.mm;
          t_ = x ? True : False;
          return *this;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = x ? True : False;
          return *this;
        case HeadSequence:
          h_.hs->dealloc();
          delete h_.hs;
          break;
        case HeadMap:
          h_.hm->dealloc();
          delete h_.hm;
          break;
        case LocalObject:
          delete h_.o;
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          break;
        case SequenceMap:
          h_.sm->dealloc();
          delete h_.sm;
          break;
        case HeadSequenceMap:
          h_.hsm->dealloc();
          delete h_.hsm;
          break;
      }
      
      t_ = x ? True : False;
      return *this;
    }

    nvar& operator=(const char* x){
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
        case Integer:
        case Float:
        case StringPointer:
        case RawPointer:
        case ObjectPointer:
        case Pointer:
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case Rational:
          delete h_.r;
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case Real:
          delete h_.x;
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case String:
        case Binary:
        case Symbol:
          t_ = String;
          *h_.s = x;
          return *this;
        case Vector:
          delete h_.v;
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case List:
          delete h_.l;
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case Function:
          delete h_.f;
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case Map:
          delete h_.m;
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case Multimap:
          delete h_.mm;
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = String;
          h_.s = new nstr(x);
          return *this;
        case HeadSequence:
          h_.hs->dealloc();
          delete h_.hs;
          break;
        case HeadMap:
          h_.hm->dealloc();
          delete h_.hm;
          break;
        case LocalObject:
          delete h_.o;
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          break;
        case SequenceMap:
          h_.sm->dealloc();
          delete h_.sm;
          break;
        case HeadSequenceMap:
          h_.hsm->dealloc();
          delete h_.hsm;
          break;
      }
      
      t_ = String;
      h_.s = new nstr(x);
      return *this;
    }
    
    nvar& operator=(double x){
      switch(t_){
        case Float:
          h_.d = x;
          return *this;
        case Rational:
          delete h_.r;
          t_ = Float;
          h_.d = x;
          return *this;
        case Real:
          delete h_.x;
          t_ = Float;
          h_.d = x;
          return *this;
        case String:
        case Binary:
        case Symbol:
          delete h_.s;
          t_ = Float;
          h_.d = x;
          return *this;
        case Vector:
          delete h_.v;
          t_ = Float;
          h_.d = x;
          return *this;
        case List:
          delete h_.l;
          t_ = Float;
          h_.d = x;
          return *this;
        case Function:
          delete h_.f;
          t_ = Float;
          h_.d = x;
          return *this;
        case Map:
          delete h_.m;
          t_ = Float;
          h_.d = x;
          return *this;
        case Multimap:
          delete h_.mm;
          t_ = Float;
          h_.d = x;
          return *this;
        case Reference:
          if(h_.ref->deref()){
            delete h_.ref;
          }
          t_ = Float;
          h_.d = x;
          return *this;
        case HeadSequence:
          h_.hs->dealloc();
          delete h_.hs;
          break;
        case HeadMap:
          h_.hm->dealloc();
          delete h_.hm;
          break;
        case LocalObject:
          delete h_.o;
          break;
        case SharedObject:
          if(h_.o->deref()){
            delete h_.o;
          }
          break;
        case SequenceMap:
          h_.sm->dealloc();
          delete h_.sm;
          break;
        case HeadSequenceMap:
          h_.hsm->dealloc();
          delete h_.hsm;
          break;
        default:
          t_ = Float;
          h_.d = x;
          return *this;
      }
      
      t_ = Float;
      h_.d = x;
      return *this;
    }
    
    nvar& operator=(nvar* x){
      return *this = nvar(x);
    }
    
    nvar& operator=(const nvar& x);
    
    nvar& operator=(unsigned long x){
      return *this = nvar(x);
    }
    
    nvar& operator=(long long x){
      return *this = nvar(x);
    }
    
    nvar& set(const nvar& x);
    
    nvar operator-() const{
      switch(t_){
        case Integer:
          return -h_.i;
        case Rational:
          return -*h_.r;
        case Float:
          return -h_.d;
        case Real:
          return -*h_.x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Neg");
          h.f->v.push_back(new nvar(*this, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvar ret;
          ret.t_ = Vector;
          ret.h_.v = new nvec(-*h_.v);
          return ret;
        }
        case HeadSequence:
          return -*h_.hs->s;
        case HeadMap:
          return -*h_.hm->h;
        case SequenceMap:
          return -*h_.sm->s;
        case HeadSequenceMap:
          return -*h_.hsm->s;
        case Reference:
          return -*h_.ref->v;
        case Pointer:
          return -*h_.vp;
      }
      
      NERROR("invalid operand");
    }
    
    nvar& operator+=(const nvar& x);
    
    nvar& operator+=(int x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          h_.i += x;
          return *this;
        case Rational:
          *h_.r += x;
          return *this;
        case Float:
          h_.d += x;
          return *this;
        case Real:
          *h_.x += x;
          return *this;
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v += x;
          return *this;
        case HeadSequence:
          *h_.hs->s += x;
          return *this;
        case HeadMap:
          *h_.hm->h += x;
          return *this;
        case SequenceMap:
          *h_.sm->s += x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s += x;
          return *this;
        case Reference:
          *h_.ref->v += x;
          return *this;
        case Pointer:
          *h_.vp += x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator+=(double x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          t_ = Float;
          h_.d = h_.i;
          h_.d += x;
          return *this;
        case Rational:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x;
          return *this;
        }
        case Float:
          h_.d += x;
          return *this;
        case Real:{
          double d = h_.x->toDouble();
          delete h_.x;
          h_.d = d + x;
          t_ = Float;
          break;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Add");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v += x;
          return *this;
        case HeadSequence:
          *h_.hs->s += x;
          return *this;
        case HeadMap:
          *h_.hm->h += x;
          return *this;
        case SequenceMap:
          *h_.sm->s += x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s += x;
          return *this;
        case Reference:
          *h_.ref->v += x;
          return *this;
        case Pointer:
          *h_.vp += x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
      
      return *this;
    }
    
    nvar operator+(const nvar& x) const;
    
    nvar operator+(int x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i + x;
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          h.r += x;
          return nvar(Rational, h);
        }
        case Float:
          return h_.d + x;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x += x;
          return nvar(Real, h);
        }
        case Binary:
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v + x;
        case HeadSequence:
          return *h_.hs->s + x;
        case HeadMap:
          return *h_.hm->h + x;
        case SequenceMap:
          return *h_.sm->s += x;
        case HeadSequenceMap:
          return *h_.hsm->s + x;
        case Reference:
          return *h_.ref->v + x;
        case Pointer:
          return *h_.vp + x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar operator+(double x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i + x;
        case Rational:
          return h_.r->toDouble() + x;
        case Float:
          return h_.d + x;
        case Real:
          return h_.x->toDouble() + x;
        case Binary:
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Add");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v + x;
        case HeadSequence:
          return *h_.hs->s + x;
        case HeadMap:
          return *h_.hm->h + x;
        case SequenceMap:
          return *h_.sm->s = x;
        case HeadSequenceMap:
          return *h_.hsm->s + x;
        case Reference:
          return *h_.ref->v + x;
        case Pointer:
          return *h_.vp + x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar operator++(int){
      nvar ret(*this);
      *this += 1;
      return ret;
    }
    
    nvar& operator++(){
      return *this += 1;
    }
    
    nvar& operator-=(const nvar& x);
    
    nvar& operator-=(int x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          h_.i -= x;
          return *this;
        case Rational:
          *h_.r -= x;
          return *this;
        case Float:
          h_.d -= x;
          return *this;
        case Real:
          *h_.x -= x;
          return *this;
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v -= x;
          return *this;
        case HeadSequence:
          *h_.hs->s -= x;
          return *this;
        case HeadMap:
          *h_.hm->h -= x;
          return *this;
        case SequenceMap:
          *h_.sm->s -= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s -= x;
          return *this;
        case Reference:
          *h_.ref->v -= x;
          return *this;
        case Pointer:
          *h_.vp -= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator-=(double x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          t_ = Float;
          h_.d = h_.i;
          h_.d -= x;
          return *this;
        case Rational:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x;
          return *this;
        }
        case Float:
          h_.d -= x;
          return *this;
        case Real:{
          double d = h_.x->toDouble();
          d -= x;
          delete h_.x;
          h_.d = d;
          t_ = Float;
          break;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Sub");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v -= x;
          return *this;
        case HeadSequence:
          *h_.hs->s -= x;
          return *this;
        case HeadMap:
          *h_.hm->h -= x;
          return *this;
        case SequenceMap:
          *h_.sm->s -= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s -= x;
          return *this;
        case Reference:
          *h_.ref->v -= x;
          return *this;
        case Pointer:
          *h_.vp -= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
      
      return *this;
    }
    
    nvar operator--(int){
      nvar ret(*this);
      *this -= 1;
      return ret;
    }
    
    nvar& operator--(){
      return *this -= 1;
    }
    
    nvar operator-(const nvar& x) const;
    
    nvar operator-(int x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i - x;
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          h.r -= x;
          return nvar(Rational, h);
        }
        case Float:
          return h_.d - x;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          h.x -= x;
          return nvar(Real, h);
        }
        case Binary:
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v - x;
        case HeadSequence:
          return *h_.hs->s - x;
        case HeadMap:
          return *h_.hm->h - x;
        case SequenceMap:
          return *h_.sm->s - x;
        case HeadSequenceMap:
          return *h_.hsm->s - x;
        case Reference:
          return *h_.ref->v - x;
        case Pointer:
          return *h_.vp - x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar operator-(double x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i - x;
        case Rational:
          return h_.r->toDouble() - x;
        case Float:
          return h_.d - x;
        case Real:
          return h_.x->toDouble() - x;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Sub");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v - x;
        case HeadSequence:
          return *h_.hs->s - x;
        case HeadMap:
          return *h_.hm->h - x;
        case SequenceMap:
          return *h_.sm->s -= x;
        case HeadSequenceMap:
          return *h_.hsm->s - x;
        case Reference:
          return *h_.ref->v - x;
        case Pointer:
          return *h_.vp - x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator*=(const nvar& x);
    
    nvar& operator*=(int x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          h_.i *= x;
          return *this;
        case Rational:
          *h_.r *= x;
          return *this;
        case Float:
          h_.d *= x;
          return *this;
        case Real:
          *h_.x *= x;
          return *this;
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v *= x;
          return *this;
        case HeadSequence:
          *h_.hs->s *= x;
          return *this;
        case HeadMap:
          *h_.hm->h *= x;
          return *this;
        case SequenceMap:
          *h_.sm->s *= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s *= x;
          return *this;
        case Reference:
          *h_.ref->v *= x;
          return *this;
        case Pointer:
          *h_.vp *= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator*=(double x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          t_ = Float;
          h_.d = h_.i;
          h_.d *= x;
          return *this;
        case Rational:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x;
          return *this;
        }
        case Float:
          h_.d *= x;
          return *this;
        case Real:{
          double d = h_.x->toDouble();
          d *= x;
          delete h_.x;
          h_.d = d;
          t_ = Float;
          break;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mul");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v *= x;
          return *this;
        case HeadSequence:
          *h_.hs->s *= x;
          return *this;
        case HeadMap:
          *h_.hm->h *= x;
          return *this;
        case SequenceMap:
          *h_.sm->s *= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s *= x;
          return *this;
        case Reference:
          *h_.ref->v *= x;
          return *this;
        case Pointer:
          *h_.vp *= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
      
      return *this;
    }
    
    nvar operator*(const nvar& x) const;
    
    nvar operator*(int x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i * x;
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r *= x;
          return nvar(Rational, h);
        }
        case Float:
          return h_.d * x;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x *= x;
          return nvar(Real, h);
        }
        case Binary:
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v * x;
        case HeadSequence:
          return *h_.hs->s * x;
        case HeadMap:
          return *h_.hm->h * x;
        case SequenceMap:
          return *h_.sm->s *= x;
        case HeadSequenceMap:
          return *h_.hsm->s * x;
        case Reference:
          return *h_.ref->v * x;
        case Pointer:
          return *h_.vp * x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar operator*(double x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i * x;
        case Rational:
          return h_.r->toDouble() * x;
        case Float:
          return h_.d * x;
        case Real:
          return h_.x->toDouble() * x;
        case Binary:
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Mul");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v * x;
        case HeadSequence:
          return *h_.hs->s * x;
        case HeadMap:
          return *h_.hm->h * x;
        case SequenceMap:
          return *h_.sm->s * x;
        case HeadSequenceMap:
          return *h_.hsm->s * x;
        case Reference:
          return *h_.ref->v * x;
        case Pointer:
          return *h_.vp * x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator/=(const nvar& x);
    
    nvar& operator/=(int x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          h_.i /= x;
          return *this;
        case Rational:
          *h_.r /= x;
          return *this;
        case Float:
          h_.d /= x;
          return *this;
        case Real:
          *h_.x /= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v /= x;
          return *this;
        case HeadSequence:
          *h_.hs->s /= x;
          return *this;
        case HeadMap:
          *h_.hm->h /= x;
          return *this;
        case SequenceMap:
          *h_.sm->s /= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s /= x;
          return *this;
        case Reference:
          *h_.ref->v /= x;
          return *this;
        case Pointer:
          *h_.vp /= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator/=(double x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          t_ = Float;
          h_.d = h_.i;
          h_.d /= x;
          return *this;
        case Rational:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = d + x;
          return *this;
        }
        case Float:
          h_.d /= x;
          return *this;
        case Real:{
          double d = h_.x->toDouble();
          d /= x;
          delete h_.x;
          h_.d = d;
          t_ = Float;
          break;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Div");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v /= x;
          return *this;
        case HeadSequence:
          *h_.hs->s /= x;
          return *this;
        case HeadMap:
          *h_.hm->h /= x;
          return *this;
        case SequenceMap:
          *h_.sm->s /= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s /= x;
          return *this;
        case Reference:
          *h_.ref->v /= x;
          return *this;
        case Pointer:
          *h_.vp /= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
      
      return *this;
    }
    
    nvar operator/(const nvar& x) const;
    
    nvar operator/(int x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i / x;
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r /= x;
          return nvar(Rational, h);
        }
        case Float:
          return h_.d / x;
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x /= x;
          return nvar(Real, h);
        }
        case Binary:
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Div");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v / x;
        case HeadSequence:
          return *h_.hs->s / x;
        case HeadMap:
          return *h_.hm->h / x;
        case SequenceMap:
          return *h_.sm->s / x;
        case HeadSequenceMap:
          return *h_.hsm->s / x;
        case Reference:
          return *h_.ref->v / x;
        case Pointer:
          return *h_.vp / x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar operator/(double x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i / x;
        case Rational:
          return h_.r->toDouble() / x;
        case Float:
          return h_.d / x;
        case Real:
          return h_.x->toDouble() / x;
        case Binary:
        case String:
        case StringPointer:
          NERROR("invalid operands");
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Div");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v / x;
        case HeadSequence:
          return *h_.hs->s / x;
        case HeadMap:
          return *h_.hm->h / x;
        case SequenceMap:
          return *h_.sm->s / x;
        case HeadSequenceMap:
          return *h_.hsm->s / x;
        case Reference:
          return *h_.ref->v / x;
        case Pointer:
          return *h_.vp / x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator%=(const nvar& x);
    
    nvar& operator%=(int x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          h_.i %= x;
          return *this;
        case Rational:
          *h_.r = *h_.r - *h_.r / x;
          return *this;
        case Float:
          h_.d = fmod(h_.d, static_cast<double>(x));
          return *this;
        case Real:
          *h_.x %= x;
          return *this;
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v %= x;
          return *this;
        case HeadSequence:
          *h_.hs->s %= x;
          return *this;
        case HeadMap:
          *h_.hm->h %= x;
          return *this;
        case SequenceMap:
          *h_.sm->s %= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s %= x;
          return *this;
        case Reference:
          *h_.ref->v %= x;
          return *this;
        case Pointer:
          *h_.vp %= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar& operator%=(double x){
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          t_ = Float;
          h_.d = fmod(h_.i, x);
          return *this;
        case Rational:{
          double d = h_.r->toDouble();
          delete h_.r;
          t_ = Float;
          h_.d = fmod(d, x);
          return *this;
        }
        case Float:
          h_.d = fmod(h_.d, x);
          return *this;
        case Real:{
          double d = h_.x->toDouble();
          delete h_.x;
          h_.d = fmod(d, x);
          t_ = Float;
          break;
        }
        case Symbol:
        case Function:{
          CFunction* f = new CFunction("Mod");
          f->v.push_back(new nvar(std::move(*this)));
          f->v.push_back(x);
          t_ = Function;
          h_.f = f;
          return *this;
        }
        case Vector:
          *h_.v %= x;
          return *this;
        case HeadSequence:
          *h_.hs->s %= x;
          return *this;
        case HeadMap:
          *h_.hm->h %= x;
          return *this;
        case SequenceMap:
          *h_.sm->s %= x;
          return *this;
        case HeadSequenceMap:
          *h_.hsm->s %= x;
          return *this;
        case Reference:
          *h_.ref->v %= x;
          return *this;
        case Pointer:
          *h_.vp %= x;
          return *this;
        default:
          NERROR("left operand is invalid");
      }
      
      return *this;
    }
    
    nvar operator%(const nvar& x) const;
    
    nvar operator%(int x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return h_.i % x;
        case Rational:{
          Head h;
          h.r = new nrat(*h_.r);
          *h.r = *h.r - *h.r / x;
          return nvar(Rational, h);
        }
        case Float:
          return fmod(h_.d, static_cast<double>(x));
        case Real:{
          Head h;
          h.x = new nreal(*h_.x);
          *h.x %= x;
          return nvar(Real, h);
        }
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v % x;
        case HeadSequence:
          return *h_.hs->s % x;
        case HeadMap:
          return *h_.hm->h % x;
        case SequenceMap:
          return *h_.sm->s % x;
        case HeadSequenceMap:
          return *h_.hsm->s % x;
        case Reference:
          return *h_.ref->v % x;
        case Pointer:
          return *h_.vp % x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    nvar operator%(double x) const{
      switch(t_){
        case None:
        case Undefined:
          NERROR("left operand is undefined");
        case Integer:
          return fmod(h_.i, x);
        case Rational:
          return fmod(h_.r->toDouble(), x);
        case Float:
          return fmod(h_.d, x);
        case Real:
          return fmod(h_.x->toDouble(), x);
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("Mod");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Vector:
          return *h_.v % x;
        case HeadSequence:
          return *h_.hs->s % x;
        case HeadMap:
          return *h_.hm->h % x;
        case SequenceMap:
          return *h_.sm->s % x;
        case HeadSequenceMap:
          return *h_.hsm->s % x;
        case Reference:
          return *h_.ref->v % x;
        case Pointer:
          return *h_.vp % x;
        default:
          NERROR("left operand is invalid");
      }
    }
    
    bool less(const nvar& x) const;
    
    nvar operator<(const nvar& x) const;
    
    nvar operator<(int x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return h_.i < x;
        case Rational:
          return *h_.r < x;
        case Float:
          return h_.d < x;
        case Real:
          return *h_.x < x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("LT");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h < x;
        case HeadMap:
          return *h_.hm->h < x;
        case HeadSequenceMap:
          return *h_.hsm->h < x;
        case Reference:
          return *h_.ref->v < x;
        case Pointer:
          return *h_.vp < x;
        default:
          return false;
      }
    }
    
    nvar operator<(double x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return h_.i < x;
        case Rational:
          return *h_.r < x;
        case Float:
          return h_.d < x;
        case Real:
          return *h_.x < x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("LT");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h < x;
        case HeadMap:
          return *h_.hm->h < x;
        case HeadSequenceMap:
          return *h_.hsm->h < x;
        case Reference:
          return *h_.ref->v < x;
        case Pointer:
          return *h_.vp < x;
        default:
          return false;
      }
    }
    
    nvar operator<=(const nvar& x) const;
    
    nvar operator<=(int x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return h_.i <= x;
        case Rational:
          return *h_.r <= x;
        case Float:
          return h_.d <= x;
        case Real:
          return *h_.x <= x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("LE");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h <= x;
        case HeadMap:
          return *h_.hm->h <= x;
        case HeadSequenceMap:
          return *h_.hsm->h <= x;
        case Reference:
          return *h_.ref->v <= x;
        case Pointer:
          return *h_.vp <= x;
        default:
          return false;
      }
    }
    
    nvar operator<=(double x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return true;
        case Integer:
          return h_.i <= x;
        case Rational:
          return *h_.r <= x;
        case Float:
          return h_.d <= x;
        case Real:
          return *h_.x <= x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("LE");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h <= x;
        case HeadMap:
          return *h_.hm->h <= x;
        case HeadSequenceMap:
          return *h_.hsm->h <= x;
        case Reference:
          return *h_.ref->v <= x;
        case Pointer:
          return *h_.vp <= x;
        default:
          return false;
      }
    }
    
    nvar operator>(const nvar& x) const;
    
    nvar operator>(int x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return h_.i > x;
        case Rational:
          return *h_.r > x;
        case Float:
          return h_.d > x;
        case Real:
          return *h_.x > x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("GT");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h > x;
        case HeadMap:
          return *h_.hm->h > x;
        case HeadSequenceMap:
          return *h_.hsm->h > x;
        case Reference:
          return *h_.ref->v > x;
        case Pointer:
          return *h_.vp > x;
        default:
          return true;
      }
    }
    
    nvar operator>(double x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return h_.i > x;
        case Rational:
          return *h_.r > x;
        case Float:
          return h_.d > x;
        case Real:
          return *h_.x > x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("GT");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h > x;
        case HeadMap:
          return *h_.hm->h > x;
        case HeadSequenceMap:
          return *h_.hsm->h > x;
        case Reference:
          return *h_.ref->v > x;
        case Pointer:
          return *h_.vp > x;
        default:
          return true;
      }
    }
    
    nvar operator>=(const nvar& x) const;
    
    nvar operator>=(int x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return h_.i >= x;
        case Rational:
          return *h_.r >= x;
        case Float:
          return h_.d >= x;
        case Real:
          return *h_.x >= x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("GE");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h >= x;
        case HeadMap:
          return *h_.hm->h >= x;
        case HeadSequenceMap:
          return *h_.hsm->h >= x;
        case Reference:
          return *h_.ref->v >= x;
        case Pointer:
          return *h_.vp >= x;
        default:
          return true;
      }
    }
    
    nvar operator>=(double x) const{
      switch(t_){
        case None:
        case Undefined:
        case False:
        case True:
          return false;
        case Integer:
          return h_.i >= x;
        case Rational:
          return *h_.r >= x;
        case Float:
          return h_.d >= x;
        case Real:
          return *h_.x >= x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("GE");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case HeadSequence:
          return *h_.hs->h >= x;
        case HeadMap:
          return *h_.hm->h >= x;
        case HeadSequenceMap:
          return *h_.hsm->h >= x;
        case Reference:
          return *h_.ref->v >= x;
        case Pointer:
          return *h_.vp >= x;
        default:
          return true;
      }
    }
    
    nvar operator==(const nvar& x) const;
    
    nvar operator==(int x) const{
      switch(t_){
        case Integer:
          return h_.i == x;
        case Rational:
          return *h_.r == x;
        case Float:
          return h_.d == x;
        case Real:
          return *h_.x == x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("EQ");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Reference:
          return *h_.ref->v == x;
        case Pointer:
          return *h_.vp == x;
        default:
          return false;
      }
    }
    
    nvar operator==(double x) const{
      switch(t_){
        case Integer:
          return h_.i == x;
        case Rational:
          return *h_.r == x;
        case Float:
          return h_.d == x;
        case Real:
          return *h_.x == x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("EQ");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Reference:
          return *h_.ref->v == x;
        case Pointer:
          return *h_.vp == x;
        default:
          return false;
      }
    }
    
    nvar operator!=(const nvar& x) const;
    
    nvar operator!=(int x) const{
      switch(t_){
        case Integer:
          return h_.i != x;
        case Rational:
          return *h_.r != x;
        case Float:
          return h_.d != x;
        case Real:
          return *h_.x != x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("NE");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Reference:
          return *h_.ref->v != x;
        case Pointer:
          return *h_.vp != x;
        default:
          return true;
      }
    }
    
    nvar operator!=(double x) const{
      switch(t_){
        case Integer:
          return h_.i != x;
        case Rational:
          return *h_.r != x;
        case Float:
          return h_.d != x;
        case Real:
          return *h_.x != x;
        case Symbol:
        case Function:{
          Head h;
          h.f = new CFunction("NE");
          h.f->v.push_back(new nvar(*this, Copy));
          h.f->v.push_back(x);
          return new nvar(Function, h);
        }
        case Reference:
          return *h_.ref->v != x;
        case Pointer:
          return *h_.vp != x;
        default:
          return true;
      }
    }
    
    nvar operator&&(const nvar& x) const;
    
    nvar operator&&(bool x) const{
      return *this && nvar(x);
    }
    
    nvar operator||(const nvar& x) const;
    
    nvar operator||(bool x) const{
      return *this || nvar(x);
    }
    
    nvar operator!() const{
      switch(t_){
        case None:
        case Undefined:
        case False:
          return true;
        case True:
          return false;
        case Integer:
          return h_.i == 0;
        case Rational:
          return *h_.r == 0;
        case Float:
          return h_.d == 0;
        case Real:
          return *h_.x == 0;
        case String:
        case StringPointer:
          return false;
        case Function:
        case Symbol:{
          Head h;
          h.f = new CFunction("Not");
          h.f->v.push_back(new nvar(*this, Copy));
          return new nvar(Function, h);
        }
        case Vector:{
          nvar ret;
          ret.t_ = Vector;
          ret.h_.v = new nvec(!*h_.v);
          return ret;
        }
        case HeadSequence:
          return !*h_.hs->s;
        case HeadMap:
          return !*h_.hm->h;
        case SequenceMap:
          return !*h_.sm->s;
        case HeadSequenceMap:
          return !*h_.hsm->s;
        default:
          NERROR("invalid operand");
      }
    }
    
    nvar& operator[](const nvar& key){
      switch(key.t_){
        case Integer:
          switch(t_){
            case Vector:{
              size_t k = key.asLong();
#ifndef NEU_FAST
              if(k >= h_.v->size()){
                NERROR("index out of range: " + key);
              }
#endif
              return (*h_.v)[k];
            }
            case List:{
              size_t k = key.asLong();
#ifndef NEU_FAST
              if(k >= h_.l->size()){
                NERROR("index out of range: " + key);
              }
#endif
              return (*h_.l)[k];
            }
            case Function:{
              size_t k = key.asLong();
#ifndef NEU_FAST
              if(k >= h_.f->v.size()){
                NERROR("index out of range: "  + key);
              }
#endif
              return h_.f->v[k];
            }
            case HeadSequence:
              return (*h_.hs->s)[key];
            case Map:{
              auto itr = h_.m->find(key);
#ifndef NEU_FAST
              if(itr == h_.m->end()){
                NERROR("invalid key: " + key);
              }
#endif
              return itr->second;
            }
            case HeadMap:
              return (*h_.hm->m)[key];
            case SequenceMap:
              return (*h_.sm->s)[key];
            case HeadSequenceMap:
              return (*h_.hsm->s)[key];
            case Reference:
              return (*h_.ref->v)[key];
            case Pointer:
              return (*h_.vp)[key];
            default:
              NERROR("invalid operand");
          }
        case Reference:
          return (*this)[*key.h_.ref->v];
        case Pointer:
          return (*this)[*key.h_.vp];
        default:
          switch(t_){
            case Function:
              if(h_.f->m){
                auto itr = h_.f->m->find(key);
#ifndef NEU_FAST
                if(itr == h_.f->m->end()){
                  NERROR("invalid key: " + key);
                }
#endif
                return itr->second;
              }
              NERROR("function does not hold a map");
            case Map:{
              auto itr = h_.m->find(key);
#ifndef NEU_FAST
              if(itr == h_.m->end()){
                NERROR("invalid key: " + key);
              }
#endif
              return itr->second;
              break;
            }
            case HeadMap:
              return (*h_.hm->m)[key];
            case SequenceMap:
              return (*h_.sm->m)[key];
            case HeadSequenceMap:
              return (*h_.hsm->m)[key];
            case Reference:
              return (*h_.ref->v)[key];
            case Pointer:
              return (*h_.vp)[key];
            default:
              NERROR("invalid operand");
          }
      }
    }
    
    const nvar& operator[](const nvar& key) const{
      return const_cast<nvar&>(*this)[key];
    }
    
    nvar& operator[](int k){
      switch(t_){
        case Vector:
#ifndef NEU_FAST
          if(k >= h_.v->size()){
            NERROR("index out of range: " + nstr::toStr(k));
          }
#endif
          return (*h_.v)[k];
        case List:
#ifndef NEU_FAST
          if(k >= h_.l->size()){
            NERROR("index out of range: " + nstr::toStr(k));
          }
#endif
          return (*h_.l)[k];
        case Function:
#ifndef NEU_FAST
          if(k >= h_.f->v.size()){
            NERROR("index out of range: " + nstr::toStr(k));
          }
#endif
          return h_.f->v[k];
        case HeadSequence:
          return (*h_.hs->s)[k];
        case Map:{
          auto itr = h_.m->find(k);
#ifndef NEU_FAST
          if(itr == h_.m->end()){
            NERROR("invalid key: " + nstr::toStr(k));
          }
#endif
          return itr->second;
        }
        case HeadMap:
          return (*h_.hm->m)[k];
        case SequenceMap:
          return (*h_.sm->s)[k];
        case HeadSequenceMap:
          return (*h_.hsm->s)[k];
        case Reference:
          return (*h_.ref->v)[k];
        case Pointer:
          return (*h_.vp)[k];
        default:
          NERROR("invalid operand");
      }
    }
    
    const nvar& operator[](int k) const{
      return const_cast<nvar&>(*this)[k];
    }
    
    nvar& operator[](const char* key){
      switch(t_){
        case Map:{
          auto itr = h_.m->find(key);
#ifndef NEU_FAST
          if(itr == h_.m->end()){
            NERROR("invalid key: " + nstr::toStr(key));
          }
#endif
          return itr->second;
        }
        case Function:
          if(h_.f->m){
            auto itr = h_.f->m->find(key);
#ifndef NEU_FAST
            if(itr == h_.f->m->end()){
              NERROR("invalid key: " + nstr::toStr(key));
            }
#endif
            return itr->second;
          }
          NERROR("function does not hold a map");
        case HeadMap:
          return (*h_.hm->m)[key];
        case SequenceMap:
          return (*h_.sm->m)[key];
        case HeadSequenceMap:
          return (*h_.hsm->m)[key];
        case Reference:
          return (*h_.ref->v)[key];
        case Pointer:
          return (*h_.vp)[key];
        default:
          NERROR("invalid operand");
      }
    }
    
    const nvar& operator[](const char* key) const{
      return const_cast<nvar&>(*this)[key];
    }
    
    nvar& get(const nvar& key){
      switch(t_){
        case Function:
          if(h_.f->m){
            auto itr = h_.f->m->find(key);
#ifndef NEU_FAST
            if(itr == h_.f->m->end()){
              NERROR("invalid key: " + key);
            }
#endif
            return itr->second;
          }
          NERROR("function does not hold a map");
        case Map:{
          auto itr = h_.m->find(key);
#ifndef NEU_FAST
          if(itr == h_.m->end()){
            NERROR("invalid key: " + key);
          }
#endif
          return itr->second;
          break;
        }
        case HeadMap:
          return (*h_.hm->m)[key];
        case SequenceMap:
          return (*h_.sm->m)[key];
        case HeadSequenceMap:
          return (*h_.hsm->m)[key];
        case Reference:
          return h_.ref->v->get(key);
        case Pointer:
          return h_.vp->get(key);
        default:
          NERROR("invalid operand");
      }
    }
    
    const nvar& get(const nvar& key) const{
      return const_cast<nvar&>(*this).get(key);
    }
    
    const nvar& get(const nvar& key, const nvar& def) const{
      switch(t_){
        case Function:
          return h_.f->m ? h_.f->m->get(key, def) : def;
        case Map:
          return h_.m->get(key, def);
        case HeadMap:
          return h_.hm->m->get(key, def);
        case SequenceMap:
          return h_.sm->m->get(key, def);
        case HeadSequenceMap:
          return h_.hsm->m->get(key, def);
        case Reference:
          return h_.ref->v->get(key, def);
        case Pointer:
          return h_.vp->get(key, def);
        default:
          return def;
      }
    }
    
    nvar& get(const nvar& key, nvar& def){
      switch(t_){
        case Function:
          return h_.f->m ? h_.f->m->get(key, def) : def;
        case Map:
          return h_.m->get(key, def);
        case HeadMap:
          return h_.hm->m->get(key, def);
        case SequenceMap:
          return h_.sm->m->get(key, def);
        case HeadSequenceMap:
          return h_.hsm->m->get(key, def);
        case Reference:
          return h_.ref->v->get(key, def);
        case Pointer:
          return h_.vp->get(key, def);
        default:
          return def;
      }
    }
    
    nvar& operator()(const nvar& key){
      switch(t_){
        case None:
        case Undefined:
          t_ = Map;
          h_.m = new nmap;
          return (*h_.m)[key];
        case Vector:
        case List:{
          Head h;
          h.m = new nmap;
          h_.sm = new CSequenceMap(new nvar(t_, h_), new nvar(Map, h));
          t_ = SequenceMap;
          return (*h.m)[key];
        }
        case Function:{
          if(!h_.f->m){
            h_.f->m = new nmap;
          }
          return (*h_.f->m)[key];
        }
        case HeadSequence:{
          Head h;
          h.m = new nmap;
          CHeadSequenceMap* hsm =
          new CHeadSequenceMap(h_.hs->h, h_.hs->s, new nvar(Map, h));
          delete h_.hs;
          h_.hsm = hsm;
          t_ = HeadSequenceMap;
          return (*h.m)[key];
        }
        case Map:
          return (*h_.m)[key];
        case Multimap:{
          auto itr = h_.mm->insert(std::make_pair(key, nvar()));
          return itr->second;
        }
        case HeadMap:
          return (*h_.hm->m)(key);
        case SequenceMap:
          return (*h_.sm->m)(key);
        case HeadSequenceMap:
          return (*h_.hsm->m)(key);
        case Reference:
          return (*h_.ref->v)(key);
        case Pointer:
          return (*h_.vp)(key);
        default:{
          Head h;
          h.m = new nmap;
          h_.hm = new CHeadMap(new nvar(t_, h_), new nvar(Map, h));
          t_ = HeadMap;
          return (*h.m)[key];
        }
      }
    }
    
    nvar& operator()(const char* k){
      if(nstr::isSymbol(k)){
        return (*this)(nvar(k, Sym));
      }
      
      return (*this)(nvar(k));
    }
    
    nvar& operator()(const nstr& k){
      if(nstr::isSymbol(k)){
        return (*this)(nvar(k, Sym));
      }
      
      return (*this)(nvar(k));
    }
    
    nvar& operator()(std::initializer_list<nvar> il){
      assert(il.size() % 2 == 0);
      
      auto itr = il.begin();
      while(itr != il.end()){
        const nvar& k = *itr;
        ++itr;
        const nvar& v = *itr;
        ++itr;
        
        if(k.isString()){
          (*this)(k.str()) = v;
        }
        else{
          (*this)(k) = v;
        }
      }
      
      return *this;
    }
    
    void erase(const nvar& key){
      switch(t_){
        case Function:
          if(h_.f->m){
            h_.f->m->erase(key);
          }
          break;
        case Map:
          h_.m->erase(key);
          break;
        case Multimap:
          h_.mm->erase(key);
          break;
        case HeadMap:
          h_.hm->m->erase(key);
          break;
        case HeadSequenceMap:
          h_.hsm->m->erase(key);
          break;
        case Reference:
          h_.ref->v->erase(key);
          break;
        case Pointer:
          h_.vp->erase(key);
          break;
      }
    }
    
    void keys(nvec& v) const{
      switch(t_){
        case Function:
          if(h_.f->m){
            for(const auto& itr : *h_.f->m){
              v.push_back(itr.first);
            }
          }
          break;
        case Map:
          for(const auto& itr : *h_.m){
            v.push_back(itr.first);
          }
          break;
        case Multimap:
          for(const auto& itr : *h_.mm){
            v.push_back(itr.first);
          }
          break;
        case HeadMap:
          h_.hm->m->keys(v);
          break;
        case SequenceMap:
          h_.sm->m->keys(v);
          break;
        case HeadSequenceMap:
          h_.hsm->m->keys(v);
          break;
      }
    }
    
    nvec::iterator begin(){
      switch(t_){
        case Function:
          return h_.f->v.begin();
        case Vector:
          return h_.v->begin();
        case HeadSequence:
          return h_.hs->s->begin();
        case SequenceMap:
          return h_.sm->s->begin();
        case HeadSequenceMap:
          return h_.hsm->s->begin();
        default:
          touchVector();
          return h_.v->begin();
      }
    }
    
    nvec::const_iterator begin() const{
      switch(t_){
        case Function:
          return h_.f->v.begin();
        case Vector:
          return h_.v->begin();
        case HeadSequence:
          return h_.hs->s->begin();
        case SequenceMap:
          return h_.sm->s->begin();
        case HeadSequenceMap:
          return h_.hsm->s->begin();
        default:
          return _emptyVec.begin();
      }
    }
    
    nvec::iterator end(){
      switch(t_){
        case Function:
          return h_.f->v.end();
        case Vector:
          return h_.v->end();
        case HeadSequence:
          return h_.hs->s->end();
        case SequenceMap:
          return h_.sm->s->end();
        case HeadSequenceMap:
          return h_.hsm->s->end();
        default:
          touchVector();
          return h_.v->end();
      }
    }
    
    nvec::const_iterator end() const{
      switch(t_){
        case Function:
          return h_.f->v.end();
        case Vector:
          return h_.v->end();
        case HeadSequence:
          return h_.hs->s->end();
        case SequenceMap:
          return h_.sm->s->end();
        case HeadSequenceMap:
          return h_.hsm->s->end();
        default:
          return _emptyVec.end();
      }
    }
    
    void touchVector(){
      switch(t_){
        case None:
        case Undefined:
          t_ = Vector;
          h_.v = new nvec;
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.v = new nvec;
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case Vector:
          break;
        case List:{
          nvec* v = new nvec(h_.l->begin(), h_.l->end());
          delete h_.l;
          h_.v = v;
          t_ = Vector;
          break;
        }
        case HeadSequence:
          h_.hs->s->touchVector();
          break;
        case Map:
        case Multimap:{
          Head h;
          h.v = new nvec;
          h_.sm = new CSequenceMap(new nvar(Vector, h), new nvar(t_, h_));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head h;
          h.v = new nvec;
          CHeadMap* hm = h_.hm;
          h_.hsm =
          new CHeadSequenceMap(hm->h, new nvar(t_, h_), hm->m);
          t_ = HeadSequenceMap;
          delete hm;
          break;
        }
        case SequenceMap:
          h_.sm->s->touchVector();
          break;
        case HeadSequenceMap:
          h_.hsm->s->touchVector();
          break;
        case Reference:
          h_.ref->v->touchVector();
          break;
        case Pointer:
          h_.vp->touchVector();
          break;
        default:
          NERROR("invalid operand");
      }
    }
    
    void touchVector(size_t size, const nvar& v=undef){
      switch(t_){
        case None:
        case Undefined:
          t_ = Vector;
          h_.v = new nvec(size, v);
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.v = new nvec(size, v);
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
          t_ = HeadSequence;
          break;
        }
        case Map:
        case Multimap:{
          Head h;
          h.v = new nvec(size, v);
          h_.sm = new CSequenceMap(new nvar(Vector, h), new nvar(t_, h_));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head h;
          h.v = new nvec(size, v);
          CHeadMap* hm = h_.hm;
          h_.hsm =
          new CHeadSequenceMap(hm->h, new nvar(t_, h_), hm->m);
          t_ = HeadSequenceMap;
          delete hm;
          break;
        }
        case Reference:
          h_.ref->v->touchVector(size, v);
          break;
        case Pointer:
          h_.vp->touchVector(size, v);
          break;
        default:
          NERROR("invalid operand");
      }
    }
    
    void touchList(){
      switch(t_){
        case None:
        case Undefined:
          t_ = List;
          h_.l = new nlist;
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.l = new nlist;
          h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(List, h));
          t_ = HeadSequence;
          break;
        }
        case Vector:{
          nlist* l = new nlist(h_.v->begin(), h_.v->end());
          delete h_.v;
          h_.l = l;
          t_ = List;
          break;
        }
        case List:
          break;
        case HeadSequence:
          h_.hs->s->touchList();
          break;
        case Map:
        case Multimap:{
          Head h;
          h.l = new nlist;
          h_.sm = new CSequenceMap(new nvar(List, h), new nvar(t_, h_));
          t_ = SequenceMap;
          break;
        }
        case HeadMap:{
          Head h;
          h.l = new nlist;
          CHeadMap* hm = h_.hm;
          h_.hsm =
          new CHeadSequenceMap(hm->h, new nvar(t_, h_), hm->m);
          t_ = HeadSequenceMap;
          delete hm;
          break;
        }
        case SequenceMap:
          h_.sm->s->touchList();
          break;
        case HeadSequenceMap:
          h_.hsm->s->touchList();
          break;
        case Reference:
          h_.ref->v->touchList();
        case Pointer:
          h_.vp->touchList();
        default:
          NERROR("invalid operand");
      }
    }
    
    void touchMap(){
      switch(t_){
        case None:
        case Undefined:
          t_ = Map;
          h_.m = new nmap;
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.m = new nmap;
          h_.hm = new CHeadMap(new nvar(t_, h_), new nvar(Map, h));
          t_ = HeadMap;
          break;
        }
        case Vector:
        case List:{
          Head h;
          h.m = new nmap;
          h_.sm = new CSequenceMap(new nvar(t_, h_), new nvar(Map, h));
          t_ = SequenceMap;
          break;
        }
        case Function:
          if(h_.f->m){
            break;
          }
          h_.f->m = new nmap;
          break;
        case HeadSequence:{
          Head h;
          h.m = new nmap;
          CHeadSequence* hs = h_.hs;
          h_.hsm = new CHeadSequenceMap(hs->h, hs->s, new nvar(Map, h));
          t_ = HeadSequenceMap;
          delete hs;
          break;
        }
        case Map:
          break;
        case Multimap:{
          const nmmap& mm = *h_.mm;
          nmap* m = new nmap;
          for(auto& itr : mm){
            auto itr2 = m->find(itr.first);
            if(itr2 == m->end()){
              m->insert(make_pair(itr.first, nvec(1, itr.second)));
            }
            else{
              itr2->second.pushBack(itr.second);
            }
          }
          delete h_.mm;
          h_.m = m;
          t_ = Map;
          break;
        }
        case HeadMap:
          h_.hm->m->touchMap();
          break;
        case SequenceMap:
          h_.sm->m->touchMap();
          break;
        case HeadSequenceMap:
          h_.hsm->m->touchMap();
          break;
        case Reference:
          h_.ref->v->touchMap();
          break;
        case Pointer:
          h_.vp->touchMap();
          break;
        default:
          NERROR("invalid operand");
      }
    }
    
    void touchMultimap(){
      switch(t_){
        case None:
        case Undefined:
          t_ = Multimap;
          h_.mm = new nmmap;
          break;
        case False:
        case True:
        case Integer:
        case Rational:
        case Float:
        case Real:
        case Symbol:
        case String:
        case StringPointer:
        case Binary:
        case RawPointer:
        case ObjectPointer:
        case LocalObject:
        case SharedObject:{
          Head h;
          h.mm = new nmmap;
          h_.hm = new CHeadMap(new nvar(t_, h_), new nvar(Multimap, h));
          t_ = HeadMap;
          break;
        }
        case Vector:
        case List:{
          Head h;
          h.mm = new nmmap;
          h_.sm = new CSequenceMap(new nvar(t_, h_), new nvar(Multimap, h));
          t_ = SequenceMap;
          break;
        }
        case HeadSequence:{
          Head h;
          h.mm = new nmmap;
          CHeadSequence* hs = h_.hs;
          h_.hsm =
          new CHeadSequenceMap(hs->h, hs->s, new nvar(Multimap, h));
          t_ = HeadSequenceMap;
          delete hs;
          break;
        }
        case Map:{
          const nmap& m = *h_.m;
          h_.mm = new nmmap(m.begin(), m.end());
          t_ = Multimap;
          break;
        }
        case Multimap:
          break;
        case HeadMap:
          h_.hm->m->touchMultimap();
        case SequenceMap:
          h_.sm->m->touchMultimap();
          break;
        case HeadSequenceMap:
          h_.hsm->m->touchMultimap();
          break;
        case Reference:
          h_.ref->v->touchMultimap();
          break;
        case Pointer:
          h_.vp->touchMultimap();
          break;
        default:
          NERROR("invalid operand");
      }
    }
    
    nstr& createBuffer(size_t size){
      switch(t_){
        case Undefined:
          h_.s = new nstr;
          h_.s->reserve(size);
          t_ = Binary;
          return *h_.s;
        default:
          NERROR("invalid operand");
      }
    }
    
    nstr& buffer(){
      switch(t_){
        case Binary:
          return *h_.s;
        default:
          NERROR("var does not hold a buffer");
      }
    }
    
    const nstr& buffer() const{
      switch(t_){
        case Binary:
          return *h_.s;
        default:
          NERROR("var does not hold a buffer");
      }
    }
    
    char* pack(size_t& length, bool compress=true) const;
    
    void unpack(char* buf, size_t size, bool compressed=true);
    
    void save(const nstr& path) const;
    
    void open(const nstr& path);
    
    static bool nearlyEqual(double a, double b, double e=1e-14){
      double d = a - b;
      if(d == 0){
        return true;
      }
      
      if(abs(d/(a+b)) < e){
        return true;
      }
      
      return false;
    }
    
    static bool nearInteger(double x, int64_t& i){
      i = x;
      if(nearlyEqual(i, x)){
        return true;
      }
      
      i = std::ceil(x);
      if(nearlyEqual(i, x)){
        return true;
      }
      
      return false;
    }
    
    // assumes that this has already been dereferenced and is a function
    void setFunc(NFunc fp) const{
      assert(t_ == Function);
      
      h_.f->fp = fp;
    }
    
    // assumes that this has already been dereferenced and is a function
    NFunc func() const{
      assert(t_ == Function);
      
      return h_.f->fp;
    }
    
    void sqrt(NObject* o=0);
    
    static nvar sqrt(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.sqrt(o);
      return ret;
    }
    
    void exp(NObject* o=0);
    
    static nvar exp(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.exp(o);
      return ret;
    }
    
    nvar pow(const nvar& x, NObject* o=0) const;
    
    static nvar pow(const nvar& y, const nvar& x, NObject* o=0){
      return y.pow(x, o);
    }
    
    void abs();
    
    static nvar abs(const nvar& x){
      nvar ret = x;
      ret.abs();
      return ret;
    }
    
    void log10(NObject* o=0);
    
    static nvar log10(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.log10(o);
      return ret;
    }
    
    void log(NObject* o=0);
    
    static nvar log(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.log(o);
      return ret;
    }
    
    void cos(NObject* o=0);
    
    static nvar cos(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.cos(o);
      return ret;
    }
    
    void acos(NObject* o=0);
    
    static nvar acos(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.acos(o);
      return ret;
    }
    
    void cosh(NObject* o=0);
    
    static nvar cosh(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.cosh(o);
      return ret;
    }
    
    void sin(NObject* o=0);
    
    static nvar sin(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.sin(o);
      return ret;
    }
    
    void asin(NObject* o=0);
    
    static nvar asin(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.asin(o);
      return ret;
    }
    
    void sinh(NObject* o=0);
    
    static nvar sinh(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.sinh(o);
      return ret;
    }
    
    void tan(NObject* o=0);
    
    static nvar tan(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.tan(o);
      return ret;
    }
    
    void atan(NObject* o=0);
    
    static nvar atan(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.atan(o);
      return ret;
    }
    
    void tanh(NObject* o=0);
    
    static nvar tanh(const nvar& x, NObject* o=0){
      nvar ret = x;
      ret.tanh(o);
      return ret;
    }
    
    void floor();
    
    static nvar floor(const nvar& x){
      nvar ret = x;
      ret.floor();
      return ret;
    }
    
    void ceil();
    
    static nvar ceil(const nvar& x){
      nvar ret = x;
      ret.ceil();
      return ret;
    }
    
    void merge(const nvar& x){
      switch(t_){
        case Map:
          switch(x.t_){
            case Map:
              h_.m->merge(*x.h_.m);
              break;
            case Multimap:
              touchMultimap();
              h_.mm->merge(*x.h_.mm);
              break;
            case Function:
              if(x.h_.f->m){
                h_.m->merge(*x.h_.f->m);
              }
              break;
            case HeadMap:
              merge(*h_.hm->m);
              break;
            case HeadSequenceMap:
              merge(*h_.hsm->m);
              break;
            case Reference:
              merge(*h_.ref->v);
              break;
            case Pointer:
              merge(*h_.vp);
              break;
            default:
              break;
          }
          break;
        case Multimap:
          switch(x.t_){
            case Map:
              h_.mm->insert(x.h_.m->begin(), x.h_.m->end());
              break;
            case Multimap:
              h_.mm->merge(*x.h_.mm);
              break;
            case Function:
              if(x.h_.f->m){
                h_.mm->insert(x.h_.f->m->begin(), x.h_.f->m->end());
              }
              break;
            case HeadMap:
              merge(*h_.hm->m);
              break;
            case HeadSequenceMap:
              merge(*h_.hsm->m);
              break;
            case Reference:
              merge(*h_.ref->v);
              break;
            case Pointer:
              merge(*h_.vp);
              break;
            default:
              break;
          }
          break;
        case Function:
          switch(x.t_){
            case Map:
              if(h_.f->m){
                h_.f->m->merge(*x.h_.m);
              }
              else{
                h_.f->m = new nmap(*x.h_.m);
              }
              break;
            case Multimap:
              NERROR("right operand is invalid");
            case Function:
              if(h_.f->m){
                h_.f->m->merge(*x.h_.f->m);
              }
              else{
                h_.f->m = new nmap(*x.h_.f->m);
              }
              break;
            case HeadMap:
              merge(*h_.hm->m);
              break;
            case HeadSequenceMap:
              merge(*h_.hsm->m);
              break;
            case Reference:
              merge(*h_.ref->v);
              break;
            case Pointer:
              merge(*h_.vp);
              break;
            default:
              break;
          }
          break;
        case HeadMap:
          h_.hm->m->merge(x);
          break;
        case HeadSequenceMap:
          h_.hsm->m->merge(x);
          break;
        case Reference:
          h_.ref->v->merge(x);
          break;
        case Pointer:
          h_.vp->merge(x);
          break;
        default:
          switch(x.t_){
            case Map:
            case Multimap:
            case Function:
            case HeadMap:
            case HeadSequenceMap:
              touchMap();
              merge(x);
              break;
            case Reference:
              merge(*x.h_.ref->v);
              break;
            case Pointer:
              merge(*x.h_.vp);
              break;
            default:
              break;
          }
      }
    }
    
    void outerMerge(const nvar& x){
      switch(t_){
        case Map:
          switch(x.t_){
            case Map:
              h_.m->outerMerge(*x.h_.m);
              break;
            case Multimap:
              touchMultimap();
              h_.mm->merge(*x.h_.mm);
              break;
            case Function:
              if(x.h_.f->m){
                h_.m->outerMerge(*x.h_.f->m);
              }
              break;
            case HeadMap:
              outerMerge(*h_.hm->m);
              break;
            case HeadSequenceMap:
              outerMerge(*h_.hsm->m);
              break;
            case Reference:
              outerMerge(*h_.ref->v);
              break;
            case Pointer:
              outerMerge(*h_.vp);
              break;
            default:
              break;
          }
          break;
        case Multimap:
          switch(x.t_){
            case Map:
              h_.mm->insert(x.h_.m->begin(), x.h_.m->end());
              break;
            case Multimap:
              h_.mm->merge(*x.h_.mm);
              break;
            case Function:
              if(x.h_.f->m){
                h_.mm->insert(x.h_.f->m->begin(), x.h_.f->m->end());
              }
              break;
            case HeadMap:
              outerMerge(*h_.hm->m);
              break;
            case HeadSequenceMap:
              outerMerge(*h_.hsm->m);
              break;
            case Reference:
              outerMerge(*h_.ref->v);
              break;
            case Pointer:
              outerMerge(*h_.vp);
              break;
            default:
              break;
          }
          break;
        case Function:
          switch(x.t_){
            case Map:
              if(h_.f->m){
                h_.f->m->outerMerge(*x.h_.m);
              }
              else{
                h_.f->m = new nmap(*x.h_.m);
              }
              break;
            case Multimap:
              NERROR("right operand is invalid");
            case Function:
              if(h_.f->m){
                h_.f->m->outerMerge(*x.h_.f->m);
              }
              else{
                h_.f->m = new nmap(*x.h_.f->m);
              }
              break;
            case HeadMap:
              outerMerge(*h_.hm->m);
              break;
            case HeadSequenceMap:
              outerMerge(*h_.hsm->m);
              break;
            case Reference:
              outerMerge(*h_.ref->v);
              break;
            case Pointer:
              outerMerge(*h_.vp);
              break;
            default:
              break;
          }
          break;
        case HeadMap:
          h_.hm->m->outerMerge(x);
          break;
        case HeadSequenceMap:
          h_.hsm->m->outerMerge(x);
          break;
        case Reference:
          h_.ref->v->outerMerge(x);
          break;
        case Pointer:
          h_.vp->outerMerge(x);
          break;
        default:
          switch(x.t_){
            case Map:
            case Multimap:
            case Function:
            case HeadMap:
            case HeadSequenceMap:
              touchMap();
              outerMerge(x);
              break;
            case Reference:
              outerMerge(*x.h_.ref->v);
              break;
            case Pointer:
              outerMerge(*x.h_.vp);
              break;
            default:
              break;
          }
      }
    }
    
    static nvar parseFuncSpec(const char* fs){
      nstr name;
      bool done = false;
      char c;
      
      size_t i = 0;
      for(;;){
        c = fs[i];
        
        switch(c){
          case '(':
            done = true;
            break;
          case ' ':
            name = "";
            break;
          case '\0':
            NERROR("invalid func spec" + nstr(fs));
            break;
          default:
            name += c;
            break;
        }

        ++i;
        
        if(done){
          break;
        }
      }
      
      size_t n = 0;
      bool first = true;
      
      for(;;){
        c = fs[i];
        
        switch(c){
          case ',':
            ++n;
            break;
          case ')':
            break;
          case '\0':
            NERROR("invalid func spec" + nstr(fs));
            break;
          default:
            if(first){
              ++n;
              first = false;
            }
            else{
              first = false;
            }
            break;
        }
        
        ++i;
      }
      
      return {name, n};
    }
  
  private:
    char* pack_(char* buf, size_t& size, size_t& pos) const;
    void unpack_(char* buf, size_t& pos);
    
    Type t_;
    Head h_;
  };
  
  inline nvar nfunc(const nstr& f){
    return new nvar(f, nvar::Func);
  }
  
  inline nvar nfunc(const char* f){
    return new nvar(f, nvar::Func);
  }
  
  inline nvar nsym(const nstr& s){
    return new nvar(s, nvar::Sym);
  }
  
  inline nvar nsym(const char* s){
    return new nvar(s, nvar::Sym);
  }
  
  inline nvar nref(const nvar& v){
    switch(v.fullType()){
      case nvar::Reference:
        return v;
      case nvar::Pointer:
        return new nvar(*v);
      default:
        return new nvar(v);
    }
  }
  
  inline nvar nptr(nvar& v){
    switch(v.fullType()){
      case nvar::Reference:
      case nvar::Pointer:
        return v;
      default:
        return new nvar(&v, nvar::Ptr);
    }
  }
  
  inline nvar nml(const nstr& code){
    return nvar::fromStr(code);
  }
  
  inline std::ostream& operator<<(std::ostream& ostr, const nvar& v){
    v.streamOutput_(ostr);
    return ostr;
  }
  
  inline bool operator<(double v1, const nvar& v2){
    return v2 > v1;
  }
  
  inline bool operator<(int v1, const nvar& v2){
    return v2 > v1;
  }
  
  inline bool operator<=(double v1, const nvar& v2){
    return v2 >= v1;
  }
  
  inline bool operator<=(int v1, const nvar& v2){
    return v2 >= v1;
  }
  
  inline bool operator>(double v1, const nvar& v2){
    return v2 < v1;
  }
  
  inline bool operator>(int v1, const nvar& v2){
    return v2 < v1;
  }
  
  inline bool operator>=(double v1, const nvar& v2){
    return v2 <= v1;
  }
  
  inline bool operator>=(int v1, const nvar& v2){
    return v2 <= v1;
  }
  
  inline bool operator==(double v1, const nvar& v2){
    return v2 == v1;
  }
  
  inline bool operator==(int v1, const nvar& v2){
    return v2 == v1;
  }
  
  inline bool operator!=(double v1, const nvar& v2){
    return v2 != v1;
  }
  
  inline bool operator!=(int v1, const nvar& v2){
    return v2 != v1;
  }
  
  inline nvar operator+(double v1, const nvar& v2){
    return v2 + v1;
  }
  
  inline nvar operator+(int v1, const nvar& v2){
    return v2 + v1;
  }
  
  inline nvar operator-(double v1, const nvar& v2){
    return -v2 + v1;
  }
  
  inline nvar operator-(int v1, const nvar& v2){
    return -v2 + v1;
  }
  
  inline nvar operator*(double v1, const nvar& v2){
    return v2 * v1;
  }
  
  inline nvar operator*(int v1, const nvar& v2){
    return v2 * v1;
  }
  
  inline nvar operator/(double v1, const nvar& v2){
    return nvar(v1) / v2;
  }
  
  inline nvar operator/(int v1, const nvar& v2){
    return nvar(v1) / v2;
  }
  
  inline nvar operator%(double v1, const nvar& v2){
    return nvar(v1) % v2;
  }
  
  inline nvar operator%(int v1, const nvar& v2){
    return nvar(v1) % v2;
  }
  
  inline nstr operator+(const char* s, const nvar& v){
    return s + v.toStr();
  }
  
  inline nstr& operator+=(nstr& s, const char* v){
    return s += v;
  }
  
  inline nstr& operator+=(nstr& s, const nvar& v){
    return s += v.toStr();
  }
  
} // end namespace neu

#endif // NEU_N_VAR_H