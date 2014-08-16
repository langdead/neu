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
 
 
Neu, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. We hope you will find
Neu powerful, useful to make money or otherwise, and fun! If so,
please consider making a donation via PayPal to: neu@andrometa.net

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

#include <neu/NDatabase.h>

#include <atomic>

#include <neu/NEncoder.h>
#include <neu/NHashMap.h>
#include <neu/NSys.h>
#include <neu/NRWMutex.h>
#include <neu/NReadGuard.h>
#include <neu/NWriteGuard.h>
#include <neu/NRegex.h>

using namespace std;
using namespace neu;

namespace{
  
  static const uint8_t DataIndexType = 255;
  
  static const size_t MAX_CHUNK_SIZE = 32768;
  static const size_t MAX_CHUNKS = 1024;
  //static const size_t MAX_CHUNK_SIZE = 10;
  //static const size_t MAX_CHUNKS = 10;
  static const size_t MAX_DATA_SIZE = 16777216;
  static const size_t DEFAULT_MEMORY_LIMIT = 1024;
  
  static const size_t SPLIT_CHUNK_SIZE = MAX_CHUNK_SIZE - 2;
  
  static const size_t EXTRA_DATA_BUFFER_SIZE = 8192;
  static const size_t MIN_COMPRESS_SIZE = 1000;
  
  static const uint32_t COMPRESS_FLAG = 0x1;
  
  template<typename T>
  void min(T& m){
    m = numeric_limits<T>::min();
  }
  
  void min(double& m){
    m = -numeric_limits<double>::infinity();
  }
  
  void min(float& m){
    m = -numeric_limits<float>::infinity();
  }
  
  template<typename T>
  void max(T& m){
    m = numeric_limits<T>::max();
  }
  
  void max(double& m){
    m = numeric_limits<double>::infinity();
  }
  
  void max(float& m){
    m = numeric_limits<float>::infinity();
  }
  
  typedef typename NTable::RowId RowId;
  
  typedef typename NTable::RowSet RowSet;
  
  typedef NHashMap<RowId, RowId> UpdateMap;
  
  typedef function<int(RowId, const nvar&)> QueryFunc_;
  
  class Pageable{
  public:
    virtual size_t memoryUsage() = 0;
    
    virtual void store() = 0;
  };
  
  typedef NMap<uint64_t, pair<Pageable*, size_t>> PMap;
  
  nstr oldPath(const nstr& path){
    nstr f = NSys::basename(path);
    return NSys::stripPath(path) + "/old/" + f;
  }
  
} // end namespace

namespace neu{
  
  class NTable_;
  
  class NDatabase_{
  public:
    NDatabase_(NDatabase* o, const nstr& path, bool create);
    
    NTable* addTable(const nstr& tableName);
    
    NTable* getTable(const nstr& tableName);

    RowId nextRowId(){
      return nextRowId_++;
    }
    
    void compact();
    
    const nstr& path(){
      return path_;
    }

    uint64_t read(){
      return tick_++;
    }
    
    uint64_t write(){
      return tick_++;
    }

    void setMemoryLimit(size_t limit){
      memoryLimit_ = 1048576*limit;
    }
    
    size_t memoryUsage(PMap& pm);
    
    void checkMemory();
    
    void save();
    
    void saveMeta();
    
    void rollback();
    
    void safeRemove(const nstr& path){
      if(!path.beginsWith(path_)){
        cerr << "Error: attempted to remove file outside "
        "database: " << path << endl;
        exit(1);
      }
      
      remove(path_.c_str());
    }
    
  private:
    typedef NMap<nstr, NTable_*> TableMap_;
    
    NDatabase* o_;
    nstr path_;
    nstr metaPath_;
    RowId nextRowId_;
    TableMap_ tableMap_;
    atomic<uint64_t> tick_;
    size_t memoryLimit_;
  }; // end class NDatabase_
  
  class NTable_{
  public:
    
    typedef uint8_t Action;
    
    static const Action None = 0x0;
    static const Action Remap = 0x1;
    static const Action Split = 0x2;
    static const Action RemapSplit = 0x3;
    static const Action Append = 0x4;
    
    class IndexBase{
    public:
      IndexBase(uint8_t type)
      : type_(type){
        
      }
      
      virtual ~IndexBase(){}
      
      uint8_t type(){
        return type_;
      }
      
      void setUnique(bool flag){
        unique_ = flag;
      }
      
      bool unique(){
        return unique_;
      }
      
      void setAutoErase(bool flag){
        autoErase_ = flag;
      }
      
      bool autoErase(){
        return autoErase_;
      }
      
      virtual size_t memoryUsage(PMap& pm) = 0;
      
      virtual void save(bool manual) = 0;
      
      virtual void saveMeta() = 0;
      
      virtual void dump(){}
      
      virtual const nstr& path() const = 0;
      
      virtual void rollback() = 0;

      virtual void clean() = 0;
      
    private:
      uint8_t type_;
    protected:
      bool unique_;
      bool autoErase_;
    }; // end class IndexBase
        
    template<class R, class V>
    class Page : public Pageable{
    public:
      typedef function<void(R& r)> TraverseFunc;
      
      class Chunk{
      public:
        
        Chunk(bool unique)
        : unique_(unique){}
        
        Chunk(bool unique, R* data, size_t n)
        : unique_(unique),
        chunk_(data, data + n){}
        
        typedef function<void(R& r)> TraverseFunc;
        
        Action findInsert(const V& value, size_t& index){
          index = 0;
          
          R* record;
          size_t length = chunk_.size();
          size_t start = 0;
          size_t end = length;
          bool after = false;
          
          while(end > start){
            index = start + (end - start)/2;

            record = &chunk_[index];
            
            if(value < record->value){
              end = index;
              after = false;
            }
            else{
              start = index + 1;
              after = true;
            }
          }
          
          index = after ? index + 1 : index;

          Action action = None;
          
          if(length >= SPLIT_CHUNK_SIZE){
            action |= Split;
          }

          if(index == length){
            action |= Append;
          }
          else if(index == 0){
            action |= Remap;
          }

          return action;
        }
        
        size_t find(const V& value){
          size_t index = 0;
          
          R* record;
          size_t length = chunk_.size();
          size_t start = 0;
          size_t end = length;
          
          while(end > start){
            index = start + (end - start)/2;
            
            record = &chunk_[index];
            
            if(value < record->value){
              end = index;
            }
            else{
              start = index + 1;
            }
          }

          return index;
        }

        R* get(const V& value){
          R* record = 0;
          
          size_t start = 0;
          size_t end = chunk_.size();
          size_t index = 0;
          
          while(end > start){
            index = start + (end - start)/2;
            
            record = &chunk_[index];

            if(value < record->value){
              end = index;
            }
            else if(value > record->value){
              start = index + 1;
            }
            else{
              return record;
            }
          }

          return record->value == value ? record : 0;
        }
        
        Action insert(const R& record){
          size_t index;
          Action action = findInsert(record.value, index);

          if(unique_ && chunk_[index].value == record.value){
            NERROR("non-unique value: " + nvar(record.value));
          }
          
          if(findInsert(record.value, index) & Append){
            chunk_.push_back(record);
          }
          else{
            chunk_.insert(chunk_.begin() + index, record);
          }
          
          return action;
        }

