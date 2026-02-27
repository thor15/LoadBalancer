#pragma once
#ifndef PCH_H
#define PCH_H

#include <thread>
#include <stdio.h>
#include <shared_mutex>
#include <semaphore>
#include <mutex>
#include <chrono>
#include <math.h>
#include <time.h>
#include <random>
#include <string>
#include <queue>
#include <array>
#include <fstream>
#include <condition_variable>
#include "Request.h"
#include "MutexContex.h"
#include "WebServer.h"
#include "GenerateRequests.h"
#include "LoadBalancer.h"
#include "Stats.h"
#include "Scaler.h"

#define MILI_PER_CYCLE 20
#define GREEN  "\x1b[32m"
#define RESET  "\x1b[0m"
#define RED    "\x1b[31m"
#define YELLOW "\x1b[33m"
#define RED_BOLD "\x1b[1;31m"

// add headers that you want to pre-compile here

#endif //PCH_H