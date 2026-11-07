#pragma once

#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace HCR
{
	class Graphics final
	{
	public:
		Graphics( const Graphics& ) = delete;
		Graphics& operator=( const Graphics& ) = delete;
		Graphics( Graphics&& ) = delete;
		Graphics& operator=( Graphics&& ) = delete;

		static Graphics& Get()
		{
			static Graphics graphics{};
			return graphics;
		}

		void Start( HWND hwnd, uint32_t width, uint32_t height ) noexcept;
		void Shutdown() noexcept;

		void Render() noexcept;

	private:
		Graphics() noexcept = default;
		~Graphics() noexcept = default;

		static constexpr UINT bufferCount{ 2u };

		UINT d3dRTVDescHeapSize{ 0u };
		uint64_t fenceValue{ 0ui64 };
		HANDLE fenceEvent{ nullptr };
		UINT nVertices{}, nIndices{};

		Microsoft::WRL::ComPtr<IDXGIFactory7>			  dxgiFactory{};
		Microsoft::WRL::ComPtr<ID3D12Device14>			  d3dDevice{};
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		  d3dCommandQueue{};
		Microsoft::WRL::ComPtr<IDXGISwapChain4>			  dxgiSwapChain{};
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			  dxgiSwapChain1{};
		Microsoft::WRL::ComPtr<ID3D12Resource>			  d3dDepthBuffer{};
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	  d3dRTVDescHeap{};
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	  d3dDSVDescHeap{};
		CD3DX12_CPU_DESCRIPTOR_HANDLE					  dsvHandle{};
		Microsoft::WRL::ComPtr<ID3D12Resource>			  backBuffers[bufferCount]{};
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	  d3dCommandAllocator{};
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dCommandList{};
		Microsoft::WRL::ComPtr<ID3D12Fence>				  d3dFence{};
		Microsoft::WRL::ComPtr<ID3D12Resource>			  d3dVertexBuffer{};
		Microsoft::WRL::ComPtr<ID3D12Resource>			  d3dIndexBuffer{};
		Microsoft::WRL::ComPtr<ID3D12RootSignature>		  d3dRootSignature{};
		Microsoft::WRL::ComPtr<ID3D12PipelineState>		  d3dPipelineState{};

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};

		CD3DX12_RECT d3dScissorRect{};
		CD3DX12_VIEWPORT d3dViewport{};
		D3D12_CLEAR_VALUE depthClearValue{};

	};

	inline Graphics& g_gfx = Graphics::Get();

} // namespace HCR