        Action push(const R& record){
          if(unique_ && chunk_.back().value == record.value){
            NERROR("non-unique value: " + nvar(record.value));
          }
          
          chunk_.push_back(record);
          Action action = Append;
          
          size_t size = chunk_.size();
          
          if(size >= MAX_CHUNK_SIZE){
            action |= Split;
          }
          else if(size == 1){
            action |= Remap;
          }
          
          return action;
        }
        
        size_t size(){
          return chunk_.size();
        }
        
        Chunk* split(){
          Chunk* c = new Chunk(unique_);
          
          auto itr = chunk_.begin();
          itr += chunk_.size()/2;
          
          c->chunk_.insert(c->chunk_.end(), itr, chunk_.end());
          chunk_.erase(itr, chunk_.end());
          
          return c;
        }
        
        V min() const{
          assert(!chunk_.empty());
         
          return chunk_.front().value;
        }
        
        void traverse(TraverseFunc f){
          size_t size = chunk_.size();
          for(size_t i = 0; i < size; ++i){
            f(chunk_[i]);
          }
        }
        
        void dump(){
          for(size_t i = 0; i < chunk_.size(); ++i){
            const R& r = chunk_[i];
            r.dump();
          }
        }
        
        int query(const V& start, QueryFunc_ f){
          size_t index = find(start);
          
          R* r;
          RowId rowId;
          int s;
          size_t end = chunk_.size() - 1;
          
          for(;;){
            r = &chunk_[index];
            rowId = r->rowId;
            
            s = f(rowId, r->exists() ? nvar(r->value) : none);

            if(s < 0){
              if(index == 0){
                return s;
              }
              --index;
            }
            else if(s > 0){
              if(index == end){
                return s;
              }
              ++index;
            }
            else{
              return 0;
            }
          }
        }
        
        R* data(){
          return chunk_.data();
        }
        
      private:
        typedef NVector<R> Chunk_;
        
        Chunk_ chunk_;
        bool unique_;
      };
      
      Page(NDatabase_* d,
           IndexBase* index,
           uint64_t id,
           bool create)
      : index_(index),
      d_(d),
      id_(id),
      loaded_(create),
      new_(create),
      firstChunk_(0),
      tick_(0),
      memoryUsage_(0){
        path_ = index_->path() + "/" + nvar(id);
      }
      
      ~Page(){
        for(auto& itr : chunkMap_){
          delete itr.second;
        }
      }
      
      void read(){
        if(!loaded_){
          load();
        }
        
        tick_ = d_->read();
      }
      
      uint64_t id(){
        return id_;
      }
      
      void write(){
        if(!loaded_){
          load();
        }
        
        tick_ = d_->write();
      }
      
      size_t memoryUsage(){
        return memoryUsage_;
      }
      
      void store(){
        save(false);
        
        for(auto& itr : chunkMap_){
          delete itr.second;
        }
        
        chunkMap_.clear();
        
        loaded_ = false;
        memoryUsage_ = 0;
      }
      
      void save(bool manual){
        if(manual){
          new_ = false;
        }
        else if(!new_){
          nstr op = oldPath(path_);
          
          if(!NSys::exists(op)){
            NSys::rename(path_, op);
          }
        }
        
        FILE* file = fopen(path_.c_str(), "wb");
        
        if(!file){
          NERROR("failed to create page file: " + path_);
        }

        uint32_t numChunks = chunkMap_.size();
        
        uint32_t n = fwrite(&numChunks, 1, 4, file);
        if(n != 4){
          NERROR("failed to write to page file [1]: " + path_);
        }

        uint32_t chunkSize;
        uint32_t dataSize;
        for(auto& itr : chunkMap_){
          Chunk* chunk = itr.second;

          chunkSize = chunk->size();
          n = fwrite(&chunkSize, 1, 4, file);
          if(n != 4){
            NERROR("failed to write to page file [2]: " + path_);
          }
          
          dataSize = sizeof(R)*chunkSize;
          
          n = fwrite(chunk->data(), 1, dataSize, file);
          if(n != dataSize){
            NERROR("failed to write to page file [3]: " + path_);
          }
        }
        
        fclose(file);
      }
      
      void load(){
        FILE* file = fopen(path_.c_str(), "rb");
        
        uint32_t numChunks;
        uint32_t n = fread(&numChunks, 1, 4, file);
        if(n != 4){
          NERROR("failed to read page file [1]: " + path_);
        }
        
        uint32_t chunkSize;
        uint32_t dataSize;
        R buf[MAX_CHUNK_SIZE];
        
        firstChunk_ = 0;
        memoryUsage_ = 0;
        
        for(size_t i = 0; i < numChunks; ++i){
          n = fread(&chunkSize, 1, 4, file);
          if(n != 4){
            NERROR("failed to read page file [2]: " + path_);
          }
        
          dataSize = sizeof(R)*chunkSize;
          memoryUsage_ += dataSize;
          
          n = fread(buf, 1, dataSize, file);
          if(n != dataSize){
            NERROR("failed to read page file [3]: " + path_);
          }
          
          Chunk* chunk = new Chunk(index_->unique(), buf, chunkSize);
          if(!firstChunk_){
            firstChunk_ = chunk;
          }
          
          chunkMap_.insert({chunk->min(), chunk});
        }
        fclose(file);
        
        loaded_ = true;
        
        d_->checkMemory();
      }
      
      bool handleFirst(const R& record){
        if(firstChunk_){
          return false;
        }
        
        firstChunk_ = new Chunk(index_->unique());
        firstChunk_->push(record);
        chunkMap_.insert({record.value, firstChunk_});
        memoryUsage_ += sizeof(R);
        return true;
      }
      
      Action insert(const R& record){
        write();
        
        if(handleFirst(record)){
          return Remap;
        }

        auto itr = findChunk(record.value);
        
        Chunk* chunk = itr->second;
        
        Action action = chunk->insert(record);
        memoryUsage_ += sizeof(R);
        
        if(action & Remap && chunk == firstChunk_){
          chunkMap_.erase(itr);
          chunkMap_.insert({record.value, chunk});
        }
        
        if(action & Split){
          if(chunkMap_.size() >= MAX_CHUNKS){
            return Split;
          }
          else{
            Chunk* c = chunk->split();
            chunkMap_.insert({c->min(), c});
            return None;
          }
        }

        return None;
      }
      
      Action push(const R& record){
        write();
        
        if(handleFirst(record)){
          return Remap;
        }

        auto itr = chunkMap_.end();
        --itr;
        Chunk* chunk = itr->second;
        
        Action action = chunk->push(record);
        memoryUsage_ = sizeof(R);
        
        if(action & Split){
          if(chunkMap_.size() >= MAX_CHUNKS){
            return Split;
          }
          else{
            Chunk* c = chunk->split();
            chunkMap_.insert({c->min(), c});
            return None;
          }
        }
        
        return None;
      }
      
      R* get(const V& value){
        read();
        
        auto itr = findChunk(value);
        if(itr == chunkMap_.end()){
          return 0;
        }
        
        return itr->second->get(value);
      }
      
