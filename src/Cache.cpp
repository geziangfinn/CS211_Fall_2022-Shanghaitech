/*
 * Implementation of a simple cache simulator
 *
 * Created By He, Hao in 2019-04-27
 */

#include <cstdio>
#include <cstdlib>

#include "Cache.h"

//#define CACHETEST
//#define EXCLUSIVECACHE
//#define SDBP
//#define SDBPTEST
#define MESILOCK
//#define DEBUG
Cache::Cache(MemoryManager* manager, Policy policy, int cachelevel, int corenumber, Cache* lowerCache, bool writeBack, bool writeAllocate) {
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
    this->higherCache            = nullptr;
    this->corenumber             = corenumber;
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

uint8_t Cache::getByte(uint32_t addr, uint64_t pc, uint32_t* cycles) {
#ifdef EXCLUSIVECACHE
    return this->getByteEx(addr, cycles);
#endif
    this->referenceCounter++;
    this->statistics.numRead++;
#ifdef SDBP
    if (this->lowerCache == nullptr && pc != -1) {  // L3，更新samplerset和predictor，先更新predictor再predict
        this->getByteSamplerset(addr, pc);
    }
#endif
    // If in cache, return directly
    int blockId;  //在L2 L3中命中时，要invalidate block(exclusive cache)
    if ((blockId = this->getBlockId(addr)) != -1) {
        uint32_t offset = this->getOffset(addr);
        this->statistics.numHit++;
        this->statistics.totalCycles += this->policy.hitLatency;
        this->blocks[blockId].lastReference = this->referenceCounter;
#ifdef SDBP
        if (this->lowerCache == nullptr || pc != -1) {              // L3，predict
            this->blocks[blockId].deadblock = this->predictor(pc);  //预测
        }
#endif
        if (cycles)
            *cycles = this->policy.hitLatency;
#ifdef MESILOCK
        if (this->cachelevel == 1) {
            MESIoperationFwd(OWNREAD, addr);
        }
#endif
#ifdef DEBUG
        if (this->cachelevel == 1)
            std::cout << "read " << int(this->blocks[blockId].data[offset]) << " at " << std::hex << addr << "\n";
#endif
        return this->blocks[blockId].data[offset];
    }

    // Else, find the data in memory or other level of cache
    this->statistics.numMiss++;
    this->statistics.totalCycles += this->policy.missLatency;

#ifdef SDBP
    if (this->lowerCache == nullptr && pc != -1) {  //其实只有L3的block会dead，这里不写也没关系。
        if (this->predictor(pc)) {
            if (cycles)
                *cycles = 100;
#ifdef SDBPTEST
            std::cout << "One dead block bypassed!\n";
#endif
            return this->memory->getByteNoCache(addr);  //这里应该是：如果它是dead，那他压根就不应该替换已有的块，应该bypass LLC。也不该替换sampler中的块？
            // bypass cache  直接返回数据，不放进LLC。 如果是写呢？
        }
    }
#endif

    this->loadBlockFromLowerLevel(addr, pc, cycles);  // this=L3且bypass时直接返回数据，不要从memoryload进L3了，即不执行loadblockfrom

    // The block is in top level cache now, return directly
    if ((blockId = this->getBlockId(addr)) != -1) {
        uint32_t offset                     = this->getOffset(addr);
        this->blocks[blockId].lastReference = this->referenceCounter;
#ifdef SDBP
        if (this->lowerCache == nullptr && pc != -1) {              // L3，predict
            this->blocks[blockId].deadblock = this->predictor(pc);  //再预测，其实能load进LLC的肯定不是dead
        }
#endif

#ifdef MESILOCK
        if (this->cachelevel == 1) {
            MESIoperationFwd(OWNREAD, addr);
        }
#endif
#ifdef DEBUG
        if (this->cachelevel == 1)
            std::cout << "read " << int(this->blocks[blockId].data[offset]) << " at " << std::hex << addr << "\n";
#endif
        return this->blocks[blockId].data[offset];
    }
    else {
        fprintf(stderr, "Error: data not in top level cache!\n");
        exit(-1);
    }
}

void Cache::setByte(uint32_t addr, uint8_t val, uint64_t pc, uint32_t* cycles) {
#ifdef EXCLUSIVECACHE
    setByteEx(addr, val, cycles);
    return;
#endif
    this->referenceCounter++;
    this->statistics.numWrite++;
#ifdef SDBP
    if (this->lowerCache == nullptr && pc != -1) {  // L3，更新samplerset和predictor，先更新predictor再predict
        this->getByteSamplerset(addr, pc);
    }
#endif
    // If in cache, write to it directly
    int blockId;
    if ((blockId = this->getBlockId(addr)) != -1) {
        uint32_t offset = this->getOffset(addr);
        this->statistics.numHit++;
        this->statistics.totalCycles += this->policy.hitLatency;
        this->blocks[blockId].modified      = true;
        this->blocks[blockId].lastReference = this->referenceCounter;
#ifdef MESILOCK
        if (this->cachelevel == 1) {
            MESIoperationFwd(OWNWRITE, addr);
        }
#endif
        this->blocks[blockId].data[offset]  = val;
        if (!this->writeBack) {
            this->writeBlockToLowerLevel(this->blocks[blockId], pc);  //对于exclusive cache，write through应该只写到L1和memory:用setbytenocache替换writetolowerlevel
            this->statistics.totalCycles += this->policy.missLatency;
        }
#ifdef SDBP
        if (this->lowerCache == nullptr && pc != -1) {              // L3，predict
            this->blocks[blockId].deadblock = this->predictor(pc);  //预测
        }
#endif
        if (cycles)
            *cycles = this->policy.hitLatency;

#ifdef DEBUG
        if (this->cachelevel == 1) {
            std::cout << "set " << int(val) << " at " << std::hex << addr << "\n";
        }
#endif
        return;
    }
    //对于通过replace装填lower level,写一定不命中，如果用writetolowerlevel函数，会走到这里，miss不应++，只需要写就行了，modified也不应为true 单独增添一个函数处理replace load的情况？
    // Else, load the data from cache
    this->statistics.numMiss++;
    this->statistics.totalCycles += this->policy.missLatency;

#ifdef SDBP
    if (this->lowerCache == nullptr && pc != -1) {  //其实只有L3的block会dead，这里不写也没关系。
        if (this->predictor(pc)) {                  //这里应该是：如果它是dead，那他压根就不应该替换已有的块，应该bypass LLC。也不该替换sampler中的块？
#ifdef SDBPTEST
            std::cout << "One dead block bypassed!\n";
#endif
            this->memory->setByteNoCache(addr, val);
            return;  // bypass 不load到LLC，也不写到LLC，直接写到memory
        }
    }
#endif

    if (this->writeAllocate) {                        // writeAllocate lab1 only write allocate
        this->loadBlockFromLowerLevel(addr, pc, cycles);  // writeallocate写不命中时只load到L1

        if ((blockId = this->getBlockId(addr)) != -1) {  //然后写L1
            uint32_t offset                     = this->getOffset(addr);
            this->blocks[blockId].modified      = true;
            this->blocks[blockId].lastReference = this->referenceCounter;
#ifdef MESILOCK
            if (this->cachelevel == 1) {
                MESIoperationFwd(OWNWRITE, addr);
            }
#endif
            this->blocks[blockId].data[offset]  = val;
#ifdef SDBP
            if (this->lowerCache == nullptr && pc != -1) {              // L3，predict
                this->blocks[blockId].deadblock = this->predictor(pc);  //预测
            }
#endif

#ifdef DEBUG
            if (this->cachelevel == 1) {
                std::cout << "set " << int(val) << " at " << std::hex << addr << "\n";
            }
#endif
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
            this->lowerCache->setByte(addr, val, pc);
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
        b.MESI          = INVALID;  // LLC里的都是INVALID，但是无所谓，它已经不属于PRIVATE CACHE了
    }
    this->samplerblocks = std::vector<SamplerBlock>(policy.samplerblockNum);
    for (uint32_t i = 0; i < this->samplerblocks.size(); ++i) {
        SamplerBlock& b = this->samplerblocks[i];
        b.valid         = false;
        b.tag           = 0;
        b.lastReference = 0;
        b.partialPC     = 0;
        b.dead          = false;
        b.id            = i / policy.samplerassociativity;
    }

    this->predictionTable1 = std::vector<int>(4096, 0);
    this->predictionTable2 = std::vector<int>(4096, 0);
    this->predictionTable3 = std::vector<int>(4096, 0);
}

void Cache::loadBlockFromLowerLevel(uint32_t addr, uint64_t pc, uint32_t* cycles) {
    uint32_t blockSize = this->policy.blockSize;

    // Initialize new block from memory
    Block b;
    b.valid                 = true;
    b.modified              = false;
    b.tag                   = this->getTag(addr);
    b.id                    = this->getId(addr);
    b.size                  = blockSize;
    b.data                  = std::vector< uint8_t >(b.size);
    b.deadblock             = false;
    b.MESI                  = INVALID;  //! 如果L2中命中，则L2不会调用这一句，否则则应该调用这一句，因为cache（L1+L2）MISS 了
    uint32_t bits           = this->log2i(blockSize);
    uint32_t mask           = ~((1 << bits) - 1);
    uint32_t blockAddrBegin = addr & mask;
    for (uint32_t i = blockAddrBegin; i < blockAddrBegin + blockSize; ++i) {
        if (this->lowerCache == nullptr) {  // LLC中也没命中
            b.data[i - blockAddrBegin] = this->memory->getByteNoCache(i);
            if (cycles)
                *cycles = 100;  //访存的延迟
        }
        else
            b.data[i - blockAddrBegin] = this->lowerCache->getByte(i, pc, cycles);  // exclusive cache: flush data from lower level cache when loading:set to invalid?
        // TODO: exclusive cache 现行的实现会从memory或lowerlevel加载到每一个higher level，evict时不会放到lower level
    }

    if (cachelevel == 1)  //! 此时L2里肯定已经有了
    {
        b.MESI = this->lowerCache->blocks[this->lowerCache->getBlockId(addr)].MESI;  // INVALID 或L2命中时L2中的MESI状态
    }
    //! 如果在LLC或memory命中，那么cache中block的mesi初始状态是invalid，放在cache的mesi函数中处理，如果在L2命中，MESI状态一定不是INVALID，
    //! 需要把L2中的mesi状态传递上去,因为invalid态的更新不会留到下一个cycle。且L2中的数据是最新的，由policy保证，与MESI无关
    //这里replace包含valid和invalid两种情况，不是valid的那个替换，即需要写到下一级的那种。
    // Find replace block
    uint32_t id           = this->getId(addr);
    uint32_t blockIdBegin = id * this->policy.associativity;
    uint32_t blockIdEnd   = (id + 1) * this->policy.associativity;
    uint32_t replaceId    = this->getReplacementBlockId(blockIdBegin, blockIdEnd);  // exclusive cache: flush from higher level cache and put in lower level
    Block    replaceBlock =
        this->blocks[replaceId];  // exclusive cache：只要被替换（replaceblock.valid=true），不管是不是dirty，是不是writeback，都要写并且只写到下一级，L3的victim是写到内存(dirty writeback)或者扔掉

    if (this->cachelevel == 1 || this->cachelevel == 2) {
        // std::cout << replaceBlock.MESI << " " << this->corenumber << " evcit " << std::hex << this->getAddr(replaceBlock) << " from level" << this->cachelevel << "\n";
        this->MESIevict(replaceBlock);
    }

    if (this->writeBack && replaceBlock.valid && replaceBlock.modified) {  // write back to memory                           // Task2: 写回时调用getSamplerBlockId不算
        this->writeBlockToLowerLevel(replaceBlock, -1);                    // 通过writeback调用setbyte的不能算命中？
        //没有修改的Victim会被扔掉，应改为放且只放到下一级，还是单独加一个函数来处理好,这里就给Writeback用，新函数调用的也应该是新函数。
        // writeback时和通过replace来load时，L2，L3写必不命中，因为exclusive writeback+exclusive: 只有replace的时候才会写到lowerlevel，lowerlevel只通过replace装载
        this->statistics.totalCycles += this->policy.missLatency;  //再加一个misslatency
    }
    this->blocks[replaceId] = b;  // b.modified=false 本函数中应该只有L1有这一步，只有L1调用这个函数？？或只有L1的b.valid=true
    // L3存L2的victim，L2的victim只可能是被L1的victim挤出来，因为L2只存L1的victim。L2中何时发生替换（也是装载）：在L1中发生替换时。L1中何时发生替换：读：不命中并load时
    // 写：不命中并allocate时，两种情况都是通过loadfromlower来做替换命中时不存在替换 L3中何时发生替换（也是装载）：在L2中发生替换时，也即L1中发生替换时，也即那两种情况时
}  //可能需要写新的write block to lower level，其中要有新的替换逻辑

uint32_t Cache::getReplacementBlockId(uint32_t begin, uint32_t end) {
    // Find invalid block first, or deadblock

    for (uint32_t i = begin; i < end; ++i) {
        if ((!this->blocks[i].valid))
            return i;  // vaild的才需要写到下一级cache，valid才发生了我所说的“替换”
    }

#ifdef SDBP
    for (uint32_t i = begin; i < end; ++i) {
        if ((this->blocks[i].deadblock))  //只有L3的block会dead
        {
#ifdef SDBPTEST
            std::cout << "One dead block replaced!\n";
#endif
            return i;  // vaild的才需要写到下一级cache，valid才发生了我所说的“替换”
        }
    }
#endif

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

void Cache::writeBlockToLowerLevel(Cache::Block& b, uint64_t pc) {
    uint32_t addrBegin = this->getAddr(b);
    if (this->lowerCache == nullptr) {
        for (uint32_t i = 0; i < b.size; ++i) {
            this->memory->setByteNoCache(addrBegin + i, b.data[i]);
        }
    }
    else {
        for (uint32_t i = 0; i < b.size; ++i) {
            this->lowerCache->setByte(addrBegin + i, b.data[i], pc);  // exclusive cache中应只放到下一级，不应是现在这样递归的
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
            // std::cout << addrBegin << "\n";
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
                // std::cout << addrBegin << "\n";
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
            // this->lowerCache->setByte(addr, val,pc);
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
            b.data[i - blockAddrBegin] = lower->blocks[blockId].data[i - blockAddrBegin];  //!: 可能有bug！！默认各级的block大小不变
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

uint64_t Cache::predictorhash1(uint64_t partialPC) {
    //先把PC砍成15位(lower 15 bits)
    uint64_t mask0 = (1 << 15) - 1;
    uint64_t maskt = (1 << 12) - 1;
    partialPC &= mask0;
    uint64_t A1     = partialPC & maskt;  // lower 12 bits
    uint64_t A2     = partialPC >> 3;     // higher 12 bits
    uint64_t result = skewh(A1) ^ reverseskewh(A2) ^ A2;
    if (result <= mask0) {
        return result;
    }
    printf("hash1 overflow!\n");
    exit(-1);
}
uint64_t Cache::predictorhash2(uint64_t partialPC) {
    //先把PC砍成15位
    uint64_t mask0 = (1 << 15) - 1;
    uint64_t maskt = (1 << 12) - 1;
    partialPC &= mask0;
    uint64_t A1     = partialPC & maskt;  // lower 12 bits
    uint64_t A2     = partialPC >> 3;     // higher 12 bits
    uint64_t result = skewh(A1) ^ reverseskewh(A2) ^ A1;
    if (result <= mask0) {
        return result;
    }
    printf("hash2 overflow!\n");
    exit(-1);
}
uint64_t Cache::predictorhash3(uint64_t partialPC) {
    //先把PC砍成15位
    uint64_t mask0 = (1 << 15) - 1;
    uint64_t maskt = (1 << 12) - 1;
    partialPC &= mask0;
    uint64_t A1     = partialPC & maskt;  // lower 12 bits
    uint64_t A2     = partialPC >> 3;     // higher 12 bits
    uint64_t result = reverseskewh(A1) ^ skewh(A2) ^ A2;
    if (result <= mask0) {
        return result;
    }
    printf("hash3 overflow!\n");
    exit(-1);
}

void Cache::updatepredictor(uint64_t pc, bool accessorevict) {
    int updater;  // true为access decrement, false为evict increment
    if (accessorevict) {
        updater = -1;
    }
    else {
        updater = 1;
#ifdef SDBPTEST
        std::cout << "inc\n";
#endif
    }
    int t1 = this->predictionTable1[this->predictorhash1(pc)];
    int t2 = this->predictionTable2[this->predictorhash2(pc)];
    int t3 = this->predictionTable3[this->predictorhash3(pc)];
    // std::cout << updater << "\n";
    t1 += updater;
    t2 += updater;
    t3 += updater;

    t1 = saturating(t1);
    t2 = saturating(t2);
    t3 = saturating(t3);

    this->predictionTable1[this->predictorhash1(pc)] = t1;
    this->predictionTable2[this->predictorhash2(pc)] = t2;
    this->predictionTable3[this->predictorhash3(pc)] = t3;
}
bool Cache::predictor(uint64_t pc) {
    bool dead = this->predictionTable1[this->predictorhash1(pc)] + this->predictionTable2[this->predictorhash2(pc)] + this->predictionTable3[this->predictorhash3(pc)] >= 8;
#ifdef SDBPTEST
    if (dead)
        std::cout << "one block dead\n";
#endif
    return dead;
}

uint32_t Cache::getSamplerBlockId(uint32_t addr) {
    uint32_t tag = this->getTag(addr);
    uint32_t id  = this->getSamplerSetId(addr);
    // printf("0x%x 0x%x 0x%x\n", addr, tag, id);
    // iterate over the given set
    for (uint32_t i = id * policy.samplerassociativity; i < (id + 1) * policy.samplerassociativity; ++i) {
        if (this->samplerblocks[i].id != id) {
            fprintf(stderr, "Inconsistent ID in samplerblock %d\n", i);
            exit(-1);
        }
        if (this->samplerblocks[i].valid && this->samplerblocks[i].tag == tag) {  // valid!
            return i;
        }
    }
    return -1;
}

uint32_t Cache::getSamplerSetId(uint32_t addr) {
    uint32_t id        = this->getId(addr);
    uint32_t samplerid = id / this->policy.samplesize;  //确定了是samplerset才能调用此函数
    return samplerid;
}

void Cache::getByteSamplerset(uint32_t addr, uint64_t pc) {
    uint32_t id = this->getId(addr);
    if (id % this->policy.samplesize == 0) {  //如果命中的是samplerset sample的set则要更新samplerset和predictor。更新（被命中的）samplerblock的pc sample第 0,64,128...
        int samplerblockId;
        if ((samplerblockId = this->getSamplerBlockId(addr)) != -1) {  // samplerset中命中
            this->samplerblocks[samplerblockId].lastReference = this->referenceCounter;
            this->samplerblocks[samplerblockId].partialPC     = pc;
            // std::cout << "hit\n";
        }
        else {
            //更新sampler set 这里也是向samplerset load block 的关键
            //如果dead，还要不要放到samplerblock?
            if (this->predictor(pc)) {
                return;  //死块不放进samplerset
            }
            SamplerBlock sb;
            sb.valid         = true;
            sb.tag           = this->getTag(addr);
            sb.dead          = false;
            sb.lastReference = this->referenceCounter;  //在此更改referencecounter
            sb.partialPC     = pc;
            sb.id            = this->getSamplerSetId(addr);
            // std::cout << sb.id << " sb\n";
            samplerblockId        = this->getSamplerBlockId(addr);
            uint32_t samplersetid = this->getSamplerSetId(addr);
            // std::cout << sb.id << " sam\n";
            uint32_t blockIdBegin = samplersetid * this->policy.samplerassociativity;
            uint32_t blockIdEnd   = (samplersetid + 1) * this->policy.samplerassociativity;
            uint32_t replaceId    = this->getSamplerReplacementBlockId(blockIdBegin, blockIdEnd);

            SamplerBlock replaceBlock = this->samplerblocks[replaceId];
            if (replaceBlock.valid) {
                updatepredictor(replaceBlock.partialPC, false);  // evict, increment
            }
            this->samplerblocks[replaceId] = sb;
        }
        updatepredictor(pc, true);  //命中，decrement
    }
    // 通过writeback调用setbyte的不能算命中？用PC=-1标识 反而应该算evict，因为它在上级都被evict了，但论文里好像没说。 但是LLC好像只可能因为writeback而写命中，因为是写分配的。
    // 通过load调用的get算命中
    // !:记录到报告中
}

uint32_t Cache::getSamplerReplacementBlockId(uint32_t begin, uint32_t end) {
    // Find invalid block first
    for (uint32_t i = begin; i < end; ++i) {
        if ((!this->samplerblocks[i].valid)) {
            // std::cout << "novalid";
            return i;
        }
    }

    // Otherwise use LRU
    uint32_t resultId = begin;
    uint32_t min      = this->samplerblocks[begin].lastReference;
    for (uint32_t i = begin; i < end; ++i) {
        if (this->samplerblocks[i].lastReference < min) {
            resultId = i;
            min      = this->samplerblocks[i].lastReference;
        }
    }
    return resultId;
}

int Cache::saturating(int counter) {
    if (counter < 0)
        return 0;
    if (counter > 3)
        return 3;
    return counter;
}

uint64_t Cache::skewh(uint64_t partialPC) {
    // PartialPC is 12 bits long
    return (((partialPC & 1) ^ (partialPC >> 11)) << 11) | (partialPC >> 1);
}

uint64_t Cache::reverseskewh(uint64_t partialPC) {
    uint64_t mask = (1 << 12) - 1;
    return ((partialPC >> 11) ^ ((partialPC >> 10) & 1)) | ((partialPC << 1) & mask);
}

void Cache::MESIoperationFwd(int operationcode, uint32_t addr) {  //?将本函数分成forward和receive两块？ forward肯定是在L1调用，因为要读写的block会被拉到L1，receive时，要操作的块不一定在哪呢
    // operation应当对L1和L2中的块都进行。要注意函数被L1还是L2调用，放在哪里调用。
    if (this->cachelevel != 1) {
        std::cout << "fwd error!\n";
        exit(0);
    }
    std::pair<int, std::vector<uint8_t>> memmesireturn;

    uint32_t blockaddr = addr >> 6;

    int directoryoriginalmesi;

    std::vector<uint8_t> datafromothercore;

    int original = this->readBlockMESIfromCache(addr);

    int newmesi;
    // ! ownread/write时，数据一定会被拉到L1里，在L1的get/set里，块已在L1中之后调用本函数，调用者一定是L1
    // !otherread/write时由memorymanager的mesi函数调用，调用的cache仍然是L1。提供数据放到另一个函数？
    // !evict: 只有l1或l2中保存了本块时，在发生替换时由本core调用
    switch (operationcode) {
    case OWNREAD:
        switch (original) {
        case MODIFIED:
            //无需修改 silent
            break;
        case EXCLUSIVE:
            // std::cout << "core " << this->corenumber << " request " << std::hex << addr << "(block: " << int(blockaddr) << ") : E->E\n";
            //无需修改
            break;
        case SHARED:
            //无需修改
            break;
        case INVALID:
            // INVALID时，L2里一定有，因为是一路load上去的
            memmesireturn = this->memory->MESIoperationmem(OWNREAD, addr, this);  //报告directory

            directoryoriginalmesi = memmesireturn.first;
            datafromothercore     = memmesireturn.second;

            newmesi = SHARED;
            if (directoryoriginalmesi == INVALID) {
                newmesi = EXCLUSIVE;
            }
            if (directoryoriginalmesi == MODIFIED || directoryoriginalmesi == EXCLUSIVE) {
                this->setdatainCache(addr, datafromothercore);  // ! potential bug because '=' for vector
            }
            this->setBlockMESIinCache(addr, newmesi);  //修改两级中block的mesi和数据
            //! directory中原状态为 M/E 时需要读other core, S/I时直接读LLC/memory，数据不用改。
            //  todo:确定状态转到S还是E需要先访问directory，
            //  todo 数据从哪来？directory中为M或E时要从othercore来，否则从
            //  todo memory来  默认从LLC和memory读，读上来后一定是INVALID态，再在这里改数据。状态也在这里修改。
            break;
        default:
            break;
        }
        break;
    case OWNWRITE:  //数据的修改要放在这里吗？
        switch (original) {
        case MODIFIED:  // M->M
            //无需修改
            break;
        case EXCLUSIVE:  // silently E->M
            newmesi = MODIFIED;
            this->setBlockMESIinCache(addr, newmesi);
            // todo 两级cache里的都要改
            // todo 无需报告directory或向别的core发送invalidate
            break;
        case SHARED:                                                               // S->M
            memmesireturn = this->memory->MESIoperationmem(OWNWRITE, addr, this);  //!此时directory中一定是S
            //! potential bug caused by 'this'
            //!向directory发送的都是own，directory发送的都是other，directory里先检查block和directory状态是否相符？
            newmesi = MODIFIED;
            this->setBlockMESIinCache(addr, newmesi);
            //!修改的数据的写入由policy完成
            // todo 两级cache里的都要改
            // todo 报告directory，由directory向别的core发送invalidate（也可能本core是唯一sharer）directory里改modified
            break;
        case INVALID:  // I->M
                       // !INVALID时，L2里一定有本块，因为是一路load上去的
            memmesireturn         = this->memory->MESIoperationmem(OWNWRITE, addr, this);
            directoryoriginalmesi = memmesireturn.first;
            if (directoryoriginalmesi == MODIFIED) {
                // std::cout << "reached here\n";
                datafromothercore = memmesireturn.second;       // 114514
                this->setdatainCache(addr, datafromothercore);  // 114514
            }
            //  directoryoriginalmesi = memmesireturn.first;//无需返回数据
            newmesi = MODIFIED;
            this->setBlockMESIinCache(addr, newmesi);
            // !修改的数据的写入由policy完成。这样L1中的数据是最新的，L2中的不是，这由policy管，跟平常一样，从LLC/MEMORYload后，L1写了最新数据，L2并没有
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Cache::MESIoperationRec(int operationcode, uint32_t addr) {  //本函数还是被L1cache调用，但块不一定在L1还是L2
    if (this->cachelevel != 1) {
        std::cout << "rec error!\n";
        exit(0);
    }
    int original;
    original = this->readBlockMESIfromCache(addr);
    int          newmesi;
    Cache::Block blocktobewrittenback;
    switch (operationcode) {
    case OTHERREAD:
        switch (original) {
        case MODIFIED:  // M->S
            newmesi                  = SHARED;
            blocktobewrittenback     = this->dataTrans(addr);
            blocktobewrittenback.tag = this->lowerCache->getTag(addr);
            blocktobewrittenback.id  = this->lowerCache->getId(addr);
            this->lowerCache->writeBlockToLowerLevel(blocktobewrittenback, -1);  //! potential bug!!!
            this->setBlockMESIinCache(addr, newmesi);
            //? M->S时应该把dirty改掉？一定要写回吗？？？？
            // todo 写回到LLC即可？
            //  todo 两级cache里的都要改
            //  todo 提供数据并将数据写回LLC/memory，状态改为S，requestor应为 I->S
            //  todo 提供数据的操作在mem里，两级都找，这里只要改状态，
            //! 但是改状态也要找两级cache，block不一定在L1还是L2
            break;
        case EXCLUSIVE:
            newmesi                  = SHARED;
            blocktobewrittenback     = this->dataTrans(addr);
            blocktobewrittenback.tag = this->lowerCache->getTag(addr);
            blocktobewrittenback.id  = this->lowerCache->getId(addr);
            this->lowerCache->writeBlockToLowerLevel(blocktobewrittenback, -1);  //! potential bug!!!
            this->setBlockMESIinCache(addr, newmesi);
            // todo 写回到LLC即可？写回到LLC的话注意要设dirty: 由policy完成
            // todo 两级cache里的都要改
            // todo 提供数据的操作在mem里，两级都找，这里只要改状态，
            // todo 提供数据，状态改为S，requestor应为 I->S
            break;
        case SHARED:
            //无需修改，状态不改，也不需要提供数据
            break;
        case INVALID:
            std::cout << "otherread 0\n";  //雨我无瓜，无操作
            break;
        default:
            break;
        }
        break;
    case OTHERWRITE:
        switch (original) {
        case MODIFIED:  //本Core是owner，requestor是I->M
            newmesi = INVALID;
            this->setBlockMESIinCache(addr, newmesi);
            this->invalidateBlockinCache(addr);
            // todo 两级cache里的都要改
            // todo invalidate，不用写回，因为requestor马上又要写了 slides里标了个写回
            //! 需要传数据！false sharing
            break;
        case EXCLUSIVE:  //本Core是owner，requestor是I->M
            newmesi = INVALID;
            this->setBlockMESIinCache(addr, newmesi);
            this->invalidateBlockinCache(addr);
            // todo 两级cache里的都要改
            // todo invalidate，不用写回，因为requestor马上又要写了 slides里标了个写回
            //! 需要传数据！false sharing
            //! M和E区别：putM时需要写回，putE时不需要
            break;
        case SHARED:  //本core不是owner，requestor是S->M或I->M
            newmesi = INVALID;
            this->setBlockMESIinCache(addr, newmesi);
            this->invalidateBlockinCache(addr);
            // todo 两级cache里的都要改
            // todo invalidate
            break;
        case INVALID:
            std::cout << "otherwrite 0\n";  //雨我无瓜，无操作
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

void Cache::MESIevict(Block& Block) {  //!本函数由L1或L2调用
    if (this->cachelevel != 1 && this->cachelevel != 2) {
        std::cout << "evict error\n";
        exit(0);
    }
    std::pair<int, std::vector<uint8_t>> returnPair;
    uint32_t                             addr = this->getAddr(Block);
    // std::cout << std::hex << Block.tag << Block.id;
    // std::cout << addr" evicted\n";
    bool   isthisblockonlyinonelevel;
    Cache* memorycontrlcache;
    if (this->cachelevel == 1) {
        isthisblockonlyinonelevel = (this->inCache(addr)) ^ (this->lowerCache->inCache(addr));
        memorycontrlcache         = this;
    }
    else {
        isthisblockonlyinonelevel = (this->inCache(addr)) ^ (this->higherCache->inCache(addr));
        memorycontrlcache         = this->higherCache;
    }

    int original = Block.MESI;

    uint32_t blockaddr = addr >> 6;
    // if (!isthisblockonlyinonelevel)
    // std::cout << "114514\n";

    if (isthisblockonlyinonelevel) {
        switch (original) {
        case MODIFIED:  // PUTM
            if (this->cachelevel == 2) {
                returnPair = memorycontrlcache->memory->MESIoperationmem(EVICT, addr, memorycontrlcache);
            }
            //? 写回由policy完成？ L1也需要写回，不是写回L2而是写回LLC
            // 如果block只在L1，那么不算evict，因为他是modifid，dirty，会被写回L2 要保证MODIFIED的block一定是dirty
            // 这与EXCLUSIVE 和 SHARED 不一致，这两个如果只在L1就丢掉了
            // !L2中被evcit时本来就要写回，写回由policy完成？
            // todo 此时L1或L2中只有某一级含该block，先判断是哪一级再调用
            // todo 写回，directory状态改为Invalidate，写回可由policy完成？
            // block的状态无需再改，都不在Cache里了
            break;
        case EXCLUSIVE:  // PUTE
            std::cout << "core " << this->corenumber << " request " << std::hex << addr << "(block: " << int(blockaddr) << ") : E->I\n";
            returnPair = memorycontrlcache->memory->MESIoperationmem(EVICT, addr, memorycontrlcache);
            // todo 此时L1或L2中只有某一级含该block
            // todo 无需写回，directory状态改为Invalidate
            break;
        case SHARED:  // PUTS
            returnPair = memorycontrlcache->memory->MESIoperationmem(EVICT, addr, memorycontrlcache);
            // todo 此时L1或L2中只有某一级含该block
            // todo 无需写回，cache中block状态改为Invalidate，directory中可能是S可能是E
            break;
        case INVALID:
            //无操作
            break;
        default:
            break;
        }
    }
}

int Cache::readBlockMESIfromCache(uint32_t addr) {
    if (this->cachelevel != 1) {
        std::cout << "read mesi error\n";
        exit(0);
    }
    int blockId = this->getBlockId(addr);
    if (blockId != -1) {  //优先读L1，未检查两级中mesi状态是否相等
        return this->blocks[blockId].MESI;
    }
    else {
        blockId = this->lowerCache->getBlockId(addr);
        if (blockId == -1) {
            std::cout << "data not in l1 or l2a\n";
            exit(0);
        }
        return this->lowerCache->blocks[blockId].MESI;
    }
}

void Cache::setBlockMESIinCache(uint32_t addr, int mesi) {
    if (this->cachelevel != 1) {
        std::cout << "set mesi error\n";
        exit(0);
    }
    bool whetherinbothlevel = this->inCache(addr) && this->lowerCache->inCache(addr);  //为真则块在both level
    int  blockId            = this->getBlockId(addr);
    if (blockId != -1) {  //块在L1
        this->blocks[blockId].MESI = mesi;
        if (whetherinbothlevel) {  //块也在L2
            blockId                                = this->lowerCache->getBlockId(addr);
            this->lowerCache->blocks[blockId].MESI = mesi;
        }
    }
    else {  //块只在L2
        blockId = this->lowerCache->getBlockId(addr);
        if (blockId == -1) {
            std::cout << "data not in l1 or l2b\n";
            exit(0);
        }
        this->lowerCache->blocks[blockId].MESI = mesi;
    }
}

Cache::Block Cache::dataTrans(uint32_t addr) {
    if (this->cachelevel != 1) {
        std::cout << "data trans error!\n";
        exit(0);
    }
    int blockId = this->getBlockId(addr);

    if (blockId != -1) {
        return blocks[blockId];
    }
    else {
        blockId = this->lowerCache->getBlockId(addr);
        return this->lowerCache->blocks[blockId];
    }
}

void Cache::setdatainCache(uint32_t addr, std::vector<uint8_t> block) {
    if (this->cachelevel != 1) {
        std::cout << "set mesi error\n";
        exit(0);
    }
    bool whetherinbothlevel = this->inCache(addr) && this->lowerCache->inCache(addr);  //为真则块在both level
    int  blockId            = this->getBlockId(addr);
    if (blockId != -1) {  //块在L1
        this->blocks[blockId].data = block;
        if (whetherinbothlevel) {  //块也在L2
            blockId                                = this->lowerCache->getBlockId(addr);
            this->lowerCache->blocks[blockId].data = block;
        }
    }
    else {  //块只在L2
        blockId = this->lowerCache->getBlockId(addr);
        if (blockId == -1) {
            std::cout << "data not in l1 or l2d\n";
            exit(0);
        }
        this->lowerCache->blocks[blockId].data = block;
    }
}

void Cache::invalidateBlockinCache(uint32_t addr) {
    if (this->cachelevel != 1) {
        std::cout << "invalidate error\n";
        exit(0);
    }
    bool whetherinbothlevel = this->inCache(addr) && this->lowerCache->inCache(addr);  //为真则块在both level
    int  blockId            = this->getBlockId(addr);
    if (blockId != -1) {  //块在L1
        this->blocks[blockId].valid = false;
        if (whetherinbothlevel) {  //块也在L2
            blockId                                 = this->lowerCache->getBlockId(addr);
            this->lowerCache->blocks[blockId].valid = false;
        }
    }
    else {  //块只在L2
        blockId = this->lowerCache->getBlockId(addr);
        if (blockId == -1) {
            std::cout << "data not in l1 or l2e\n";
            exit(0);
        }
        this->lowerCache->blocks[blockId].valid = false;
    }
}
