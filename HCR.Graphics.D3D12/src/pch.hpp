#pragma once

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "HCR/Core/Win32API.hpp"

#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

#include <DirectXMath.h>
//#include <DirectXTex.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif


// Standard Libs
#include <iostream>
#include <print>
#include <format>
#include <string>
#include <string_view>
#include <span>
#include <cstdint>
#include <functional>
#include <memory>
#include <chrono>
#include <stdexcept>
#include <bitset>
#include <array>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <system_error>
#include <concepts>
#include <execution>
#include <numeric>
#include <variant>
#include <assert.h>

//#ifdef _DEBUG
//[[maybe_unused]] inline constexpr auto IS_DEBUG = true;
//#else
//[[maybe_unused]] inline constexpr auto IS_DEBUG = false;
//#endif