      Page* split(uint64_t id){
        Page* p = new Page(d_, index_, id, true);
        p->write();
        
        typename ChunkMap_::iterator itr;

        for(size_t i = 0; i < MAX_CHUNKS/2; ++i){
          itr = chunkMap_.end();
          --itr;
          
          p->chunkMap_.insert({itr->first, itr->second});
          chunkMap_.erase(itr);
          ++i;
        }
        
        size_t half = memoryUsage_/2;
        
        p->memoryUsage_ = half;
        memoryUsage_ = half;
        
        return p;
      }
      
      V min(){
        auto itr = chunkMap_.begin();

        if(itr == chunkMap_.end()){
          V m;
          ::min(m);
          return m;
        }
        
        return itr->second->min();
      }
      
      void traverse(TraverseFunc f){
        read();
        for(auto& itr : chunkMap_){
          itr.second->traverse(f);
        }
      }
      
      void dump(){
        read();
        
        for(auto& itr : chunkMap_){
          cout << "@@@ CHUNK LOWER: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
      int query(const V& start, QueryFunc_ f){
        read();
        
        auto itr = findChunk(start);
        assert(itr != chunkMap_.end());
        
        for(;;){
          int s = itr->second->query(start, f);
          
          if(s > 0){
            ++itr;
            if(itr == chunkMap_.end()){
              return s;
            }
          }
          else if(s < 0){
            if(itr == chunkMap_.begin()){
              return s;
            }
            else{
              --itr;
            }
          }
          else{
            return 0;
          }
        }
      }
      
      uint64_t tick(){
        return tick_;
      }
      
    private:
      typedef NMap<V, Chunk*> ChunkMap_;
      
      IndexBase* index_;
      NDatabase_* d_;
      uint64_t id_;
      bool loaded_;
      bool new_;
      ChunkMap_ chunkMap_;
      Chunk* firstChunk_;
      nstr path_;
      uint64_t tick_;
      size_t memoryUsage_;
      
      typename ChunkMap_::iterator findChunk(const V& v){
        auto itr = chunkMap_.upper_bound(v);
        return itr == chunkMap_.begin() ? itr : --itr;
      }
    }; // end class Page
    
    template<typename R, typename V>
    class Index : public IndexBase{
    public:
      typedef Page<R, V> IndexPage;
      
      typedef function<void(R& r)> TraverseFunc;
      
      Index(NDatabase_* d, uint8_t type, const nstr& path, bool create)
      : IndexBase(type),
      d_(d),
      path_(path),
      firstPage_(0){
        min(min_);
        
        metaPath_ = path_ + "/meta.nvar";
        
        if(create){
          nextPageId_ = 0;
          
          firstPage_ = new IndexPage(d_, this, nextPageId_++, true);
          pageMap_.insert({min_, firstPage_});
          
          if(NSys::exists(path_)){
            NERROR("index path exists: " + path_);
          }
          
          NSys::makeDir(path_);
          
          metaPath_ = path_ + "/meta.nvar";
          if(NSys::exists(metaPath_)){
            NERROR("index meta path exists: " + metaPath_);
          }
          
          NSys::makeDir(path_);
          
          nstr oldPath = path_ + "/old";
          if(NSys::exists(oldPath)){
            NERROR("index old path exists: " + oldPath);
          }
          
          NSys::makeDir(oldPath);
        }
        else{
          firstPage_ = 0;
          
          nvar m;
          m.open(metaPath_);
          
          nget(m, nextPageId_);
          nget(m, unique_);
          nget(m, autoErase_);
          
          const nhmap& pm = m["pageMap"];
          
          for(auto& itr : pm){
            uint64_t pageId = itr.first;
            V min = itr.second;
            IndexPage* page = new IndexPage(d_, this, pageId, false);
            pageMap_.insert({min, page});
          }
        }
      }
      
      virtual ~Index(){
        for(auto& itr : pageMap_){
          delete itr.second;
        }
      }
      
      void saveMeta(){
        nvar m;
        nput(m, nextPageId_);
        nput(m, unique_);
        nput(m, autoErase_);
        
        nhmap& pm = m("pageMap") = nhmap();
        
        for(auto& itr : pageMap_){
          IndexPage* page = itr.second;
          pm.insert({page->id(), page->min()});
        }
        
        m.save(metaPath_);
      }
      
      size_t memoryUsage(PMap& pm){
        size_t m = 0;
        for(auto& itr : pageMap_){
          IndexPage* p = itr.second;
          
          size_t mi = p->memoryUsage();
          
          if(mi > 0){
            pm.insert({p->tick(), {p, mi}});
            m += mi;
          }
        }
        
        return m;
      }
      
      void rollback(){
        nstr oldPath = path_ + "/old";
        
        nvec oldFiles;
        if(!NSys::dirFiles(oldPath, oldFiles)){
          NERROR("index failed to rollback[1]");
        }
        
        for(const nstr& p : oldFiles){
          nstr fromPath = oldPath + "/" + p;
          nstr toPath = path_ + "/" + p;
          NSys::rename(fromPath, toPath);
        }
        
        nvar m;
        m.open(metaPath_);
        
        const nhmap& pm = m["pageMap"];
        
        nvec newFiles;
        if(!NSys::dirFiles(path_, newFiles)){
          NERROR("index failed to rollback[2]");
        }
        
        NRegex r("\\d+");
        
        for(const nstr& p : oldFiles){
          if(r.match(p)){
            size_t pageId = nvar(p).toLong();
            
            if(!pm.hasKey(pageId)){
              nstr fullPath = path_ + "/" + p;
              d_->safeRemove(fullPath.c_str());
            }
          }
        }
      }
      
      void clean(){
        nstr oldPath = path_ + "/old";
        
        nvec oldFiles;
        if(!NSys::dirFiles(oldPath, oldFiles)){
          NERROR("index failed to clean[1]");
        }
        
        for(const nstr& p : oldFiles){
          nstr fullPath = oldPath + "/" + p;
          d_->safeRemove(fullPath.c_str());
        }
      }
      
      const nstr& path() const{
        return path_;
      }
      
      void save(bool manual){
        for(auto& itr : pageMap_){
          itr.second->save(manual);
        }
      }
      
      void insertRecord(const R& record){
        auto itr = findPage(record.value);
        
        IndexPage* page = itr->second;
        Action action = page->insert(record);
        
        if(action & Split){
          IndexPage* p = page->split(nextPageId_++);
          pageMap_.insert({p->min(), p});
        }
        else if(action & Remap && page != firstPage_){
          pageMap_.erase(itr);
          pageMap_.insert({page->min(), page});
        }
      }
      
      void pushRecord(const R& record){
        auto itr = pageMap_.end();
        --itr;
        IndexPage* page = itr->second;
        
        Action action = page->push(record);
        
        if(action & Split){
          IndexPage* p = page->split(nextPageId_++);
          pageMap_.insert({p->min(), p});
        }
        else if(action & Remap && page != firstPage_){
          pageMap_.erase(itr);
          pageMap_.insert({page->min(), page});
        }
      }
      
      R* getRecord(const V& value){
        return findPage(value)->second->get(value);
      }
      
      void traverse(TraverseFunc f){
        for(auto& itr : pageMap_){
          itr.second->traverse(f);
        }
      }

      void compact(Index& ni, const RowSet& rs){
        traverse([&](R& r){
          if(rs.hasKey(r.rowId)){
            return;
          }
          
          ni.pushRecord(r);
        });
      }
      
      void compact(Index& ni, const RowSet& rs, const UpdateMap& um){
        traverse([&](R& r){
          if(rs.hasKey(r.rowId)){
            return;
          }
          
          RowId rowId = r.value;
          if(rs.hasKey(rowId)){
            auto itr = um.find(rowId);
            if(itr != um.end()){
              r.value = itr->second;
            }
            else{
              if(autoErase()){
                return;
              }
              
              r.value = 0;
            }
          }
          
          ni.pushRecord(r);
        });
      }

      void query(const V& start, QueryFunc_ f){
        auto itr = findPage(start);
        assert(itr != pageMap_.end());
        
        for(;;){
          int s = itr->second->query(start, f);
          
          if(s > 0){
            ++itr;

            if(itr == pageMap_.end()){
              break;
            }
          }
          else if(s < 0){
            if(itr == pageMap_.begin()){
              break;
            }

            --itr;
          }
          else{
            break;
          }
        }
      }

      size_t memoryUsage(){
        size_t m = 0;
        for(auto& itr : pageMap_){
          m += itr.second->memoryUsage();
        }
        
        return m;
      }
      
      void dump(){
        cout << "@@@@ INDEX: " << path_ << endl;
        
        for(auto& itr : pageMap_){
          cout << "@@@@@@@ PAGE: " << itr.first << endl;
          itr.second->dump();
        }
      }
      
    private:
      typedef NMap<V, IndexPage*> PageMap_;
      
      NDatabase_* d_;
      V min_;
      uint64_t nextPageId_;
      PageMap_ pageMap_;
      IndexPage* firstPage_;
      nstr path_;
      nstr metaPath_;
      
      typename PageMap_::iterator findPage(const V& v){
        auto itr = pageMap_.upper_bound(v);
        return itr == pageMap_.begin() ? itr : --itr;
      }
    }; // end class Index

    class DataRecord{
    public:
      
      void set(RowId id, uint32_t dataId, uint32_t offset){
        remap = 0;
        value = id;
        rowId = uint64_t(dataId) << 32 | uint64_t(offset);
      }
      
      void erase(){
        remap = 1;
        rowId = 0;
      }
      
      void update(uint64_t newRowId){
        remap = 1;
        rowId = newRowId;
      }
      
      bool exists(){
        return !remap;
      }
      
      RowId value;
      uint64_t rowId : 63;
      uint64_t remap : 1;
      
      uint32_t offset() const{
        return rowId & 0xffffffff;
      }
      
      uint32_t dataId() const{
        return rowId >> 32;
      }
      
      void dump() const{
        if(remap){
          if(rowId == 0){
            cout << "deleted: " << value << endl;
          }
          else{
            cout << "updated: " << rowId << endl;
          }
        }
        else{
          
        }
        cout << "rowId: " << value << "; dataId: " << dataId() <<
        "; offset: " << offset() << endl;
      }
    };
    
    class DataIndex : public Index<DataRecord, RowId>{
    public:
      
      DataIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, DataIndexType, path, create),
      d_(d){

      }
      
      void insert(uint32_t dataId, uint32_t offset, RowId rowId){
        record_.set(rowId, dataId, offset);
        pushRecord(record_);
      }

      bool get(RowId rowId, uint32_t& dataId, uint32_t& offset){
        DataRecord* record = getRecord(rowId);
        if(record){
          dataId = record->dataId();
          offset = record->offset();
          return true;
        }
        
        return false;
      }
      
      bool exists(RowId rowId){
        DataRecord* record = getRecord(rowId);
        if(record){
          return record->exists();
        }
        
        return false;
      }
      
      void erase(RowId rowId){
        DataRecord* record = getRecord(rowId);
        
        if(record){
          record->erase();
        }
      }
      
      RowId update(RowId rowId){
        DataRecord* record = getRecord(rowId);
        if(record){
          RowId newRowId = d_->nextRowId();
          record->update(newRowId);
          return newRowId;
        }
        
        return 0;
      }
      
      void mapCompact(DataIndex& ni, RowSet& rs, UpdateMap& um){
        traverse([&](DataRecord& r){
          if(r.remap){
            rs.insert(r.value);
            if(r.rowId != 0){
              um.insert({r.value, RowId(r.rowId)});
            }
          }
          else{
            ni.pushRecord(r);
          }
        });
      }
      
    private:
      DataRecord record_;
      NDatabase_* d_;
    };

