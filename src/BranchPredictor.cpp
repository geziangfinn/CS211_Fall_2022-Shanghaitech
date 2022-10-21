/*
 * Created by He, Hao on 2019-3-25
 */

#include "BranchPredictor.h"
#include "Debug.h"

BranchPredictor::BranchPredictor() {
  for (int i = 0; i < PRED_BUF_SIZE; ++i) {
    this->predbuf[i] = WEAK_TAKEN;
  }
  this->perceptronnumber = 100;
  this->historylength    = 34;
  this->perceptronTable  = std::vector<std::vector<int>>(this->perceptronnumber, std::vector<int>(this->historylength + 1, 0));
  this->historyregister  = 0;
  this->yout             = 0;
  this->threshold        = 79;
}

BranchPredictor::~BranchPredictor() {}

bool BranchPredictor::predict(uint32_t pc, uint32_t insttype, int64_t op1,
                              int64_t op2, int64_t offset) {
  switch (this->strategy) {
  case NT:
    return false;
  case AT:
    return true;
  case BTFNT: {
    if (offset >= 0) {
      return false;
    } else {
      return true;
    }
  }
  break;
  case BPB: {
    PredictorState state = this->predbuf[pc % PRED_BUF_SIZE];
    if (state == STRONG_TAKEN || state == WEAK_TAKEN) {
      return true;
    } else if (state == STRONG_NOT_TAKEN || state == WEAK_NOT_TAKEN) {
      return false;
    } else {
      dbgprintf("Strange Prediction Buffer!\n");
    }
  } break;
  case PERCEPTRON: {
      return perceptronPredict(pc);
  } break;
  default:
      dbgprintf("Unknown Branch Perdiction Strategy!\n");
      break;
  }
  return false;
}

void BranchPredictor::update(uint32_t pc, bool branch) {
  int id = pc % PRED_BUF_SIZE;
  PredictorState state = this->predbuf[id];
  if (branch) {
    if (state == STRONG_NOT_TAKEN) {
      this->predbuf[id] = WEAK_NOT_TAKEN;
    } else if (state == WEAK_NOT_TAKEN) {
      this->predbuf[id] = WEAK_TAKEN;
    } else if (state == WEAK_TAKEN) {
      this->predbuf[id] = STRONG_TAKEN;
    } // do nothing if STRONG_TAKEN
  } else { // not branch
    if (state == STRONG_TAKEN) {
      this->predbuf[id] = WEAK_TAKEN;
    } else if (state == WEAK_TAKEN) {
      this->predbuf[id] = WEAK_NOT_TAKEN;
    } else if (state == WEAK_NOT_TAKEN) {
      this->predbuf[id] = STRONG_NOT_TAKEN;
    } // do noting if STRONG_NOT_TAKEN
  }   //这样实现比真的+1好多了，sdbp里的实现太傻了
}

std::string BranchPredictor::strategyName() {
  switch (this->strategy) {
  case NT:
    return "Always Not Taken";
  case AT:
    return "Always Taken";
  case BTFNT:
    return "Back Taken Forward Not Taken";
  case BPB:
    return "Branch Prediction Buffer";
  case PERCEPTRON:
      return "Perceptron predictor";
  default:
    dbgprintf("Unknown Branch Perdiction Strategy!\n");
    break;
  }
  return "error"; // should not go here
}

void BranchPredictor::updatehistoryregister(bool branch) {
    uint64_t mask         = (1 << this->historylength) - 1;  //会有问题吗？
    this->historyregister = this->historyregister << 1;
    this->historyregister = this->historyregister & mask;
    if (branch)
        this->historyregister += 1;
}

bool BranchPredictor::perceptronPredict(uint32_t pc) {
    int      id     = pc % this->perceptronnumber;
    int      result = 0;  //需要用long吗
    int      x;
    uint64_t tempregister = this->historyregister;
    result += perceptronTable[id][0];  // x0=1;
    for (int i = 1; i <= this->historylength; i++) {
        if ((tempregister & 1) == 0) {
            x = -1;
        }
        else {
            x = 1;  // bit 为0时应视为-1, 1就是1
        }
        result += perceptronTable[id][i] * x;
        tempregister >>= 1;
    }
    this->yout = result;
    if (this->yout < 0)
        yout = 0 - yout;
    return result >= 0;
}

void BranchPredictor::perceptronUpdate(uint32_t pc, bool branch, bool predictedBranch) {

    int t = -1;
    if (branch) {
        t = 1;
    }
    int      id           = pc % this->perceptronnumber;
    uint64_t tempregister = this->historyregister;
    int      x;
    if ((predictedBranch != branch) || yout <= this->threshold)  //预测错了才update? 看论文 Yout到底是啥?
    {
        perceptronTable[id][0] += t;  // x0=1;
        for (int i = 1; i <= this->historylength; i++) {
            if ((tempregister & 1) == 0) {
                x = -1;
            }
            else {
                x = 1;  // bit 为0时应视为-1, 1就是1
            }
            perceptronTable[id][i] += t * x;
            tempregister >>= 1;
        }
    }
    this->updatehistoryregister(branch);  //先trainperceptron再升级history register?
}
