/*
 * A Memory Simulator Similar to X86 Virtual Memory
 * Created by He, Hao at 2019-3-11
 */

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstdint>
#include <cstdio>

#include <elfio/elfio.hpp>

#include "Cache.h"

class Cache;

class MemoryManager
{
public:
  MemoryManager();
  ~MemoryManager();

  struct Directoryblock {  // directory
      int      MESI;
      uint32_t blockaddr;  // offset位可以不要,blockaddr实际上就是vector的序号，不用显式存储
      Cache*   owner;
      int      sharer;  // 0代表0share，1代表1share，2代表真的share了，-1代表无core share
      Directoryblock() {}
      Directoryblock(const Directoryblock& b) {
          blockaddr = b.blockaddr;
          MESI      = b.MESI;
          owner     = b.owner;
          sharer    = b.sharer;
      }
  };

  std::pair<int, std::vector<uint8_t>> MESIoperationmem(int operationcode, uint32_t addr, Cache* issuedcache);

  bool addPage(uint32_t addr);
  bool isPageExist(uint32_t addr);

  bool copyFrom(const void *src, uint32_t dest, uint32_t len);

  bool    setByte(uint32_t addr, uint8_t val, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);
  bool setByteNoCache(uint32_t addr, uint8_t val);
  uint8_t getByte(uint32_t addr, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);
  uint8_t getByteNoCache(uint32_t addr);

  bool     setShort(uint32_t addr, uint16_t val, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);
  uint16_t getShort(uint32_t addr, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);

  bool     setInt(uint32_t addr, uint32_t val, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);
  uint32_t getInt(uint32_t addr, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);

  bool     setLong(uint32_t addr, uint64_t val, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);
  uint64_t getLong(uint32_t addr, int corenumber, uint64_t pc = -1, uint32_t* cycles = nullptr);

  void printInfo();
  void printStatistics();

  std::string dumpMemory();

  void setCache(Cache* cache, Cache* core1cache);

  private:
  uint32_t getFirstEntryId(uint32_t addr);
  uint32_t getSecondEntryId(uint32_t addr);
  uint32_t getPageOffset(uint32_t addr);
  bool isAddrExist(uint32_t addr);
  void     initDirectory();

  uint8_t **memory[1024];
  Cache *cache;
  Cache*    core1cache;
  std::vector<Directoryblock> directoryblocks;
};

#endif