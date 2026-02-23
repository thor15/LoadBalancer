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
#include <condition_variable>
#include "Request.h"
#include "MutexContex.h"
#include "WebServer.h"
#include "GenerateRequests.h"
#include "LoadBalancer.h"
#include "Stats.h"
#include "Scaler.h"




// add headers that you want to pre-compile here

#endif //PCH_H