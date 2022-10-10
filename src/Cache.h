/*
 * Basic cache simulator
 *
 * Created by He, Hao on 2019-4-27
 */

#ifndef CACHE_H
#define CACHE_H

#include <cstdint>
#include <vector>

#include "MemoryManager.h"

class MemoryManager;

class Cache {
public:
    struct Policy {
        // In bytes, must be power of 2
        uint32_t cacheSize;
        uint32_t blockSize;
        uint32_t blockNum;
        uint32_t associativity;
        uint32_t hitLatency;   // in cycles
        uint32_t missLatency;  // in cycles
    };

    struct SamplerBlock {
        uint32_t tag;
        uint32_t partialPC;
        bool     dead;
        bool     valid;
        uint32_t lastReference;
        // LRU通过读已有的referencenumber即可实现，partialPC怎么得到？有了partialPC，predictor的代码也可以写在cache里
        //搞清楚什么时候访问L3和Sampler的换入换出
        // TODO: samplerBlock 和samplerblocks的构造，只用L3的sampler即可，每层都可以有。利用cache level可表明LLC
    };

    struct Block {  // added in 2022.10.9 commit
        bool                 valid;
        bool                 modified;
        uint32_t             tag;
        uint32_t             id;
        uint32_t             size;
        uint32_t             lastReference;
        std::vector<uint8_t> data;
        bool                 deadblock;
        Block() {}
        Block(const Block& b) : valid(b.valid), modified(b.modified), tag(b.tag), id(b.id), size(b.size) {
            data = b.data;
        }
    };

    struct Statistics {
        uint32_t numRead;
        uint32_t numWrite;
        uint32_t numHit;
        uint32_t numMiss;
        uint64_t totalCycles;
    };

    Cache(MemoryManager* manager, Policy policy, int cachelevel, Cache* lowerCache = nullptr, bool writeBack = true, bool writeAllocate = true);

    bool     inCache(uint32_t addr);
    uint32_t getBlockId(uint32_t addr);
    uint8_t  getByte(uint32_t addr, uint32_t* cycles = nullptr);
    uint8_t  getByteEx(uint32_t addr, uint32_t* cycles = nullptr);
    void     setByte(uint32_t addr, uint8_t val, uint32_t* cycles = nullptr);
    void     setByteEx(uint32_t addr, uint8_t val, uint32_t* cycles = nullptr);
    void     printInfo(bool verbose);
    void     printStatistics();

    Statistics statistics;

private:
    uint32_t                  referenceCounter;
    bool                      writeBack;      // default true
    bool                      writeAllocate;  // default true
    MemoryManager*            memory;
    Cache*                    lowerCache;
    Policy                    policy;
    std::vector<Block>        blocks;
    std::vector<SamplerBlock> samplerblocks;
    bool                      exclusive = false;
    int                       cachelevel;

    std::vector<Block> predictionTable1;  // added in 2022.10.9 commit
    std::vector<Block> predictionTable2;  // added in 2022.10.9 commit
    std::vector<Block> predictionTable3;  // added in 2022.10.9 commit

    void     initCache();
    void     loadBlockFromLowerLevel(uint32_t addr, uint32_t* cycles = nullptr);
    Block    loadBlockFromLowerLevelEx(uint32_t addr, uint32_t* cycles = nullptr, Cache* lower = nullptr);
    uint32_t getReplacementBlockId(uint32_t begin, uint32_t end);
    void     writeBlockToLowerLevel(Block& b);
    void     writeBlockToLowerLevelEx(Block& b, uint32_t* cycles);

    uint32_t predictorhash1(uint32_t partialPC);  // added in 2022.10.9 commit
    uint32_t predictorhash2(uint32_t partialPC);  // added in 2022.10.9 commit
    uint32_t predictorhash3(uint32_t partialPC);  // added in 2022.10.9 commit

    // Utility Functions
    bool     isPolicyValid();
    bool     isPowerOfTwo(uint32_t n);
    uint32_t log2i(uint32_t val);
    uint32_t getTag(uint32_t addr);
    uint32_t getId(uint32_t addr);
    uint32_t getOffset(uint32_t addr);
    uint32_t getAddr(Block& b);
};

#endif