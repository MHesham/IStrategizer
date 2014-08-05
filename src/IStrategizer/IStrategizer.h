#ifndef ISTRATEGIZER_H
#define ISTRATEGIZER_H

#include "RtsAiEngine.h"

#ifndef RTSAIENGINEAPI
#define RTSAIENGINEAPI extern "C" __declspec(dllimport)
#endif // RTSAIENGINEAPI

RTSAIENGINEAPI IStrategizer::IRtsAiEngineFactory* GetRtsAiEngineFactory();

#endif // ISTRATEGIZER_H