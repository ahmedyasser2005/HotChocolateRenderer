#pragma once

#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

#include "HCR/Core/Win32API.hpp"

#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Weverything"
#endif

#include <DirectXMath.h>
// #include <DirectXTex.h>

#ifdef __clang__
	#pragma clang diagnostic pop
#endif

// Standard Libs
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <execution>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
