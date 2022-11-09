/*
 * Created by He, Hao at 2019-3-11
 */

#include "MemoryManager.h"
#include "Debug.h"

#include <cstdio>
#include <string>
#define DUALCORE
MemoryManager::MemoryManager() {
    this->cache      = nullptr;
    this->core1cache = nullptr;
    for (uint32_t i = 0; i < 1024; ++i) {
        this->memory[i] = nullptr;
    }
}

MemoryManager::~MemoryManager() {
  for (uint32_t i = 0; i < 1024; ++i) {
    if (this->memory[i] != nullptr) {
      for (uint32_t j = 0; j < 1024; ++j) {
        if (this->memory[i][j] != nullptr) {
          delete[] this->memory[i][j];
          this->memory[i][j] = nullptr;
        }
      }
      delete[] this->memory[i];
      this->memory[i] = nullptr;
    }
  }
}

bool MemoryManager::addPage(uint32_t addr) {
  uint32_t i = this->getFirstEntryId(addr);
  uint32_t j = this->getSecondEntryId(addr);
  if (this->memory[i] == nullptr) {
    this->memory[i] = new uint8_t *[1024];
    memset(this->memory[i], 0, sizeof(uint8_t *) * 1024);
  }
  if (this->memory[i][j] == nullptr) {
    this->memory[i][j] = new uint8_t[4096];
    memset(this->memory[i][j], 0, 4096);
  } else {
    dbgprintf("Addr 0x%x already exists and do not need an addPage()!\n", addr);
    return false;
  }
  return true;
}

bool MemoryManager::isPageExist(uint32_t addr) {
  return this->isAddrExist(addr);
}

bool MemoryManager::copyFrom(const void *src, uint32_t dest, uint32_t len) {
  for (uint32_t i = 0; i < len; ++i) {
    if (!this->isAddrExist(dest + i)) {
      dbgprintf("Data copy to invalid addr 0x%x!\n", dest + i);
      return false;
    }
    this->setByteNoCache(dest + i, (( uint8_t* )src)[i]);  // SetByteNO119
  }
  return true;
}

bool MemoryManager::setByte(uint32_t addr, uint8_t val, int corenumber, uint64_t pc, uint32_t* cycles) {
    if (!this->isAddrExist(addr)) {
        dbgprintf("Byte write to invalid addr 0x%x!\n", addr);
        return false;
    }
#ifdef DUALCORE
    if (corenumber == 0) {
        if (this->cache != nullptr) {
            this->cache->setByte(addr, val, pc, cycles);
            return true;
        }
    }
    else if (corenumber == 1) {
        if (this->core1cache != nullptr) {
            this->core1cache->setByte(addr, val, pc, cycles);
            return true;
        }
    }
    else {
        printf("wrong core numberW\n");
        exit(0);
    }
#endif
    if (this->cache != nullptr) {
        this->cache->setByte(addr, val, pc, cycles);
        return true;
    }

    uint32_t i            = this->getFirstEntryId(addr);
    uint32_t j            = this->getSecondEntryId(addr);
    uint32_t k            = this->getPageOffset(addr);
    this->memory[i][j][k] = val;
    return true;
}

bool MemoryManager::setByteNoCache(uint32_t addr, uint8_t val) {
  if (!this->isAddrExist(addr)) {
    dbgprintf("Byte write to invalid addr 0x%x!\n", addr);
    return false;
  }

  uint32_t i = this->getFirstEntryId(addr);
  uint32_t j = this->getSecondEntryId(addr);
  uint32_t k = this->getPageOffset(addr);
  this->memory[i][j][k] = val;
  return true;
}

uint8_t MemoryManager::getByte(uint32_t addr, int corenumber, uint64_t pc, uint32_t* cycles) {
    if (!this->isAddrExist(addr)) {
        dbgprintf("Byte read to invalid addr 0x%x!\n", addr);
        return false;
    }
#ifdef DUALCORE
    if (corenumber == 0) {
        if (this->cache != nullptr) {
            return this->cache->getByte(addr, pc, cycles);
        }
    }
    else if (corenumber == 1) {
        if (this->core1cache != nullptr) {
            return this->core1cache->getByte(addr, pc, cycles);
        }
    }
    else {
        printf("wrong core numberR\n");
        exit(0);
    }
#endif

    if (this->cache != nullptr) {
        return this->cache->getByte(addr, pc, cycles);
    }
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    uint32_t k = this->getPageOffset(addr);
    return this->memory[i][j][k];
}

uint8_t MemoryManager::getByteNoCache(uint32_t addr) {
  if (!this->isAddrExist(addr)) {
    dbgprintf("Byte read to invalid addr 0x%x!\n", addr);
    return false;
  }
  uint32_t i = this->getFirstEntryId(addr);
  uint32_t j = this->getSecondEntryId(addr);
  uint32_t k = this->getPageOffset(addr);
  return this->memory[i][j][k];
}

bool MemoryManager::setShort(uint32_t addr, uint16_t val, int corenumber, uint64_t pc, uint32_t* cycles) {
    if (!this->isAddrExist(addr)) {
        dbgprintf("Short write to invalid addr 0x%x!\n", addr);
        return false;
    }
    this->setByte(addr, val & 0xFF, corenumber, pc, cycles);
    this->setByte(addr + 1, (val >> 8) & 0xFF, corenumber);
    return true;
}

uint16_t MemoryManager::getShort(uint32_t addr, int corenumber, uint64_t pc, uint32_t* cycles) {
    uint32_t b1 = this->getByte(addr, corenumber, pc, cycles);
    uint32_t b2 = this->getByte(addr + 1, corenumber);
    return b1 + (b2 << 8);
}

