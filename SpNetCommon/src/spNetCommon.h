#pragma once

#include <iostream>

#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <optional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>

#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "Containers/StaticArray.h"

// Send server error message
#define SERVER_ERROR(e) std::cerr << "[SERVER] " << " Exception: " << e << "\n"

// Send client error message
#define CLIENT_ERROR(e) std::cerr <<  "[CLIENT] " << " Exception: "<< e << "\n"

// Send server-like log message
#define SERVER_MESSAGE(msg)  std::cout << "[SERVER] " << msg << "\n" 

// Send client-like log message
#define CLIENT_MESSAGE(msg)  std::cout << "[CLIENT] " << msg << "\n"

using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

using int8 = char;
using int16 = short;
using int32 = int;
using int64 = long long;

using b8 = bool;
using float32 = float;
using float64 = double;

// Undef Windows stuff that conflicts
#ifdef _WIN32
#undef min 
#undef max 
#undef ERROR 
#undef DELETE 
#undef MessageBox 
#undef Error
#undef OK
#undef CONNECT_DEFERRED 
#endif

#undef ABS
#undef SIGN
#undef MIN
#undef MAX
#undef CLAMP
#undef CopyMemory