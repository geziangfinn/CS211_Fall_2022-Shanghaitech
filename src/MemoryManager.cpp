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
    initDirectory();
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
        dbgprintf("Byte re0ad to invalid addr 0x%x!\n", addr);
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
      dbgprintf("Byte re1ad to invalid addr 0x%x!\n", addr);
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

void MemoryManager::initDirectory() {
    int directoryblocknumber = 0xFFFFFFC;  // cache size为64B,offset:6bits
    this->directoryblocks    = std::vector<Directoryblock>(directoryblocknumber);
    for (uint32_t i = 0; i < this->directoryblocks.size(); ++i) {
        Directoryblock& b = this->directoryblocks[i];
        b.MESI            = INVALID;
        b.blockaddr       = i;
        b.owner           = nullptr;
        b.sharer          = -1;
    }
}

std::pair<int, std::vector<uint8_t>> MemoryManager::MESIoperationmem(int operationcode, uint32_t addr, Cache* issuedcache) {
    uint32_t                             blockaddr = addr >> 6;
    std::pair<int, std::vector<uint8_t>> returnPair;

    int original     = directoryblocks[blockaddr].MESI;
    int final        = original;
    returnPair.first = original;

    //? ownread/write M/E时检查requestor是不是invalid?requestor是不是owner？
    //! issued cache 一定是L1 Cache，L1 Cache当作controller用
    Cache* sharer = core1cache;
    if (issuedcache == core1cache)
        sharer = cache;

    // std::cout << "core " << issuedcache->corenumber << " request " << std::hex << addr << "(block: " << int(blockaddr) << ") : ";

    switch (operationcode) {
    case OWNREAD:
        switch (original) {
        case MODIFIED:  // M->S
            // std::cout << "M->S\n";
            final = SHARED;
            // requestor一定不是owner，因为ownread M->M是个silent操作, requestor一定是INVALID
            if (directoryblocks[blockaddr].owner == issuedcache)
                std::cout << "error\n";
            returnPair.second = directoryblocks[blockaddr].owner->dataTrans(addr).data;
            directoryblocks[blockaddr].owner->MESIoperationRec(OTHERREAD, addr);
            //就俩cache，owner一定不是requestor
            directoryblocks[blockaddr].sharer = 2;  //两个sharer
            break;
        case EXCLUSIVE:  // E->S
            // std::cout << "E->S\n";
            final = SHARED;
            // requestor一定不是owner，因为ownread E->E是个silent操作, requestor一定是INVALID
            returnPair.second = directoryblocks[blockaddr].owner->dataTrans(addr).data;
            if (directoryblocks[blockaddr].owner == issuedcache)
                std::cout << "error2\n";
            directoryblocks[blockaddr].owner->MESIoperationRec(OTHERREAD, addr);
            //就俩cache，owner一定不是requestor
            directoryblocks[blockaddr].sharer = 2;
            break;
        case SHARED:
            // std::cout << "S->S\n";                       // S->S，requestor一定是INVALID，因为ownread S->S是silent操作
            final                             = SHARED;  // sharer同时也是owner，不用给数据
            directoryblocks[blockaddr].sharer = 2;       //两个sharer
            break;
        case INVALID:  // I->E
            // std::cout << "I->E\n";
            final                            = EXCLUSIVE;
            directoryblocks[blockaddr].owner = issuedcache;
            break;
        default:
            std::cout << "umcinmem\n";
            break;
        }
        break;
    case OWNWRITE:
        switch (original) {
        case MODIFIED:  // M->M
            // std::cout << "M->M\n";
            final = MODIFIED;
            // requestor一定不是owner，因为ownread M->M是个silent操作, requestor一定是INVALID
            returnPair.second = directoryblocks[blockaddr].owner->dataTrans(addr).data;  // 114514
            directoryblocks[blockaddr].owner->MESIoperationRec(OTHERWRITE, addr);        // invalidate
            directoryblocks[blockaddr].owner = issuedcache;                              //改变owner
            break;
        case EXCLUSIVE:  // E->M
            // std::cout << "E->M\n";
            final = MODIFIED;
            // requestor一定不是owner，因为ownread E->M是个silent操作, requestor一定是INVALID
            directoryblocks[blockaddr].owner->MESIoperationRec(OTHERWRITE, addr);  // invalidate
            directoryblocks[blockaddr].owner = issuedcache;
            break;
        case SHARED:  // S->M
            // std::cout << "S->M\n";
            final = MODIFIED;
            // requestor可能是sharer，S-S S-I I-S 三种情况，都发invalidate就完事了？
            sharer->MESIoperationRec(OTHERWRITE, addr);
            directoryblocks[blockaddr].owner  = issuedcache;
            directoryblocks[blockaddr].sharer = -1;
            break;
        case INVALID:  // I->M
            // std::cout << "I->M\n";
            final                            = MODIFIED;
            directoryblocks[blockaddr].owner = issuedcache;
            break;
        default:
            std::cout << "umcinmem\n";
            break;
        }
        break;
    case EVICT:  //! evict时dirctory里也是INVALID怎么处理？
                 //只有block是从M/E/S到I时才会给mem发消息，requestor一定是owner/sharer
        switch (original) {
        case MODIFIED:
            // std::cout << "M->I\n";
            final                             = INVALID;
            directoryblocks[blockaddr].owner  = nullptr;  //! potential bug！
            directoryblocks[blockaddr].sharer = -1;
            //被owner evict，改invalid，数据的写回放在cache的mesi中
            // L2中被evcit时本来就要写回，这样会写回两次？
            break;
        case EXCLUSIVE:
            // std::cout << "M->I\n";
            final                             = INVALID;
            directoryblocks[blockaddr].owner  = nullptr;  //! potential bug！
            directoryblocks[blockaddr].sharer = -1;
            break;
        case SHARED:  //?为什么会只有一个sharer? 一种可能是有一个sharer被evict了，这是唯一的可能吗
            // requestor 可能不是唯一sharer

            if (directoryblocks[blockaddr].sharer != 2) {  //!=2说明是唯一sharer
                final                             = INVALID;
                directoryblocks[blockaddr].sharer = -1;
                // std::cout << "S->I\n";
            }
            else {
                if (issuedcache == core1cache)
                    directoryblocks[blockaddr].sharer = 0;
                else
                    directoryblocks[blockaddr].sharer = 1;
                final = SHARED;  //还是SHARED
                // std::cout << "S->S\n";
            }
            directoryblocks[blockaddr].owner = nullptr;  //! potential bug！
            break;
        case INVALID:
            //无操作
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    directoryblocks[blockaddr].MESI = final;  //修改directory中状态

    return returnPair;  //返回数据就行，不必返回block?
}
