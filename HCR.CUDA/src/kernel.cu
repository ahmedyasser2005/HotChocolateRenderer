#include "CUDA/kernel.cuh"
#include <cuda_runtime.h>
#include <cassert>

namespace
{
	static __global__ void gradient_kernel( uint8_t* pixels, const uint32_t width, const uint32_t height )
	{
		const auto x = blockDim.x * blockIdx.x + threadIdx.x;
		const auto y = blockDim.y * blockIdx.y + threadIdx.y;

		if( !( x < width && y < height ) ) [[unlikely]]
			return;

		const auto idx = 4 * ( y * width + x );

		pixels[idx] = static_cast<uint8_t>( 255.0f * x / ( width - 1 ) );       // red
		pixels[idx + 1] = static_cast<uint8_t>( 255.0f * y / ( height - 1 ) );  // green
		pixels[idx + 2] = 128ui8;                                               // blue
		pixels[idx + 3] = 255ui8;                                               // alpha
	}
}  // namespace

namespace HCR
{
	void make_gradient( uint8_t** pixels, const uint32_t width, const uint32_t height )
	{
		assert( width > 0 && height > 0 );
		assert( pixels && *pixels );

		[[maybe_unused]] cudaError_t err = cudaError::cudaSuccess;

		uint8_t* d_pixels = nullptr;

		const size_t sizeInBytes = static_cast<size_t>( 4 ) * width * height;

		err = cudaMalloc( (void**)&d_pixels, sizeInBytes );
		assert( err == cudaError::cudaSuccess );

		constexpr dim3 block{ 16, 16 };
		const dim3 grid{ ( width + block.x - 1 ) / block.x, ( height + block.y - 1 ) / block.y };

		gradient_kernel<<<grid, block>>>( d_pixels, width, height );
		err = cudaGetLastError();
		assert( err == cudaError::cudaSuccess );

		err = cudaDeviceSynchronize();
		assert( err == cudaError::cudaSuccess );

		err = cudaMemcpy( *pixels, d_pixels, sizeInBytes, cudaMemcpyDeviceToHost );
		assert( err == cudaError::cudaSuccess );

		err = cudaFree( d_pixels );
		assert( err == cudaError::cudaSuccess );
	}
}  // namespace HCR
