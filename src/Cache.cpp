/*
 * Implementation of a simple cache simulator
 *
 * Created By He, Hao in 2019-04-27
 */

#include <cstdio>
#include <cstdlib>

#include "Cache.h"

#define CACHETEST

Cache::Cache(MemoryManager* manager, Policy policy, int cachelevel, Cache* lowerCache, bool writeBack, bool writeAllocate) {
    this->referenceCounter = 0;
    this->memory           = manager;
    this->policy           = policy;
    this->lowerCache       = lowerCache;
    if (!this->isPolicyValid()) {
        fprintf(stderr, "Policy invalid!\n");
        exit(-1);
    }
    this->initCache();
    this->statistics.numRead     = 0;
    this->statistics.numWrite    = 0;
    this->statistics.numHit      = 0;
    this->statistics.numMiss     = 0;
    this->statistics.totalCycles = 0;
    this->writeBack              = writeBack;
    this->writeAllocate          = writeAllocate;
    this->cachelevel             = cachelevel;
}

bool Cache::inCache(uint32_t addr) {
    return getBlockId(addr) != -1 ? true : false;
}

uint32_t Cache::getBlockId(uint32_t addr) {
    uint32_t tag = this->getTag(addr);
    uint32_t id  = this->getId(addr);
    // printf("0x%x 0x%x 0x%x\n", addr, tag, id);
    // iterate over the given set
    for (uint32_t i = id * policy.associativity; i < (id + 1) * policy.associativity; ++i) {
        if (this->blocks[i].id != id) {
            fprintf(stderr, "Inconsistent ID in block %d\n", i);
            exit(-1);
        }
        if (this->blocks[i].valid && this->blocks[i].tag == tag) {  // valid!
            return i;
        }
    }
    return -1;
}

uint8_t Cache::getByte(uint32_t addr, uint32_t* cycles) {

    return this->getByteEx(addr, cycles);

    this->referenceCounter++;
    this->statistics.numRead++;

    // If in cache, return directly
    int blockId;  //在L2 L3中命中时，要invalidate block
    if ((blockId = this->getBlockId(addr)) != -1) {
        uint32_t offset = this->getOffset(addr);
        this->statistics.numHit++;
        this->statistics.totalCycles += this->policy.hitLatency;
        this->blocks[blockId].lastReference = this->referenceCounter;
        if (cycles)
            *cycles = this->policy.hitLatency;
        return this->blocks[blockId].data[offset];
    }

    // Else, find the data in memory or other level of cache
    this->statistics.numMiss++;
    this->statistics.totalCycles += this->policy.missLatency;
    this->loadBlockFromLowerLevel(addr, cycles);

    // The block is in top level cache now, return directly
    if ((blockId = this->getBlockId(addr)) != -1) {
        uint32_t offset                     = this->getOffset(addr);
        this->blocks[blockId].lastReference = this->referenceCounter;
        return this->blocks[blockId].data[offset];
    }
    else {
        fprintf(stderr, "Error: data not in top level cache!\n");
        exit(-1);
    }
}

void Cache::setByte(uint32_t addr, uint8_t val, uint32_t* cycles) {

    setByteEx(addr, val, cycles);
    return;
    this->referenceCounter++;
    this->statistics.numWrite++;

    // If in cache, write to it directly
    int blockId;
    if ((blockId = this->getBlockId(addr)) != -1) {
        uint32_t offset = this->getOffset(addr);
        this->statistics.numHit++;
        this->statistics.totalCycles += this->policy.hitLatency;
        this->blocks[blockId].modified      = true;
        this->blocks[blockId].lastReference = this->referenceCounter;
        this->blocks[blockId].data[offset]  = val;
        if (!this->writeBack) {
            this->writeBlockToLowerLevel(this->blocks[blockId]);  //对于exclusive cache，write through应该只写到L1和memory:用setbytenocache替换writetolowerlevel
            this->statistics.totalCycles += this->policy.missLatency;
        }
        if (cycles)
            *cycles = this->policy.hitLatency;
        return;
    }
    //对于通过replace装填lower level,写一定不命中，如果用writetolowerlevel函数，会走到这里，miss不应++，只需要写就行了，modified也不应为true 单独增添一个函数处理replace load的情况？
    // Else, load the data from cache
    // TODO: implement bypassing
    this->statistics.numMiss++;
    this->statistics.totalCycles += this->policy.missLatency;

    if (this->writeAllocate) {                        // writeAllocate lab1 only write allocate
        this->loadBlockFromLowerLevel(addr, cycles);  // writeallocate写不命中时只load到L1

        if ((blockId = this->getBlockId(addr)) != -1) {  //然后写L1
            uint32_t offset                     = this->getOffset(addr);
            this->blocks[blockId].modified      = true;
            this->blocks[blockId].lastReference = this->referenceCounter;
            this->blocks[blockId].data[offset]  = val;
            return;
        }
        else {
            fprintf(stderr, "Error: data not in top level cache!\n");
            exit(-1);
        }
    }
    else {  // lab1中不考虑这种情况
        if (this->lowerCache == nullptr) {
            this->memory->setByteNoCache(addr, val);
        }
        else {
            this->lowerCache->setByte(addr, val);
        }
    }
}

