#pragma once
#include "pch.h"

struct SyncContext {
    std::shared_mutex rw;
    std::counting_semaphore<> sem{ 0 };
};