bool MemoryManager::setInt(uint32_t addr, uint32_t val, int corenumber, uint64_t pc, uint32_t* cycles) {
    if (!this->isAddrExist(addr)) {
        dbgprintf("Int write to invalid addr 0x%x!\n", addr);
        return false;
    }
    this->setByte(addr, val & 0xFF, corenumber, pc, cycles);
    this->setByte(addr + 1, (val >> 8) & 0xFF, corenumber);
    this->setByte(addr + 2, (val >> 16) & 0xFF, corenumber);
    this->setByte(addr + 3, (val >> 24) & 0xFF, corenumber);
    return true;
}

uint32_t MemoryManager::getInt(uint32_t addr, int corenumber, uint64_t pc, uint32_t* cycles) {
    uint32_t b1 = this->getByte(addr, corenumber, pc, cycles);
    uint32_t b2 = this->getByte(addr + 1, corenumber);
    uint32_t b3 = this->getByte(addr + 2, corenumber);
    uint32_t b4 = this->getByte(addr + 3, corenumber);
    return b1 + (b2 << 8) + (b3 << 16) + (b4 << 24);
}

bool MemoryManager::setLong(uint32_t addr, uint64_t val, int corenumber, uint64_t pc, uint32_t* cycles) {
    if (!this->isAddrExist(addr)) {
        dbgprintf("Long write to invalid addr 0x%x!\n", addr);
        return false;
    }
    this->setByte(addr, val & 0xFF, corenumber, pc, cycles);
    this->setByte(addr + 1, (val >> 8) & 0xFF, corenumber);
    this->setByte(addr + 2, (val >> 16) & 0xFF, corenumber);
    this->setByte(addr + 3, (val >> 24) & 0xFF, corenumber);
    this->setByte(addr + 4, (val >> 32) & 0xFF, corenumber);
    this->setByte(addr + 5, (val >> 40) & 0xFF, corenumber);
    this->setByte(addr + 6, (val >> 48) & 0xFF, corenumber);
    this->setByte(addr + 7, (val >> 56) & 0xFF, corenumber);
    return true;
}

uint64_t MemoryManager::getLong(uint32_t addr, int corenumber, uint64_t pc, uint32_t* cycles) {
    uint64_t b1 = this->getByte(addr, corenumber, pc, cycles);
    uint64_t b2 = this->getByte(addr + 1, corenumber);
    uint64_t b3 = this->getByte(addr + 2, corenumber);
    uint64_t b4 = this->getByte(addr + 3, corenumber);
    uint64_t b5 = this->getByte(addr + 4, corenumber);
    uint64_t b6 = this->getByte(addr + 5, corenumber);
    uint64_t b7 = this->getByte(addr + 6, corenumber);
    uint64_t b8 = this->getByte(addr + 7, corenumber);
    return b1 + (b2 << 8) + (b3 << 16) + (b4 << 24) + (b5 << 32) + (b6 << 40) + (b7 << 48) + (b8 << 56);
}

void MemoryManager::printInfo() {
  printf("Memory Pages: \n");
  for (uint32_t i = 0; i < 1024; ++i) {
    if (this->memory[i] == nullptr) {
      continue;
    }
    printf("0x%x-0x%x:\n", i << 22, (i + 1) << 22);
    for (uint32_t j = 0; j < 1024; ++j) {
      if (this->memory[i][j] == nullptr) {
        continue;
      }
      printf("  0x%x-0x%x\n", (i << 22) + (j << 12),
             (i << 22) + ((j + 1) << 12));
    }
  }
}

void MemoryManager::printStatistics() {
  printf("---------- CACHE STATISTICS ----------\n");
  this->cache->printStatistics();
}

std::string MemoryManager::dumpMemory() {
  char buf[65536];
  std::string dump;

  dump += "Memory Pages: \n";
  for (uint32_t i = 0; i < 1024; ++i) {
    if (this->memory[i] == nullptr) {
      continue;
    }
    sprintf(buf, "0x%x-0x%x:\n", i << 22, (i + 1) << 22);
    dump += buf;
    for (uint32_t j = 0; j < 1024; ++j) {
      if (this->memory[i][j] == nullptr) {
        continue;
      }
      sprintf(buf, "  0x%x-0x%x\n", (i << 22) + (j << 12),
              (i << 22) + ((j + 1) << 12));
      dump += buf;

      for (uint32_t k = 0; k < 1024; ++k) {
        sprintf(buf, "    0x%x: 0x%x\n", (i << 22) + (j << 12) + k,
                this->memory[i][j][k]);
        dump += buf;
      }
    }
  }
  return dump;
}

uint32_t MemoryManager::getFirstEntryId(uint32_t addr) {
  return (addr >> 22) & 0x3FF;
}

uint32_t MemoryManager::getSecondEntryId(uint32_t addr) {
  return (addr >> 12) & 0x3FF;
}

uint32_t MemoryManager::getPageOffset(uint32_t addr) { return addr & 0xFFF; }

bool MemoryManager::isAddrExist(uint32_t addr) {
  uint32_t i = this->getFirstEntryId(addr);
  uint32_t j = this->getSecondEntryId(addr);
  if (memory[i] && memory[i][j]) {
    return true;
  }
  return false;
}

void MemoryManager::setCache(Cache* cache, Cache* core1cache) {
    this->cache      = cache;
    this->core1cache = core1cache;
}