void Cache::printInfo(bool verbose) {
    printf("---------- Cache Info -----------\n");
    printf("Cache Size: %d bytes\n", this->policy.cacheSize);
    printf("Block Size: %d bytes\n", this->policy.blockSize);
    printf("Block Num: %d\n", this->policy.blockNum);
    printf("Associativiy: %d\n", this->policy.associativity);
    printf("Hit Latency: %d\n", this->policy.hitLatency);
    printf("Miss Latency: %d\n", this->policy.missLatency);

    if (verbose) {
        for (int j = 0; j < this->blocks.size(); ++j) {
            const Block& b = this->blocks[j];
            printf("Block %d: tag 0x%x id %d %s %s (last ref %d)\n", j, b.tag, b.id, b.valid ? "valid" : "invalid", b.modified ? "modified" : "unmodified", b.lastReference);
            // printf("Data: ");
            // for (uint8_t d : b.data)
            // printf("%d ", d);
            // printf("\n");
        }
    }
}

void Cache::printStatistics() {
    printf("-------- STATISTICS ----------\n");
    printf("Num Read: %d\n", this->statistics.numRead);
    printf("Num Write: %d\n", this->statistics.numWrite);
    printf("Num Hit: %d\n", this->statistics.numHit);
    printf("Num Miss: %d\n", this->statistics.numMiss);
    printf("Total Cycles: %llu\n", this->statistics.totalCycles);
    if (this->lowerCache != nullptr) {
        printf("---------- LOWER CACHE ----------\n");
        this->lowerCache->printStatistics();
    }
}

bool Cache::isPolicyValid() {
    if (!this->isPowerOfTwo(policy.cacheSize)) {
        fprintf(stderr, "Invalid Cache Size %d\n", policy.cacheSize);
        return false;
    }
    if (!this->isPowerOfTwo(policy.blockSize)) {
        fprintf(stderr, "Invalid Block Size %d\n", policy.blockSize);
        return false;
    }
    if (policy.cacheSize % policy.blockSize != 0) {
        fprintf(stderr, "cacheSize %% blockSize != 0\n");
        return false;
    }
    if (policy.blockNum * policy.blockSize != policy.cacheSize) {
        fprintf(stderr, "blockNum * blockSize != cacheSize\n");
        return false;
    }
    if (policy.blockNum % policy.associativity != 0) {
        fprintf(stderr, "blockNum %% associativity != 0\n");
        return false;
    }
    return true;
}

void Cache::initCache() {
    this->blocks = std::vector< Block >(policy.blockNum);
    for (uint32_t i = 0; i < this->blocks.size(); ++i) {
        Block& b        = this->blocks[i];
        b.valid         = false;
        b.modified      = false;
        b.size          = policy.blockSize;
        b.tag           = 0;
        b.id            = i / policy.associativity;
        b.lastReference = 0;
        b.deadblock     = false;
        b.data          = std::vector< uint8_t >(b.size);
    }
}

