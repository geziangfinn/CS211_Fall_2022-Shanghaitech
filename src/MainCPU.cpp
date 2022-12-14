/*
 * Created by He, Hao at 2019-3-11
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <elfio/elfio.hpp>

#include "BranchPredictor.h"
#include "Cache.h"
#include "Debug.h"
#include "MemoryManager.h"
#include "Simulator.h"

#define DUALCORE

bool parseParameters(int argc, char **argv);
void printUsage();
void printElfInfo(ELFIO::elfio *reader);
void loadElfToMemory(ELFIO::elfio *reader, MemoryManager *memory);


char *elfFile = nullptr;
char* core0Elf = nullptr;
char* core1Elf = nullptr;

bool verbose = 0;
bool isSingleStep = 0;
bool dumpHistory = 0;
uint32_t stackBaseAddr      = 0x80000000;  // Core 0
uint32_t stackBaseAddrCore1 = 0x70000000;  // Core 1
uint32_t stackSize          = 0x400000;    // Core 0 and Core 1

uint32_t clockCycle = 0;

MemoryManager memory;
Cache *l1Cache, *l2Cache, *l3Cache;
Cache *       l1CacheCore1, *l2CacheCore1;

BranchPredictor::Strategy strategy = BranchPredictor::Strategy::NT;

BranchPredictor branchPredictor;
BranchPredictor branchPredictorCore1;

Simulator simulator(&memory, &branchPredictor, 0);
Simulator simulatorCore1(&memory, &branchPredictorCore1, 1);  // share memory but not BP

int main(int argc, char **argv) {
  if (!parseParameters(argc, argv)) {
    printUsage();
    exit(-1);
  }
  // Init cache

  Cache::Policy l1Policy, l2Policy, l3Policy;

  l1Policy.cacheSize     = 32 * 1024;
  l1Policy.blockSize = 64;
  l1Policy.blockNum = l1Policy.cacheSize / l1Policy.blockSize;
  l1Policy.associativity = 8;
  l1Policy.hitLatency = 0;
  l1Policy.missLatency = 8;

  l2Policy.cacheSize     = 256 * 1024;
  l2Policy.blockSize     = 64;
  l2Policy.blockNum = l2Policy.cacheSize / l2Policy.blockSize;
  l2Policy.associativity = 8;
  l2Policy.hitLatency = 8;
  l2Policy.missLatency = 20;

  l3Policy.cacheSize            = 8 * 1024 * 1024;
  l3Policy.blockSize = 64;
  l3Policy.blockNum = l3Policy.cacheSize / l3Policy.blockSize;
  l3Policy.associativity = 8;
  l3Policy.hitLatency = 20;
  l3Policy.missLatency = 100;
  l3Policy.samplerassociativity = (l3Policy.associativity * 3) / 4;
  l3Policy.samplesize           = 64;
  l3Policy.samplerblockNum      = ((l3Policy.blockNum / l3Policy.associativity) / l3Policy.samplesize) * l3Policy.samplerassociativity;  // a sampler set for every 64 sets

  l3Cache = new Cache(&memory, l3Policy, 3, -1);
  l2Cache = new Cache(&memory, l2Policy, 2, 0, l3Cache);
  l1Cache = new Cache(&memory, l1Policy, 1, 0, l2Cache);

  l2CacheCore1 = new Cache(&memory, l2Policy, 2, 1, l3Cache);
  l1CacheCore1 = new Cache(&memory, l1Policy, 1, 1, l2CacheCore1);

  l2CacheCore1->higherCache = l1CacheCore1;
  l2Cache->higherCache      = l1Cache;  // Connect all cache controllers with a ring

  memory.setCache(l1Cache, l1CacheCore1);  // todo: how to add Core0 cache and Core1 cache? add core number as an parameter in memory::get/setByte()?
                                           // todo??? so core number should be added in simulator.cpp as well use set/getnocache when functions are not called by cores
  // Read ELF file
  ELFIO::elfio reader;
  ELFIO::elfio readerCore1;

  // if (!reader.load(elfFile)) {
  //     fprintf(stderr, "Fail to load ELF file %s!\n", elfFile);
  //     return -1;
  // }

#ifdef DUALCORE  //?????????????????????????????????
  if (!reader.load(core0Elf)) {
      fprintf(stderr, "Fail to load ELF file %s!\n", core0Elf);
      return -1;
  }
  if (!readerCore1.load(core1Elf)) {
      fprintf(stderr, "Fail to load ELF file %s!\n", core1Elf);
      return -1;
  }
#endif

  if (verbose) {
    printElfInfo(&reader);
  }
#ifdef DUALCORE
  loadElfToMemory(&reader, &memory);       // Core0
  loadElfToMemory(&readerCore1, &memory);  // Core1
#endif
  if (verbose) {
    memory.printInfo();
  }

  simulator.isSingleStep = isSingleStep;
  simulator.verbose = verbose;
  simulator.shouldDumpHistory = dumpHistory;
  simulator.branchPredictor->strategy = strategy;
  simulator.pc                        = reader.get_entry();
  simulator.initStack(stackBaseAddr, stackSize);

  simulatorCore1.isSingleStep              = isSingleStep;  // share these 3 parameters
  simulatorCore1.verbose                   = verbose;
  simulatorCore1.shouldDumpHistory         = dumpHistory;
  simulatorCore1.branchPredictor->strategy = strategy;
  simulatorCore1.pc                        = readerCore1.get_entry();
  simulatorCore1.initStack(stackBaseAddrCore1, stackSize);

  // simulator.simulate();

  simulator.setUp();
  simulatorCore1.setUp();

  for (int i = 0; i < 1024; i++) {
      uint32_t start = 0x100;
      start += i;
      start <<= 12;
      memory.addPage(start);  //??????????????????(4MB)
  }
#ifdef DUALCORE
  while (true) {
      if (!simulator.terminateCore() && simulator.stallCycles() == 0) {
          simulator.singleStep(clockCycle);  // todo: history.cycle??????????????????????????????
      }
      if (!simulatorCore1.terminateCore() && simulatorCore1.stallCycles() == 0) {
          simulatorCore1.singleStep(clockCycle);
      }
      clockCycle++;
      if (simulator.terminateCore() && simulatorCore1.terminateCore()) {
          break;
      }
  }
#endif

  if (dumpHistory) {
      printf("Dumping history to dump.txt...\n");
      simulator.dumpHistory();
  }
  delete l1CacheCore1;
  delete l2CacheCore1;
  delete l1Cache;
  delete l2Cache;
  delete l3Cache;
  return 0;
}

bool parseParameters(int argc, char **argv) {
  // Read Parameters
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'v':
        verbose = 1;
        break;
      case 's':
        isSingleStep = 1;
        break;
      case 'd':
        dumpHistory = 1;
        break;
      case 'c': {
          if (argv[i][2] == '0') {
              if (core0Elf == nullptr) {
                  core0Elf = argv[i + 1];
              }
              else {
                  return false;
              }
          }
          else if (argv[i][2] == '1') {
              if (core1Elf == nullptr) {
                  core1Elf = argv[i + 1];
              }
              else {
                  return false;
              }
          }
          else {
              printf("Invalid core number\n");
              return false;
          }
          break;
      }
      case 'b':
        if (i + 1 < argc) {
          std::string str = argv[i + 1];
          i++;
          if (str == "AT") {
            strategy = BranchPredictor::Strategy::AT;
          } else if (str == "NT") {
            strategy = BranchPredictor::Strategy::NT;
          } else if (str == "BTFNT") {
            strategy = BranchPredictor::Strategy::BTFNT;
          } else if (str == "BPB") {
            strategy = BranchPredictor::Strategy::BPB;
          }
          else if (str == "PERCEPTRON") {
              strategy = BranchPredictor::Strategy::PERCEPTRON;
          }
          else {
              return false;
          }
        } else {
          return false;
        }
        break;
      default:
        return false;
      }
    } else {
        // if (elfFile == nullptr) {
        //   elfFile = argv[i];
        // } else {
        //   return false;
        // }
    }
  }  // !:parseparameter?????????????????????????????????????????????????????????????????????main???
  // if (elfFile == nullptr) {
  //   return false;
  // }
  return true;
}

void printUsage() {
  printf("Usage: Simulator riscv-elf-file [-v] [-s] [-d] [-b param]\n");
  printf("Parameters: \n\t[-v] verbose output \n\t[-s] single step\n");
  printf("\t[-d] dump memory and register trace to dump.txt\n");
  printf("\t[-b param] branch perdiction strategy, accepted param AT, NT, "
         "BTFNT, BPB, PERCEPTRON\n");
}

void printElfInfo(ELFIO::elfio *reader) {
  printf("==========ELF Information==========\n");

  if (reader->get_class() == ELFCLASS32) {
    printf("Type: ELF32\n");
  } else {
    printf("Type: ELF64\n");
  }

  if (reader->get_encoding() == ELFDATA2LSB) {
    printf("Encoding: Little Endian\n");
  } else {
    printf("Encoding: Large Endian\n");
  }

  if (reader->get_machine() == EM_RISCV) {
    printf("ISA: RISC-V(0x%x)\n", reader->get_machine());
  } else {
    dbgprintf("ISA: Unsupported(0x%x)\n", reader->get_machine());
    exit(-1);
  }

  ELFIO::Elf_Half sec_num = reader->sections.size();
  printf("Number of Sections: %d\n", sec_num);
  printf("ID\tName\t\tAddress\tSize\n");

  for (int i = 0; i < sec_num; ++i) {
    const ELFIO::section *psec = reader->sections[i];

    printf("[%d]\t%-12s\t0x%llx\t%lld\n", i, psec->get_name().c_str(),
           psec->get_address(), psec->get_size());
  }

  ELFIO::Elf_Half seg_num = reader->segments.size();
  printf("Number of Segments: %d\n", seg_num);
  printf("ID\tFlags\tAddress\tFSize\tMSize\n");

  for (int i = 0; i < seg_num; ++i) {
    const ELFIO::segment *pseg = reader->segments[i];

    printf("[%d]\t0x%x\t0x%llx\t%lld\t%lld\n", i, pseg->get_flags(),
           pseg->get_virtual_address(), pseg->get_file_size(),
           pseg->get_memory_size());
  }

  printf("===================================\n");
}

void loadElfToMemory(ELFIO::elfio *reader, MemoryManager *memory) {
  ELFIO::Elf_Half seg_num = reader->segments.size();
  for (int i = 0; i < seg_num; ++i) {
    const ELFIO::segment *pseg = reader->segments[i];

    uint64_t fullmemsz = pseg->get_memory_size();
    uint64_t fulladdr = pseg->get_virtual_address();
    // Our 32bit simulator cannot handle this
    if (fulladdr + fullmemsz > 0xFFFFFFFF) {
      dbgprintf(
          "ELF address space larger than 32bit! Seg %d has max addr of 0x%lx\n",
          i, fulladdr + fullmemsz);
      exit(-1);
    }

    uint32_t filesz = pseg->get_file_size();
    uint32_t memsz = pseg->get_memory_size();
    uint32_t addr   = ( uint32_t )pseg->get_virtual_address();

    for (uint32_t p = addr; p < addr + memsz; ++p) {
      if (!memory->isPageExist(p)) {
        memory->addPage(p);
      }

      if (p < addr + filesz) {
        memory->setByteNoCache(p, pseg->get_data()[p - addr]);
      } else {
        memory->setByteNoCache(p, 0);
      }
    }
  }  // Lab3: ??????????????????elf???memory?????????????????????????????????????????? 2022.11.8: ???????????????ELF???????????????elf?????????????????????memory???????????????????????????
}