    struct Int64Record{
      int64_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class Int64Index : public Index<Int64Record, int64_t>{
    public:
      Int64Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Int64, path, create){
        
      }
      
      void insert(RowId rowId, int64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      Int64Record record_;
    };
    
    struct UInt64Record{
      uint64_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class UInt64Index : public Index<UInt64Record, uint64_t>{
    public:
      UInt64Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::UInt64, path, create){
        
      }
      
      void insert(RowId rowId, uint64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      UInt64Record record_;
    };
    
    struct RowRecord{
      RowId value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class RowIndex : public Index<RowRecord, RowId>{
    public:
      RowIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Row, path, create){
        
      }
      
      void insert(RowId rowId, uint64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      RowRecord record_;
    };
    
    struct Int32Record{
      int32_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class Int32Index : public Index<Int32Record, int32_t>{
    public:
      Int32Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Int32, path, create){
        
      }
      
      void insert(RowId rowId, int32_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
      RowId get(int32_t value){
        return 0;
      }
      
    private:
      Int32Record record_;
    };
    
    struct UInt32Record{
      uint32_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class UInt32Index : public Index<UInt32Record, uint32_t>{
    public:
      UInt32Index(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::UInt32, path, create){
        
      }
      
      void insert(RowId rowId, uint32_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      UInt32Record record_;
    };

    struct DoubleRecord{
      double value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class DoubleIndex : public Index<DoubleRecord, double>{
    public:
      DoubleIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Double, path, create){
        
      }
      
      void insert(RowId rowId, double value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      DoubleRecord record_;
    };
    
    struct FloatRecord{
      float value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class FloatIndex : public Index<FloatRecord, double>{
    public:
      FloatIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Float, path, create){
        
      }
      
      void insert(RowId rowId, float value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      FloatRecord record_;
    };
    
    struct HashRecord{
      uint64_t value;
      RowId rowId;
      
      void dump() const{
        cout << "value: " << value << "; rowId: " << rowId << endl;
      }
      
      bool exists(){
        return true;
      }
    };
    
    class HashIndex : public Index<HashRecord, uint64_t>{
    public:
      HashIndex(NDatabase_* d, const nstr& path, bool create)
      : Index(d, NTable::Hash, path, create){
        
      }
      
      void insert(RowId rowId, uint64_t value){
        record_.value = value;
        record_.rowId = rowId;
        
        insertRecord(record_);
      }
      
    private:
      HashRecord record_;
    };
    
    class Data : public Pageable{
    public:
      Data(NTable_* table, uint64_t id, size_t size=0)
      : table_(table),
      d_(table_->database()),
      data_(0),
      size_(size),
      new_(size == 0),
      id_(id),
      tick_(0){
        path_ = table_->path() + "/__data/" + nvar(id);
      }
      
      ~Data(){
        if(data_){
          free(data_);
        }
      }
      
      size_t size(){
        return size_;
      }
      
      size_t memoryUsage(){
        return size_;
      }
      
      uint64_t id(){
        return id_;
      }

      uint64_t tick(){
        return tick_;
      }
      
      void read(){
        load();
        tick_ = d_->read();
      }
      
      void write(){
        load();
        tick_ = d_->write();
      }
      
      void save(bool manual){
        if(manual){
          new_ = false;
        }
        else if(!new_){
          nstr op = oldPath(path_);
          
          if(!NSys::exists(op)){
            NSys::rename(path_, op);
          }
        }
        
        FILE* file = fopen(path_.c_str(), "wb");
        
        if(!file){
          NERROR("failed to create data file: " + path_);
        }
        
        uint32_t n = fwrite(data_, 1, size_, file);
        if(n != size_){
          NERROR("failed to write data file: " + path_);
        }
        
        fclose(file);
      }

      void store(){
        save(false);
        free(data_);
        data_ = 0;
      }
      
      void load(){
        if(data_ || new_){
          return;
        }
        
        FILE* file = fopen(path_.c_str(), "rb");
        
        if(!file){
          NERROR("failed to open data file: " + path_);
        }
        
        data_ = (char*)malloc(size_);
        
        uint32_t n = fread(data_, 1, size_, file);
        
        if(n != size_){
          NERROR("failed to read data file : " + path_);
        }
        
        fclose(file);
        
        d_->checkMemory();
      }

      uint32_t insert(RowId rowId, char* buf, uint32_t size, uint32_t flags){
        write();
        
        uint32_t offset = size_;
        
        if(data_){
          data_ = (char*)realloc(data_, size_ + size + 16);
        }
        else{
          data_ = (char*)malloc(size + 16);
        }

        memcpy(data_ + size_, &rowId, 8);
        size_ += 8;
        
        memcpy(data_ + size_, &size, 4);
        size_ += 4;

        memcpy(data_ + size_, &flags, 4);
        size_ += 4;
        
        memcpy(data_ + size_, buf, size);
        size_ += size;
        
        return offset;
      }
      
      void get(uint32_t offset, nvar& v){
        read();
        
        RowId rowId;
        memcpy(&rowId, data_ + offset, 8);
        offset += 8;
        
        uint32_t size;
        memcpy(&size, data_ + offset, 4);
        offset += 4;
        
        uint32_t flags;
        memcpy(&flags, data_ + offset, 4);
        offset += 4;
        
        bool compressed = flags & COMPRESS_FLAG;

        v.unpack(data_ + offset, size, compressed);
      }
      
      uint32_t compact(Data* newData, const RowSet& rs){
        RowId rowId;
        uint32_t size;
        uint32_t flags;
        uint32_t offset = 0;
        
        while(offset < size_){
          memcpy(&rowId, data_ + offset, 8);
          offset += 8;

          memcpy(&size, data_ + offset, 4);
          offset += 4;

          memcpy(&flags, data_ + offset, 4);
          offset += 4;
          
          if(!rs.hasKey(rowId)){
            newData->insert(rowId, data_ + offset, size, flags);
          }
          offset += size;
        }
        
        return offset;
      }
      
      void dump(){
        cout << "###### DUMP DATA" << endl;
        
        read();
        
        size_t offset = 0;
        while(offset < size_){
          RowId rowId;
          memcpy(&rowId, data_ + offset, 8);
          offset += 8;
          
          uint32_t size;
          memcpy(&size, data_ + offset, 4);
          offset += 4;

          uint32_t flags;
          memcpy(&flags, data_ + offset, 4);
          offset += 4;
          
          cout << "### ";

          bool compressed = flags & COMPRESS_FLAG;
          
          nvar v;
          v.unpack(data_ + offset, size, compressed);

          cout << v << endl;
          
          offset += size;
        }
      }
      
    private:
      NTable_* table_;
      NDatabase_* d_;
      uint32_t size_;
      bool new_;
      char* data_;
      uint64_t id_;
      nstr path_;
      size_t tick_;
    }; // end class Data
    
    NTable_(NTable* o, NDatabase_* d, const nstr& path, bool create)
    : o_(o),
    d_(d),
    path_(path),
    lastData_(0){
      
      if(create){
        nextDataId_ = 0;
        
        if(NSys::exists(path_)){
          NERROR("table path exists: " + path_);
        }
        
        NSys::makeDir(path_);
        
        metaPath_ = path_ + "/meta.nvar";
        
        if(NSys::exists(metaPath_)){
          NERROR("table meta path exists: " + metaPath_);
        }
        
        dataPath_ = path_ + "/__data";
        
        if(NSys::exists(dataPath_)){
          NERROR("table data path exists: " + dataPath_);
        }
        
        NSys::makeDir(dataPath_);
        
        nstr op = dataPath_ + "/old";
        if(NSys::exists(op)){
          NERROR("table old data path exists: " + op);
        }
        
        NSys::makeDir(op);
        
        dataMetaPath_ = dataPath_ + "/meta.nvar";
        
        if(NSys::exists(dataMetaPath_)){
          NERROR("table data meta path exists: " + dataMetaPath_);
        }

        dataIndex_ = new DataIndex(d_, path_ + "/__data.index", true);
        
        saveMeta();
      }
      else{
        metaPath_ = path_ + "/meta.nvar";
        
        if(!NSys::exists(metaPath_)){
          NERROR("table meta path not found: " + metaPath_);
        }
        
        dataPath_ = path_ + "/__data";
        
        if(!NSys::exists(dataPath_)){
          NERROR("table data path not found: " + metaPath_);
        }
        
        dataMetaPath_ = dataPath_ + "/meta.nvar";
        if(!NSys::exists(dataMetaPath_)){
          NERROR("table data meta path not found: " + dataMetaPath_);
        }
                
        dataIndex_ = new DataIndex(d_, path_ + "/__data.index", false);
        
        nvar m;
        m.open(metaPath_);
        nget(m, nextDataId_);
        
        m = undef;
        m.open(dataMetaPath_);
        const nhmap& dm = m;
        
        for(auto& itr : dm){
          uint64_t dataId = itr.first;
          size_t size = itr.second;
          
          dataMap_.insert({dataId, new Data(this, dataId, size)});
        }
        
        nvec files;
        if(!NSys::dirFiles(path_, files)){
          NERROR("failed to read table[1]: " + path_);
        }
        
        NRegex r("(.+?)\\.(\\d+)\\.index");
        
        for(const nstr& p : files){
          nvec m;
          if(r.match(p, m)){
            nstr indexName = NSys::fileName(m[1]);
            nstr fullPath = path_ + "/" + p;
            uint8_t type = m[2].toLong();
            
            IndexBase* index;
            
            switch(type){
              case NTable::Int32:
                index = new Int32Index(d_, fullPath, false);
                break;
              case NTable::UInt32:
                index = new UInt32Index(d_, fullPath, false);
                break;
              case NTable::Int64:
                index = new Int64Index(d_, fullPath, false);
                break;
              case NTable::UInt64:
                index = new UInt64Index(d_, fullPath, false);
                break;
              case NTable::Float:
                index = new FloatIndex(d_, fullPath, false);
                break;
              case NTable::Double:
                index = new DoubleIndex(d_, fullPath, false);
                break;
              case NTable::Row:
                index = new RowIndex(d_, fullPath, false);
                break;
              case NTable::Hash:
                index = new HashIndex(d_, fullPath, false);
                break;
              default:
                NERROR("invalid index type: " + fullPath);
            }
            
            indexMap_.insert({indexName, index});
          }
        }
      }
    }
    
    const nstr& path(){
      return path_;
    }
    
    void readLock_(){
      mutex_.readLock();
    }
    
    void writeLock_(){
      mutex_.writeLock();
    }
    
    void unlock_(){
      mutex_.unlock();
    }
    
    void rollback(){
      NWriteGuard guard(mutex_);
      
      nstr oldPath = dataPath_ + "/old";
      
      nvec oldFiles;
      if(!NSys::dirFiles(oldPath, oldFiles)){
        NERROR("table failed to rollback[1]");
      }
      
      for(const nstr& p : oldFiles){
        nstr fromPath = oldPath + "/" + p;
        nstr toPath = dataPath_ + "/" + p;
        NSys::rename(fromPath, toPath);
      }
      
      nvar m;
      m.open(dataMetaPath_);
      const nhmap& dm = m;
      
      nvec newFiles;
      if(!NSys::dirFiles(dataPath_, newFiles)){
        NERROR("table failed to rollback[2]");
      }
      
      NRegex r("\\d+");
      
      for(const nstr& p : oldFiles){
        if(r.match(p)){
          size_t dataId = nvar(p).toLong();
          
          if(!dm.hasKey(dataId)){
            nstr fullPath = dataPath_ + "/" + p;
            d_->safeRemove(fullPath);
          }
        }
      }
      
      for(auto& itr : indexMap_){
        itr.second->rollback();
      }
    }
    
    void clean(){
      nstr oldPath = dataPath_ + "/old";
      
      nvec oldFiles;
      if(!NSys::dirFiles(oldPath, oldFiles)){
        NERROR("index failed to clean[1]");
      }
      
      for(const nstr& p : oldFiles){
        nstr fullPath = oldPath + "/" + p;
        d_->safeRemove(fullPath);
      }
      
      for(auto& itr : indexMap_){
        itr.second->clean();
      }
    }
    
    void addIndex(const nstr& indexName,
                  uint8_t indexType,
                  bool unique,
                  bool autoErase){
      NWriteGuard guard(mutex_);
      
      auto itr = indexMap_.find(indexName);
      
      if(itr != indexMap_.end()){
        NERROR("index exists: " + indexName);
      }

      IndexBase* index;
      
      nstr path = path_ + "/" + indexName + "." + nvar(indexType) + ".index";
      
      switch(indexType){
        case NTable::Int32:
          index = new Int32Index(d_, path, true);
          break;
        case NTable::UInt32:
          index = new UInt32Index(d_, path, true);
          break;
        case NTable::Int64:
          index = new Int64Index(d_, path, true);
          break;
        case NTable::UInt64:
          index = new UInt64Index(d_, path, true);
          break;
        case NTable::Float:
          index = new FloatIndex(d_, path, true);
          break;
        case NTable::Double:
          index = new DoubleIndex(d_, path, true);
          break;
        case NTable::Row:
          index = new RowIndex(d_, path, true);
          break;
        case NTable::Hash:
          index = new HashIndex(d_, path, true);
          break;
        default:
          NERROR("invalid index type");
      }
      
      index->setUnique(unique);
      index->setAutoErase(autoErase);
      
      indexMap_.insert({indexName, index});
    }
    
    RowId insert(nvar& row){
      NWriteGuard guard(mutex_);
      
      RowId rowId = d_->nextRowId();
      insert_(rowId, row);
      return rowId;
    }
    
    void insert_(RowId rowId, nvar& row){
      const nmap& m = row;
      for(auto& itr : m){
        const nvar& k = itr.first;
        const nvar& v = itr.second;
        
        if(k.isSymbol()){
          auto iitr = indexMap_.find(k);
          if(iitr != indexMap_.end()){
            IndexBase* index = iitr->second;
            
            switch(index->type()){
              case NTable::Int32:{
                Int32Index* i = static_cast<Int32Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::UInt32:{
                UInt32Index* i = static_cast<UInt32Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Int64:{
                Int64Index* i = static_cast<Int64Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::UInt64:{
                UInt64Index* i = static_cast<UInt64Index*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Float:{
                FloatIndex* i = static_cast<FloatIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Double:{
                DoubleIndex* i = static_cast<DoubleIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Row:{
                RowIndex* i = static_cast<RowIndex*>(index);
                i->insert(rowId, v);
                break;
              }
              case NTable::Hash:{
                HashIndex* i = static_cast<HashIndex*>(index);
                i->insert(rowId, v.hash());
                break;
              }
              default:
                assert(false);
            }
          }
        }
      }
      
      row("id") = rowId;

      uint32_t flags = 0;
      uint32_t size;
      char* buf = row.packWithParams(size, MIN_COMPRESS_SIZE);
      if(size > MIN_COMPRESS_SIZE){
        flags |= COMPRESS_FLAG;
      }
      
      Data* data;
      
      if(lastData_ && lastData_->size() + size <= MAX_DATA_SIZE){
        data = lastData_;
      }
      else{
        data = 0;
        
        for(auto& itr : dataMap_){
          Data* d = itr.second;
          if(d->size() + size <= MAX_DATA_SIZE){
            data = d;
          }
        }
        
        if(!data){
          uint64_t id = nextDataId_++;
          data = new Data(this, id);
          dataMap_.insert({id, data});
        }
      }
      
      uint32_t offset = data->insert(rowId, buf, size, flags);
      free(buf);
      
      dataIndex_->insert(data->id(), offset, rowId);
      lastData_ = data;
    }
    
    void update(nvar& row){
      NWriteGuard guard(mutex_);
      
      RowId rowId = row["id"];
      RowId newRowId = dataIndex_->update(rowId);
      
      if(newRowId == 0){
        NERROR("invalid row id: " + nvar(rowId));
      }
      
      row["id"] = newRowId;
      insert_(newRowId, row);
    }
    
    bool get(RowId rowId, nvar& row){
      NReadGuard guard(mutex_);
      
      uint32_t dataId;
      uint32_t offset;
      
      if(!dataIndex_->get(rowId, dataId, offset)){
        return false;
      }
      
      auto itr = dataMap_.find(dataId);
      assert(itr != dataMap_.end());
      
      Data* data = itr->second;
      data->get(offset, row);
      
      return true;
    }
    
    bool exists_(RowId rowId){
      return dataIndex_->exists(rowId);
    }
    
    NTable* outer(){
      return o_;
    }
    
    void erase(RowId rowId){
      NWriteGuard guard(mutex_);
      
      dataIndex_->erase(rowId);
    }
    
    void mapCompact(RowSet& rs, UpdateMap& um){
      DataIndex* newDataIndex = new DataIndex(d_, "changme", true);
      dataIndex_->mapCompact(*newDataIndex, rs, um);
      delete dataIndex_;
      dataIndex_ = newDataIndex;
    }

    void compact(const RowSet& rs, const UpdateMap& um){
      IndexMap_ newIndexMap_;
      
      for(auto& itr : indexMap_){
        IndexBase* oldIndex = itr.second;
        IndexBase* newIndex;
        
        switch(oldIndex->type()){
          case NTable::Int32:{
            Int32Index* ni = new Int32Index(d_, "changme", true);
            Int32Index* oi = static_cast<Int32Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::UInt32:{
            UInt32Index* ni = new UInt32Index(d_, "changme", true);
            UInt32Index* oi = static_cast<UInt32Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Int64:{
            Int64Index* ni = new Int64Index(d_, "changme", true);
            Int64Index* oi = static_cast<Int64Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::UInt64:{
            UInt64Index* ni = new UInt64Index(d_, "changme", true);
            UInt64Index* oi = static_cast<UInt64Index*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Float:{
            FloatIndex* ni = new FloatIndex(d_, "changme", true);
            FloatIndex* oi = static_cast<FloatIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Double:{
            DoubleIndex* ni = new DoubleIndex(d_, "changme", true);
            DoubleIndex* oi = static_cast<DoubleIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          case NTable::Row:{
            RowIndex* ni = new RowIndex(d_, "changme", true);
            RowIndex* oi = static_cast<RowIndex*>(oldIndex);
            oi->compact(*ni, rs, um);
            newIndex = ni;
            break;
          }
          case NTable::Hash:{
            HashIndex* ni = new HashIndex(d_, "changme", true);
            HashIndex* oi = static_cast<HashIndex*>(oldIndex);
            oi->compact(*ni, rs);
            newIndex = ni;
            break;
          }
          default:
            NERROR("invalid index type");
        }

        newIndex->setUnique(oldIndex->unique());
        newIndex->setAutoErase(oldIndex->autoErase());
        
        delete oldIndex;
        newIndexMap_.insert({itr.first, newIndex});
      }
      
      indexMap_ = move(newIndexMap_);
      
      Data* newData;
      Data* data;
      
      for(auto& itr : dataMap_){
        newData = new Data(this, itr.first);
        data = itr.second;
        itr.second = newData;
        delete data;
      }
    }
    
    size_t memoryUsage(PMap& pm){
      NReadGuard guard(mutex_);
      
      size_t m = 0;
      for(auto& itr : indexMap_){
        m += itr.second->memoryUsage(pm);
      }
      
      m += dataIndex_->memoryUsage(pm);
      
      size_t mi;
      for(auto& itr : dataMap_){
        Data* data = itr.second;
        
        mi = data->memoryUsage();
        if(mi > 0){
          pm.insert({data->tick(), {data, mi}});
        }
      }
      
      return m;
    }
    
    void save(){
      NReadGuard guard(mutex_);
      
      dataIndex_->save(true);
      
      for(auto& itr : indexMap_){
        itr.second->save(true);
      }
      
      for(auto& itr : dataMap_){
        itr.second->save(true);
      }
    }
    
    void saveMeta(){
      NReadGuard guard(mutex_);
      
      nvar m;
      nput(m, nextDataId_);
      m.save(metaPath_);
      
      nvar dm = nhmap();
      
      Data* data;
      for(auto& itr : dataMap_){
        data = itr.second;
        dm(itr.first) = data->size();
      }
      
      dm.save(dataMetaPath_);

      dataIndex_->saveMeta();
      
      for(auto& itr : indexMap_){
        itr.second->saveMeta();
      }
    }
    
    void query_(const nstr& indexName,
                const nvar& start,
                QueryFunc_ f){

      IndexBase* index;
      if(indexName == "__data"){
        index = dataIndex_;
      }
      else{
        auto itr = indexMap_.find(indexName);
        if(itr == indexMap_.end()){
          NERROR("invalid index: " + indexName);
        }
        index = itr->second;
      }
      
      switch(index->type()){
        case NTable::Int32:{
          Int32Index* i = static_cast<Int32Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::UInt32:{
          UInt32Index* i = static_cast<UInt32Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Int64:{
          Int64Index* i = static_cast<Int64Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::UInt64:{
          UInt64Index* i = static_cast<UInt64Index*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Float:{
          FloatIndex* i = static_cast<FloatIndex*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Double:{
          DoubleIndex* i = static_cast<DoubleIndex*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Row:{
          RowIndex* i = static_cast<RowIndex*>(index);
          i->query(start, f);
          break;
        }
        case NTable::Hash:{
          HashIndex* i = static_cast<HashIndex*>(index);
          i->query(start, f);
          break;
        }
        case DataIndexType:{
          DataIndex* i = static_cast<DataIndex*>(index);
          i->query(start, f);
          break;
        }
        default:
          assert(false);
      }
    }
    
    void query(const nstr& indexName,
               const nvar& start,
               NTable::QueryFunc qf){
      NReadGuard guard(mutex_);
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;

        bool success = get(rowId, row);
        assert(success);
        return qf(row);
      };
      
      query_(indexName, start, f);
    }
    
    void indexQuery(const nstr& indexName,
                    const nvar& start,
                    const nvar& end,
                    RowSet& rs){
      NReadGuard guard(mutex_);
      
      if(start > end){
        NERROR("invalid start/end");
      }
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        if(v > end){
          return 0;
        }
        
        if(exists_(rowId)){
          rs.insert(rowId);
        }
        
        return 1;
      };
      
      query_(indexName, start, f);
    }
    
    void traverseStart(NTable::QueryFunc qf){
      NReadGuard guard(mutex_);
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;
        bool success = get(rowId, row);
        assert(success);
        return qf(row);
      };
      
      query_("__data", 1, f);
    }
    
    void traverseEnd(NTable::QueryFunc qf){
      NReadGuard guard(mutex_);
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        nvar row;
        bool success = get(rowId, row);
        assert(success);
        return qf(row);
      };

      RowId m;
      max(m);
      query_("__data", m, f);
    }
    
    void join(const nstr& indexName, const RowSet& js, RowSet& rs){
      NReadGuard guard(mutex_);
      
      for(RowId findRowId : js){
        auto f = [&](RowId rowId, const nvar& v) -> int{
          RowId toRowId = v;
          
          if(toRowId != findRowId){
            return 0;
          }
          
          if(exists_(rowId)){
            rs.insert(rowId);
          }
          
          return 1;
        };
        
        query_(indexName, findRowId, f);
      }
    }
    
    void get(const RowSet& rs, NTable::QueryFunc qf){
      NReadGuard guard(mutex_);
      
      for(RowId findRowId : rs){
        auto f = [&](RowId rowId, const nvar& v) -> int{
          if(v.some()){
            RowId toRowId = v;
            
            if(toRowId != findRowId){
              return 0;
            }
            
            nvar row;
            bool success = get(rowId, row);
            assert(success);
            qf(row);
          }
          
          return 0;
        };
        
        query_("__data", findRowId, f);
      }
    }
    
    bool getFirst(const nstr& indexName, const nvar& value, nvar& row){
      NReadGuard guard(mutex_);
      
      bool success = false;
      
      auto f = [&](RowId rowId, const nvar& v) -> int{
        if(v != value){
          return 0;
        }
        
        nvar row;
        success = get(rowId, row);
        assert(success);
        return 0;
      };
      
      query_(indexName, value, f);
      
      return success;
    }
    
    void dump(){
      cout << "+++++++++ DUMP TABLE: " << path_ << endl;
      
      for(auto& itr : indexMap_){
        cout << "---- DUMP INDEX: " << itr.first << endl;
        itr.second->dump();
      }
      
      dataIndex_->dump();
      
      for(auto& itr : dataMap_){
        itr.second->dump();
      }
    }
    
    NDatabase_* database(){
      return d_;
    }
    
  private:
    typedef NMap<nstr, IndexBase*> IndexMap_;
    typedef NMap<uint64_t, Data*> DataMap_;
    
    NTable* o_;
    NDatabase_* d_;
    IndexMap_ indexMap_;
    uint32_t nextDataId_;
    DataMap_ dataMap_;
    Data* lastData_;
    DataIndex* dataIndex_;
    nstr path_;
    nstr dataPath_;
    nstr dataMetaPath_;
    nstr metaPath_;
    size_t memoryUsage_;
    NRWMutex mutex_;
  }; // end class NTable_
  
  NDatabase_::NDatabase_(NDatabase* o, const nstr& path, bool create)
  : o_(o),
  path_(path),
  metaPath_(path + "/meta.nvar"),
  tick_(0){
    
    if(create){
      if(NSys::exists(path_)){
        NERROR("path exists: " + path_);
      }
      
      NSys::makeDir(path);
      
      nextRowId_ = 1;
      memoryLimit_ = DEFAULT_MEMORY_LIMIT;
    }
    else{
      nvar m;
      m.open(metaPath_);
      nget(m, memoryLimit_);
      nget(m, nextRowId_);
      
      nvec files;
      if(!NSys::dirFiles(path_, files)){
        NERROR("failed to read database[1]: " + path_);
      }
      
      for(const nstr& p : files){
        if(p.endsWith(".table")){
          nstr tableName = NSys::fileName(p);
          nstr fullPath = path_ + "/" + p;
          
          NTable* table = new NTable(this, fullPath, false);
          
          tableMap_.insert({tableName, table->x_});
        }
      }
    }
  }
  
  void NDatabase_::saveMeta(){
    nvar m = nhmap();
    
    nput(m, memoryLimit_);
    nput(m, nextRowId_);
    
    m.save(metaPath_);
    
    for(auto& itr : tableMap_){
      itr.second->saveMeta();
    }
  }
  
  NTable* NDatabase_::addTable(const nstr& tableName){
    auto itr = tableMap_.find(tableName);
    if(itr != tableMap_.end()){
      NERROR("table exists: " + tableName);
    }
    
    nstr path = path_ + "/" + tableName + ".table";
    
    NTable* table = new NTable(this, path, true);
    NTable_* t = table->x_;
    
    tableMap_.insert({tableName, t});
    
    return table;
  }
  
  NTable* NDatabase_::getTable(const nstr& tableName){
    auto itr = tableMap_.find(tableName);
    if(itr == tableMap_.end()){
      NERROR("invalid table: " + tableName);
    }
    
    return itr->second->outer();
  }
  
  void NDatabase_::compact(){
    RowSet rs;
    UpdateMap um;

    for(auto& itr : tableMap_){
      NTable_* t = itr.second;
      t->writeLock_();
    }
    
    for(auto& itr : tableMap_){
      NTable_* t = itr.second;
      t->mapCompact(rs, um);
    }
    
    for(auto& itr : tableMap_){
      NTable_* t = itr.second;
      t->compact(rs, um);
    }
    
    for(auto& itr : tableMap_){
      NTable_* t = itr.second;
      t->unlock_();
    }
    
    saveMeta();
  }

  void NDatabase_::rollback(){
    for(auto& itr : tableMap_){
      NTable_* t = itr.second;
      t->rollback();
    }
  }
  
  size_t NDatabase_::memoryUsage(PMap& pm){
    size_t m = 0;
    for(auto& itr : tableMap_){
      m += itr.second->memoryUsage(pm);
    }
    
    return m;
  }
  
  void NDatabase_::checkMemory(){
    // ndm - test
    return;
    
    PMap pm;
    int64_t m = memoryUsage(pm);
    
    while(m > memoryLimit_){
      auto itr = pm.begin();
      if(itr == pm.end()){
        break;
      }
    
      Pageable* p = itr->second.first;
      p->store();
      
      size_t mi = itr->second.second;
      m -= mi;
    }
  }
  
  void NDatabase_::save(){
    saveMeta();
    
    for(auto& itr : tableMap_){
      itr.second->save();
    }
    
    for(auto& itr : tableMap_){
      itr.second->clean();
    }
  }
  
} // end namespace neu

NTable::NTable(NDatabase_* d, const nstr& path, bool create){
  x_ = new NTable_(this, d, path, create);
}

void NTable::addIndex(const nstr& indexName,
                      IndexType indexType,
                      bool unique){
  x_->addIndex(indexName, indexType, unique, false);
}

void NTable::addRowIndex(const nstr& indexName,
                         bool unique,
                         bool autoErase){
  x_->addIndex(indexName, Row, unique, autoErase);
}

uint64_t NTable::insert(nvar& row){
  return x_->insert(row);
}

void NTable::update(nvar& row){
  x_->update(row);
}

bool NTable::get(RowId rowId, nvar& row){
  return x_->get(rowId, row);
}

void NTable::erase(RowId rowId){
  x_->erase(rowId);
}

void NTable::query(const nstr& indexName,
                   const nvar& start,
                   QueryFunc f){
  x_->query(indexName, start, f);
}

void NTable::indexQuery(const nstr& indexName,
                        const nvar& start,
                        const nvar& end,
                        RowSet& rs){
  x_->indexQuery(indexName, start, end, rs);
}

void NTable::traverseStart(QueryFunc f){
  x_->traverseStart(f);
}

void NTable::traverseEnd(QueryFunc f){
  x_->traverseEnd(f);
}

void NTable::join(const nstr& indexName, const RowSet& js, RowSet& rs){
  x_->join(indexName, js, rs);
}

void NTable::get(const RowSet& rs, QueryFunc f){
  x_->get(rs, f);
}

bool NTable::getFirst(const nstr& indexName, const nvar& value, nvar& row){
  return x_->getFirst(indexName, value, row);
}

void NTable::dump(){
  x_->dump();
}

void NTable::save(){
  x_->save();
}

NDatabase::NDatabase(){}

NDatabase::NDatabase(const nstr& path){
  x_ = new NDatabase_(this, path, false);
}

NTable* NDatabase::addTable(const nstr& tableName){
  return x_->addTable(tableName);
}

NTable* NDatabase::getTable(const nstr& tableName){
  return x_->getTable(tableName);
}

NDatabase* NDatabase::create(const nstr& path){
  NDatabase* db = new NDatabase;
  db->x_ = new NDatabase_(db, path, true);
  
  return db;
}

void NDatabase::compact(){
  x_->compact();
}

void NDatabase::save(){
  x_->save();
}

void NDatabase::setMemoryLimit(size_t megabytes){
  x_->setMemoryLimit(megabytes);
}

void NDatabase::rollback(){
  x_->rollback();
}