void Cache::loadBlockFromLowerLevel(uint32_t addr, uint32_t* cycles) {
    uint32_t blockSize = this->policy.blockSize;

    // Initialize new block from memory
    Block b;
    b.valid                 = true;
    b.modified              = false;
    b.tag                   = this->getTag(addr);
    b.id                    = this->getId(addr);
    b.size                  = blockSize;
    b.data                  = std::vector< uint8_t >(b.size);
    uint32_t bits           = this->log2i(blockSize);
    uint32_t mask           = ~((1 << bits) - 1);
    uint32_t blockAddrBegin = addr & mask;
    for (uint32_t i = blockAddrBegin; i < blockAddrBegin + blockSize; ++i) {
        if (this->lowerCache == nullptr) {
            b.data[i - blockAddrBegin] = this->memory->getByteNoCache(i);
            if (cycles)
                *cycles = 100;  //访存的延迟
        }
        else
            b.data[i - blockAddrBegin] = this->lowerCache->getByte(i, cycles);  // exclusive cache: flush data from lower level cache when loading:set to invalid?
        // TODO: exclusive cache 现行的实现会从memory或lowerlevel加载到每一个higher level，evict时不会放到lower level
    }
    //这里replace包含valid和invalid两种情况，不是valid的那个替换，即需要写到下一级的那种。
    // Find replace block
    uint32_t id           = this->getId(addr);
    uint32_t blockIdBegin = id * this->policy.associativity;
    uint32_t blockIdEnd   = (id + 1) * this->policy.associativity;
    uint32_t replaceId    = this->getReplacementBlockId(blockIdBegin, blockIdEnd);  // exclusive cache: flush from higher level cache and put in lower level
    Block    replaceBlock =
        this->blocks[replaceId];  // exclusive cache：只要被替换（replaceblock.valid=true），不管是不是dirty，是不是writeback，都要写并且只写到下一级，L3的victim是写到内存(dirty writeback)或者扔掉
    if (this->writeBack && replaceBlock.valid && replaceBlock.modified) {  // write back to memory
        this->writeBlockToLowerLevel(replaceBlock);  //现在没有修改的Victim会被扔掉，应改为放且只放到下一级，还是单独加一个函数来处理好,这里就给Writeback用，新函数调用的也应该是新函数。
        // writeback时和通过replace来load时，L2，L3写必不命中，因为exclusive writeback+exclusive: 只有replace的时候才会写到lowerlevel，lowerlevel只通过replace装载
        this->statistics.totalCycles += this->policy.missLatency;  //再加一个misslatency
    }
    this->blocks[replaceId] = b;  // b.modified=false 本函数中应该只有L1有这一步，只有L1调用这个函数？？或只有L1的b.valid=true
    // L3存L2的victim，L2的victim只可能是被L1的victim挤出来，因为L2只存L1的victim。L2中何时发生替换（也是装载）：在L1中发生替换时。L1中何时发生替换：读：不命中并load时
    // 写：不命中并allocate时，两种情况都是通过loadfromlower来做替换命中时不存在替换 L3中何时发生替换（也是装载）：在L2中发生替换时，也即L1中发生替换时，也即那两种情况时
}  //可能需要写新的write block to lower level，其中要有新的替换逻辑

uint32_t Cache::getReplacementBlockId(uint32_t begin, uint32_t end) {
    // Find invalid block first
    for (uint32_t i = begin; i < end; ++i) {
        if (!this->blocks[i].valid)
            return i;  // vaild的才需要写到下一级cache，valid才发生了我所说的“替换”
    }

    // Otherwise use LRU
    uint32_t resultId = begin;
    uint32_t min      = this->blocks[begin].lastReference;
    for (uint32_t i = begin; i < end; ++i) {
        if (this->blocks[i].lastReference < min) {
            resultId = i;
            min      = this->blocks[i].lastReference;
        }
    }
    return resultId;
}

void Cache::writeBlockToLowerLevel(Cache::Block& b) {
    uint32_t addrBegin = this->getAddr(b);
    if (this->lowerCache == nullptr) {
        for (uint32_t i = 0; i < b.size; ++i) {
            this->memory->setByteNoCache(addrBegin + i, b.data[i]);
        }
    }
    else {
        for (uint32_t i = 0; i < b.size; ++i) {
            this->lowerCache->setByte(addrBegin + i, b.data[i]);  // exclusive cache中应只放到下一级，不应是现在这样递归的
        }
    }
}

