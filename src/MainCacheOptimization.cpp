/**
 * Entry point for the optimized cache
 *
 * Created by He, Hao at 2019/04/30
 */

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Cache.h"
#include "Debug.h"
#include "MemoryManager.h"

bool parseParameters(int argc, char **argv);
void printUsage();

const char *traceFilePath;

int main(int argc, char **argv) {
  if (!parseParameters(argc, argv)) {
    return -1;
  }

  Cache::Policy l1policy, l2policy, l3policy;
  l1policy.cacheSize = 32 * 1024;
  l1policy.blockSize = 64;
  l1policy.blockNum = 32 * 1024 / 64;
  l1policy.associativity = 8;
  l1policy.hitLatency = 2;
  l1policy.missLatency = 8;
  l2policy.cacheSize = 256 * 1024;
  l2policy.blockSize = 64;
  l2policy.blockNum = 256 * 1024 / 64;
  l2policy.associativity = 8;
  l2policy.hitLatency = 8;
  l2policy.missLatency = 100;
  l3policy.cacheSize = 8 * 1024 * 1024;
  l3policy.blockSize = 64;
  l3policy.blockNum = l3policy.cacheSize / l3policy.blockSize;
  l3policy.associativity = 8;
  l3policy.hitLatency = 20;
  l3policy.missLatency = 100;


  // Initialize memory and cache
  MemoryManager *memory = nullptr;
  Cache *        l1cache = nullptr, *l2cache = nullptr, *l3cache = nullptr, *l2cacheCore1 = nullptr, *l1cacheCore1 = nullptr;
  memory = new MemoryManager();
  l3cache = new Cache(memory, l3policy, 3, 0);
  l2cache = new Cache(memory, l2policy, 2, 0, l3cache);
  l1cache = new Cache(memory, l1policy, 1, 0, l2cache);

  l2cacheCore1 = new Cache(memory, l2policy, 2, 1, l3cache);
  l1cacheCore1 = new Cache(memory, l1policy, 1, 1, l2cacheCore1);

  l2cacheCore1->higherCache = l1cacheCore1;
  l2cache->higherCache      = l1cache;  // Connect all cache controllers with a ring

  memory->setCache(l1cache, l1cacheCore1);
  // memory->setCache(l1cache, l1cache);  //! randomly set here!

  // Read and execute trace in cache-trace/ folder
  std::ifstream trace(traceFilePath);
  if (!trace.is_open()) {
    printf("Unable to open file %s\n", traceFilePath);
    exit(-1);
  }

  char type; //'r' for read, 'w' for write
  char     corenumber;
  uint32_t addr;
  while (trace >> type >> corenumber >> std::hex >> addr) {
      if (!memory->isPageExist(addr))
          memory->addPage(addr);
      switch (type) {
      case 'r':
          switch (corenumber) {
          case '0':
              std::cout << "core 0 ";
              memory->getByte(addr, 0);
              break;
          case '1':
              std::cout << "core 1 ";
              memory->getByte(addr, 1);
              break;
          default:
              break;
          }
          break;
      case 'w':
          switch (corenumber) {
          case '0':
              std::cout << "core 0 ";
              memory->setByte(addr, 1, 0);
              break;
          case '1':
              std::cout << "core 1 ";
              memory->setByte(addr, 1, 1);
              break;
          default:
              break;
          }
          break;
      default:
          dbgprintf("Illegal type %c\n", type);
          exit(-1);
      }
  }

  // Output Simulation Results
  printf("L1 Cache:\n");
  l1cache->printStatistics();

  delete l1cache;
  delete l2cache;
  delete l3cache;
  delete memory;
  return 0;
}

bool parseParameters(int argc, char **argv) {
  // Read Parameters
  if (argc > 1) {
    traceFilePath = argv[1];
    return true;
  } else {
    return false;
  }
}

void printUsage() { printf("Usage: CacheSim trace-file\n"); }