bool Cache::isPowerOfTwo(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

uint32_t Cache::log2i(uint32_t val) {
    if (val == 0)
        return uint32_t(-1);
    if (val == 1)
        return 0;
    uint32_t ret = 0;
    while (val > 1) {
        val >>= 1;
        ret++;
    }
    return ret;
}

uint32_t Cache::getTag(uint32_t addr) {
    uint32_t offsetBits = log2i(policy.blockSize);
    uint32_t idBits     = log2i(policy.blockNum / policy.associativity);
    uint32_t mask       = (1 << (32 - offsetBits - idBits)) - 1;
    return (addr >> (offsetBits + idBits)) & mask;
}

uint32_t Cache::getId(uint32_t addr) {
    uint32_t offsetBits = log2i(policy.blockSize);
    uint32_t idBits     = log2i(policy.blockNum / policy.associativity);
    // std::cout << idBits << " level " << this->cachelevel<<"\n";
    uint32_t mask = (1 << idBits) - 1;
    return (addr >> offsetBits) & mask;
}

uint32_t Cache::getOffset(uint32_t addr) {
    uint32_t bits = log2i(policy.blockSize);
    uint32_t mask = (1 << bits) - 1;
    return addr & mask;
}

uint32_t Cache::getAddr(Cache::Block& b) {
    uint32_t offsetBits = log2i(policy.blockSize);
    uint32_t idBits     = log2i(policy.blockNum / policy.associativity);
    return (b.tag << (offsetBits + idBits)) | (b.id << offsetBits);
}

void Cache::setByteEx(uint32_t addr, uint8_t val, uint32_t* cycles) {
#ifdef CACHETEST
    printf("\nwriting addr: %x\n", addr);
#endif
    this->referenceCounter++;
    this->statistics.numWrite++;
    Cache* lowlevelpointer = this->lowerCache;

    Block replaceblock;

    // If in L1 cache, return directly
    int blockId;
    if ((blockId = this->getBlockId(addr)) != -1) {
#ifdef CACHETEST
        printf("addr: %x", addr);
        std::cout << " Hit in level " << this->cachelevel << "!\n";
#endif
        uint32_t offset = this->getOffset(addr);
        this->statistics.numHit++;
        this->statistics.totalCycles += this->policy.hitLatency;
        this->blocks[blockId].modified      = true;
        this->blocks[blockId].lastReference = this->referenceCounter;
        this->blocks[blockId].data[offset]  = val;

        if (!this->writeBack) {
#ifdef CACHETEST
            std::cout << "Writing through to memory.\n";
#endif
            uint32_t addrBegin = this->getAddr(blocks[blockId]);
            std::cout << addrBegin << "\n";
            for (uint32_t i = 0; i < blocks[blockId].size; ++i) {
                this->memory->setByteNoCache(addrBegin + i, blocks[blockId].data[i]);
            }                                                          //对于exclusive cache，write through应该只写到L1和memory:用setbytenocache替换writetolowerlevel
            this->statistics.totalCycles += this->policy.missLatency;  //?why
        }
        if (cycles)
            *cycles = this->policy.hitLatency;
        return;
    }

    // Else, find the data in memory or L2, L3
    this->statistics.numMiss++;
    this->statistics.totalCycles += this->policy.missLatency;

    if (this->writeAllocate) {  // writeAllocate lab1 only write allocate
        while (lowlevelpointer != nullptr) {
            replaceblock = this->loadBlockFromLowerLevelEx(addr, cycles, lowlevelpointer);  // L2,L3
            if (replaceblock.valid) {
                break;
            }
            else {
                lowlevelpointer = lowlevelpointer->lowerCache;
            }
        }
        if (!replaceblock.valid) {
            replaceblock = this->loadBlockFromLowerLevelEx(addr, cycles, nullptr);  // memory
        }

        uint32_t id                = this->getId(addr);  // replace
        uint32_t blockIdBegin      = id * this->policy.associativity;
        uint32_t blockIdEnd        = (id + 1) * this->policy.associativity;
        uint32_t replaceId         = this->getReplacementBlockId(blockIdBegin, blockIdEnd);
        Block    blocktobereplaced = this->blocks[replaceId];

        if (blocktobereplaced.valid) {                                  // write to next level
            this->writeBlockToLowerLevelEx(blocktobereplaced, cycles);  // writeB to be implemented
            this->statistics.totalCycles += this->policy.missLatency;
        }
        this->blocks[replaceId] = replaceblock;  // writeallocate写不命中时, 从L2或L3或memoryload到L1

        if ((blockId = this->getBlockId(addr)) != -1) {  //然后写L1
            uint32_t offset                     = this->getOffset(addr);
            this->blocks[blockId].modified      = true;
            this->blocks[blockId].lastReference = this->referenceCounter;
            this->blocks[blockId].data[offset]  = val;

            if (!this->writeBack) {  // write through
#ifdef CACHETEST
                std::cout << "Writing through to memory.\n";
#endif
                uint32_t addrBegin = this->getAddr(blocks[blockId]);
                std::cout << addrBegin << "\n";
                for (uint32_t i = 0; i < blocks[blockId].size; ++i) {
                    this->memory->setByteNoCache(addrBegin + i, blocks[blockId].data[i]);
                }                                                          //对于exclusive cache，write through应该只写到L1和memory: 用setbytenocache替换writetolowerlevel
                this->statistics.totalCycles += this->policy.missLatency;  //?why
            }

            return;
        }
        else {
            fprintf(stderr, "Error: data not in top level cache!\n");
            exit(-1);
        }
    }
    else {  // lab1中不考虑这种情况，因为都是writeallocate
        if (this->lowerCache == nullptr) {
            this->memory->setByteNoCache(addr, val);
        }
        else {
            this->lowerCache->setByte(addr, val);
        }
    }
}

uint8_t Cache::getByteEx(uint32_t addr, uint32_t* cycles) {
#ifdef CACHETEST
    printf("\nreading addr: %x\n", addr);
#endif
    this->referenceCounter++;
    this->statistics.numRead++;

    Cache* lowlevelpointer = this->lowerCache;

    Block replaceblock;

    // If in L1 cache, return directly
    int blockId;
    if ((blockId = this->getBlockId(addr)) != -1) {
#ifdef CACHETEST
        printf("addr: %x", addr);
        std::cout << " Hit in level " << this->cachelevel << "!\n";
#endif
        uint32_t offset = this->getOffset(addr);
        this->statistics.numHit++;
        this->statistics.totalCycles += this->policy.hitLatency;
        this->blocks[blockId].lastReference = this->referenceCounter;
        if (cycles)
            *cycles = this->policy.hitLatency;
        return this->blocks[blockId].data[offset];
    }

    // Else, find the data in memory or L2, L3
    this->statistics.numMiss++;
    this->statistics.totalCycles += this->policy.missLatency;
    while (lowlevelpointer != nullptr) {
        replaceblock = this->loadBlockFromLowerLevelEx(addr, cycles, lowlevelpointer);  // L2,L3
        if (replaceblock.valid) {
            break;
        }
        else {
            lowlevelpointer = lowlevelpointer->lowerCache;
        }
    }
    if (!replaceblock.valid) {
        replaceblock = this->loadBlockFromLowerLevelEx(addr, cycles, nullptr);  // memory
    }

    uint32_t id                = this->getId(addr);  // replace
    uint32_t blockIdBegin      = id * this->policy.associativity;
    uint32_t blockIdEnd        = (id + 1) * this->policy.associativity;
    uint32_t replaceId         = this->getReplacementBlockId(blockIdBegin, blockIdEnd);
    Block    blocktobereplaced = this->blocks[replaceId];
    if (blocktobereplaced.valid) {                                  // write back to memory
        this->writeBlockToLowerLevelEx(blocktobereplaced, cycles);  // writeB to be implemented
        this->statistics.totalCycles += this->policy.missLatency;
    }
    this->blocks[replaceId] = replaceblock;

    // The block is in top level cache now, return directly
    if ((blockId = this->getBlockId(addr)) != -1) {
        uint32_t offset                     = this->getOffset(addr);  //不应再用递归而应是迭代的思维做，因为L2和L3与L1性质不同了？
        this->blocks[blockId].lastReference = this->referenceCounter;
        return this->blocks[blockId].data[offset];
    }
    else {
        fprintf(stderr, "Error: data not in top level cache!\n");
        exit(-1);
    }
}

Cache::Block Cache::loadBlockFromLowerLevelEx(uint32_t addr, uint32_t* cycles, Cache* lower) {

    uint32_t blockSize = this->policy.blockSize;  //此处为L1的blocksize
    // uint32_t blockSize = 64;
    Block b;
    b.valid                 = false;
    b.modified              = false;
    b.tag                   = this->getTag(addr);
    b.id                    = this->getId(addr);
    b.size                  = blockSize;
    b.data                  = std::vector< uint8_t >(b.size);
    uint32_t bits           = this->log2i(blockSize);
    uint32_t mask           = ~((1 << bits) - 1);
    uint32_t blockAddrBegin = addr & mask;  // b: L1中的一个block

    if (lower == nullptr) {  // 从memory里向L1读
#ifdef CACHETEST
        printf("addr: %x", addr);
        std::cout << " Hit in memory " << this->cachelevel << "! Load to L1\n";
#endif
        for (uint32_t i = blockAddrBegin; i < blockAddrBegin + blockSize; ++i) {
            b.data[i - blockAddrBegin] = this->memory->getByteNoCache(i);
        }
        if (cycles)
            *cycles = 100;
        b.valid = true;
        return b;
    }
    // If in cache, return block for replacement 从L2/L3向L1读
    lower->referenceCounter++;
    lower->statistics.numRead++;
    int blockId;  //在L2/L3中寻找，hit时要invalidate block
#ifdef CACHETEST
    std::cout << "searching in" << lower->cachelevel << "!\n";
#endif
    if ((blockId = lower->getBlockId(addr)) != -1) {
        // uint32_t offset = lower->getOffset(addr);
#ifdef CACHETEST
        printf("addr: %x", addr);
        std::cout << " Hit in level " << lower->cachelevel << "! Load to L1\n";
#endif
        lower->statistics.numHit++;
        lower->statistics.totalCycles += lower->policy.hitLatency;
        lower->blocks[blockId].lastReference = lower->referenceCounter;  //没用，反正最后这个block要设为invalid

        if (cycles)
            *cycles = lower->policy.hitLatency;

        for (uint32_t i = blockAddrBegin; i < blockAddrBegin + blockSize; ++i) {
            b.data[i - blockAddrBegin] = lower->blocks[blockId].data[i - blockAddrBegin];  //!: 可能有bug！！
        }
        lower->blocks[blockId].valid = false;  //从命中的低级cache中清除
        b.valid                      = true;
        return b;
    }
    else {
        lower->statistics.numMiss++;
        lower->statistics.totalCycles += lower->policy.missLatency;
    }
    return b;
}

void Cache::writeBlockToLowerLevelEx(Cache::Block& b, uint32_t* cycles) {
    uint32_t addrBegin = this->getAddr(b);  // addrBegin只能本层用？？？不是addr？

    if (this->lowerCache == nullptr)  // this=L3
    {
        if (this->writeBack) {
            if (b.modified) {
                std::cout << "Modified. Write back to memory.\n";
                for (uint32_t i = 0; i < b.size; ++i) {
                    this->memory->setByteNoCache(addrBegin + i, b.data[i]);
                }
            }  // modified就写回内存(writeback)，没有就算了，注意cycles
        }
        return;
    }
    this->lowerCache->referenceCounter++;
    this->lowerCache->statistics.numWrite++;

    Block tolower;
    tolower.valid         = b.valid;
    tolower.modified      = b.modified;
    tolower.tag           = this->lowerCache->getTag(addrBegin);
    tolower.id            = this->lowerCache->getId(addrBegin);
    tolower.size          = this->lowerCache->Cache::policy.blockSize;
    tolower.data          = std::vector< uint8_t >(b.data);
    tolower.lastReference = this->lowerCache->referenceCounter;  //!: 再考虑一下
    /*uint32_t bits = this->log2i(tolower.size);
    uint32_t mask = ~((1 << bits) - 1);
    uint32_t blockAddrBegin = addrBegin & mask;  // b: L1中的一个block，要塞
    */
    uint32_t id                = this->lowerCache->getId(addrBegin);           // replace
    uint32_t blockIdBegin      = id * this->lowerCache->policy.associativity;  //!: 可能出bug
    uint32_t blockIdEnd        = (id + 1) * this->lowerCache->policy.associativity;
    uint32_t replaceId         = this->lowerCache->getReplacementBlockId(blockIdBegin, blockIdEnd);
    Block    blocktobereplaced = this->lowerCache->blocks[replaceId];

    this->lowerCache->blocks[replaceId] = tolower;
#ifdef CACHETEST
    printf("addrb: %x, tag: %x, id: %x ", addrBegin, tolower.tag, tolower.id);
    std::cout << " from level " << this->cachelevel << " evicted to level " << this->lowerCache->cachelevel << "!\n";
#endif
    if (blocktobereplaced.valid) {  //需要再写到L3/memory  2022:10.5：为什么每次都是valid?? 为什么没有hit in level2?
        this->lowerCache->writeBlockToLowerLevelEx(blocktobereplaced, cycles);
        this->statistics.totalCycles += this->policy.missLatency;
    }
    else {
#ifdef CACHETEST
        std::cout << "no victim in" << this->lowerCache->cachelevel << "\n";
#endif
    }

    //写到L2/L3，注意b中的参数是上层的！应改为lowercache的参数！！或者只要b中的数据（l1l2l3blocksize相等）
    //写到lowerCache，如果发生替换，那么换出的再写到下一级
}
