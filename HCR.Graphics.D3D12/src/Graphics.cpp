#include "pch.hpp"
#include "Graphics/Graphics.hpp"
#include "HCR/Core/Logger.hpp"
#include "HCR/Platform/Window.hpp"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
namespace rn = std::ranges;


void HCR::Graphics::Start( HWND hwnd, uint32_t width, uint32_t height ) noexcept
{
	LINFO( "Starting Graphics..." );

	[[maybe_unused]] HRESULT hr = S_OK;

#pragma region DX12 Debug Layer
#if _DEBUG
	ComPtr<ID3D12Debug6> d3dDebug;
	{
		hr = D3D12GetDebugInterface( IID_PPV_ARGS( &d3dDebug ) );
		assert( SUCCEEDED( hr ) && "Failed to get the debug interface" );
		d3dDebug->EnableDebugLayer();
		d3dDebug->SetEnableGPUBasedValidation( TRUE );
	}
#endif
#pragma endregion

#pragma region Create Factory
	hr = CreateDXGIFactory2( DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS( &dxgiFactory ) );
	assert( SUCCEEDED( hr ) && "Failed to create the factory" );
#pragma endregion

#pragma region Create Device
	hr = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS( &d3dDevice ) );
	assert( SUCCEEDED( hr ) && "Failed to create the device" );
#pragma endregion

#pragma region Create Command Queue
	constexpr D3D12_COMMAND_QUEUE_DESC cqd{
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0u,
	};
	hr = d3dDevice->CreateCommandQueue( &cqd, IID_PPV_ARGS( &d3dCommandQueue ) );
	assert( SUCCEEDED( hr ) && "Failed to create the command queue" );
#pragma endregion

#pragma region Create SwapChain
	const DXGI_SWAP_CHAIN_DESC1 scd{
		.Width = width,
		.Height = height,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.Stereo = FALSE,
		.SampleDesc = {
			.Count = 1u,
			.Quality = 0u,
		},
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = bufferCount,
		.Scaling = DXGI_SCALING_STRETCH,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags = 0u,
	};
	constexpr DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd{
		.RefreshRate = {
			.Numerator = 0u,
			.Denominator = 1u,
		},
		.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
		.Windowed = TRUE,
	};
	hr = dxgiFactory->CreateSwapChainForHwnd( d3dCommandQueue.Get(), hwnd, &scd, &scfd, nullptr, &dxgiSwapChain1 );
	assert( SUCCEEDED( hr ) && "Failed to create the swapchain" );
#pragma endregion

#pragma region Upgrade SwapChain
	hr = dxgiSwapChain1.As( &dxgiSwapChain );
	assert( SUCCEEDED( hr ) && "Failed to upgrade the swapchain" );
#pragma endregion

#pragma region RTV Descriptor Heap
	constexpr D3D12_DESCRIPTOR_HEAP_DESC rtvdhd{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = bufferCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0u,
	};
	hr = d3dDevice->CreateDescriptorHeap( &rtvdhd, IID_PPV_ARGS( &d3dRTVDescHeap ) );
	assert( SUCCEEDED( hr ) && "Failed to create the RTV descriptor heap" );

	d3dRTVDescHeapSize = d3dDevice->GetDescriptorHandleIncrementSize( rtvdhd.Type );

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ d3dRTVDescHeap->GetCPUDescriptorHandleForHeapStart() };
	for( uint8_t i = 0ui8; i < bufferCount; ++i )
	{
		hr = dxgiSwapChain->GetBuffer( i, IID_PPV_ARGS( &backBuffers[i] ) );
		assert( SUCCEEDED( hr ) && std::format( "Failed to get buffer number {}", i ).data() );
		d3dDevice->CreateRenderTargetView( backBuffers[i].Get(), nullptr, rtvHandle );
		rtvHandle.Offset( d3dRTVDescHeapSize );
	}
#pragma endregion

#pragma region DSV Resource
	const CD3DX12_HEAP_PROPERTIES dhp{ D3D12_HEAP_TYPE_DEFAULT };

	const CD3DX12_RESOURCE_DESC rd{
		CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT, width, height,
			1u, 0u, 1u, 0u,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		)
	};

	depthClearValue = {
		.Format = DXGI_FORMAT_D32_FLOAT,
		.DepthStencil = {
			.Depth = 1.0f,
			.Stencil = 0u,
		},
	};

	hr = d3dDevice->CreateCommittedResource( &dhp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS( &d3dDepthBuffer ) );
	assert( SUCCEEDED( hr ) && "Failed to create the depth resource" );
#pragma endregion

#pragma region DSV Descriptor Heap
	constexpr D3D12_DESCRIPTOR_HEAP_DESC dsvhd{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		.NumDescriptors = 1u,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0u,
	};
	hr = d3dDevice->CreateDescriptorHeap( &dsvhd, IID_PPV_ARGS( &d3dDSVDescHeap ) );
	assert( SUCCEEDED( hr ) && "Failed to create the DSV descriptor heap" );

	constexpr D3D12_DEPTH_STENCIL_VIEW_DESC dsvd{
		.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT,
		.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
		.Texture2D = {
			.MipSlice = 0u
		},
	};
	dsvHandle = { d3dDSVDescHeap->GetCPUDescriptorHandleForHeapStart() };
	d3dDevice->CreateDepthStencilView( d3dDepthBuffer.Get(), &dsvd, dsvHandle );
#pragma endregion

#pragma region Command Allocator
	hr = d3dDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &d3dCommandAllocator ) );
	assert( SUCCEEDED( hr ) && "Failed to create a command allocator" );
#pragma endregion

#pragma region Command List
	hr = d3dDevice->CreateCommandList( 0u, D3D12_COMMAND_LIST_TYPE_DIRECT, d3dCommandAllocator.Get(), nullptr, IID_PPV_ARGS( &d3dCommandList ) );
	assert( SUCCEEDED( hr ) && "Failed to create the graphics command list" );
	hr = d3dCommandList->Close();
	assert( SUCCEEDED( hr ) && "Failed to close the graphics command list" );
#pragma endregion

#pragma region Fence
	hr = d3dDevice->CreateFence( fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &d3dFence ) );
	assert( SUCCEEDED( hr ) && "Failed to create a fence" );
#pragma endregion

#pragma region Create Fence Signaling Event
	fenceEvent = CreateEventW( nullptr, FALSE, FALSE, nullptr );
	assert( (fenceEvent != nullptr) && "Failed to create a fence event" );
#pragma endregion

	//////////// Rendering ////////////

#pragma region Create Geometry Data on the CPU
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 color;
	};

	constexpr std::array<Vertex, 8> vertices{
		Vertex{ XMFLOAT3{ -0.5f, -0.5f, -0.5f }, XMFLOAT3{ 0.15f, 0.15f, 0.20f } }, // Dark Slate
		Vertex{ XMFLOAT3{ -0.5f,  0.5f, -0.5f }, XMFLOAT3{ 0.20f, 0.80f, 0.60f } }, // Emerald Green
		Vertex{ XMFLOAT3{ 0.5f,  0.5f, -0.5f }, XMFLOAT3{ 0.95f, 0.75f, 0.30f } }, // Warm Gold
		Vertex{ XMFLOAT3{ 0.5f, -0.5f, -0.5f }, XMFLOAT3{ 0.90f, 0.35f, 0.35f } }, // Coral Red
		Vertex{ XMFLOAT3{ -0.5f, -0.5f,  0.5f }, XMFLOAT3{ 0.35f, 0.55f, 0.90f } }, // Soft Blue
		Vertex{ XMFLOAT3{ -0.5f,  0.5f,  0.5f }, XMFLOAT3{ 0.40f, 0.85f, 0.90f } }, // Soft Cyan
		Vertex{ XMFLOAT3{ 0.5f,  0.5f,  0.5f }, XMFLOAT3{ 0.95f, 0.95f, 0.95f } }, // Bright Off-White
		Vertex{ XMFLOAT3{ 0.5f, -0.5f,  0.5f }, XMFLOAT3{ 0.80f, 0.40f, 0.80f } }, // Lavender / Purple
	};
	constexpr std::array<uint32_t, 36> indices{
		0, 1, 2,   0, 2, 3,
		7, 6, 5,   7, 5, 4,
		4, 5, 1,   4, 1, 0,
		3, 2, 6,   3, 6, 7,
		1, 5, 6,   1, 6, 2,
		4, 0, 3,   4, 3, 7,
	};

	nVertices = static_cast<UINT>(vertices.size());
	nIndices = static_cast<UINT>(indices.size());
#pragma endregion

#pragma region Create a Vertex Buffer Resource
	const CD3DX12_HEAP_PROPERTIES hpd1{ D3D12_HEAP_TYPE_DEFAULT };
	const auto rd1{ CD3DX12_RESOURCE_DESC::Buffer( sizeof( vertices ) ) };
	hr = d3dDevice->CreateCommittedResource( &hpd1,
											 D3D12_HEAP_FLAG_NONE,
											 &rd1,
											 D3D12_RESOURCE_STATE_COPY_DEST,
											 nullptr,
											 IID_PPV_ARGS( &d3dVertexBuffer ) );
	assert( SUCCEEDED( hr ) && "Failed to create a resource" );
#pragma endregion

#pragma region Create a Vertex Buffer Upload Resource
	ComPtr<ID3D12Resource> d3dVertexUploadBuffer;
	{
		const CD3DX12_HEAP_PROPERTIES hpd2{ D3D12_HEAP_TYPE_UPLOAD };
		const auto rd2{ CD3DX12_RESOURCE_DESC::Buffer( sizeof( vertices ) ) };
		hr = d3dDevice->CreateCommittedResource( &hpd2,
												 D3D12_HEAP_FLAG_NONE,
												 &rd2,
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 nullptr,
												 IID_PPV_ARGS( &d3dVertexUploadBuffer ) );
		assert( SUCCEEDED( hr ) && "Failed to create a resource" );
	}
#pragma endregion

#pragma region Create a Index Buffer Resource
	CD3DX12_HEAP_PROPERTIES hpd3{ D3D12_HEAP_TYPE_DEFAULT };
	const auto rd3{ CD3DX12_RESOURCE_DESC::Buffer( sizeof( indices ) ) };
	hr = d3dDevice->CreateCommittedResource( &hpd3,
										D3D12_HEAP_FLAG_NONE,
										&rd3,
										D3D12_RESOURCE_STATE_COPY_DEST,
										nullptr,
										IID_PPV_ARGS( &d3dIndexBuffer ) );
	assert( SUCCEEDED( hr ) && "Failed to create a resource" );
#pragma endregion

#pragma region Create a Index Buffer Upload Resource
	ComPtr<ID3D12Resource> d3dIndexUploadBuffer;
	{
		CD3DX12_HEAP_PROPERTIES hpd4{ D3D12_HEAP_TYPE_UPLOAD };
		const auto rd4{ CD3DX12_RESOURCE_DESC::Buffer( sizeof( indices ) ) };
		hr = d3dDevice->CreateCommittedResource( &hpd4,
											D3D12_HEAP_FLAG_NONE,
											&rd4,
											D3D12_RESOURCE_STATE_GENERIC_READ,
											nullptr,
											IID_PPV_ARGS( &d3dIndexUploadBuffer ) );
		assert( SUCCEEDED( hr ) && "Failed to create a resource" );
	}
#pragma endregion

#pragma region Upload Vertex & Index Data
	Vertex* mappedVertexData = nullptr;
	hr = d3dVertexUploadBuffer->Map( 0u, nullptr, reinterpret_cast<void**>(&mappedVertexData) );
	assert( SUCCEEDED( hr ) && "Failed to map the vertex upload buffer" );
	rn::copy( vertices, mappedVertexData );
	d3dVertexUploadBuffer->Unmap( 0u, nullptr );

	uint32_t* mappedIndexData = nullptr;
	hr = d3dIndexUploadBuffer->Map( 0u, nullptr, reinterpret_cast<void**>(&mappedIndexData) );
	assert( SUCCEEDED( hr ) && "Failed to map the index upload buffer" );
	rn::copy( indices, mappedIndexData );
	d3dIndexUploadBuffer->Unmap( 0u, nullptr );
#pragma endregion

#pragma region Make Vertex Buffer View
	vertexBufferView = {
		.BufferLocation = d3dVertexBuffer->GetGPUVirtualAddress(),
		.SizeInBytes = static_cast<UINT>(nVertices * sizeof( Vertex )),
		.StrideInBytes = sizeof( Vertex ),
	};

	indexBufferView = {
		.BufferLocation = d3dIndexBuffer->GetGPUVirtualAddress(),
		.SizeInBytes = static_cast<UINT>(nIndices * sizeof( uint32_t )),
		.Format = DXGI_FORMAT_R32_UINT,
	};
#pragma endregion

#pragma region Submit the copy request for the vertex & index buffers to the command queue
	{
		hr = d3dCommandAllocator->Reset();
		assert( SUCCEEDED( hr ) && "Failed to reset the command allocator" );
		hr = d3dCommandList->Reset( d3dCommandAllocator.Get(), nullptr );
		assert( SUCCEEDED( hr ) && "Failed to reset the command list" );

		d3dCommandList->CopyResource( d3dVertexBuffer.Get(), d3dVertexUploadBuffer.Get() );
		d3dCommandList->CopyResource( d3dIndexBuffer.Get(), d3dIndexUploadBuffer.Get() );

		const CD3DX12_RESOURCE_BARRIER barriers[2] = {
			CD3DX12_RESOURCE_BARRIER::Transition(
				d3dVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			),
			CD3DX12_RESOURCE_BARRIER::Transition(
				d3dIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER
			)
		};

		d3dCommandList->ResourceBarrier( std::size( barriers ), barriers );

		hr = d3dCommandList->Close();
		assert( SUCCEEDED( hr ) && "Failed to close the command list" );
		ID3D12CommandList* const commandLists[] = { d3dCommandList.Get() };
		d3dCommandQueue->ExecuteCommandLists( static_cast<UINT>(std::size( commandLists )), commandLists );
	}
#pragma endregion

#pragma region Fence Signaling Event
	hr = d3dCommandQueue->Signal( d3dFence.Get(), ++fenceValue );
	assert( SUCCEEDED( hr ) && "Failed to signal the command queue" );
	hr = d3dFence->SetEventOnCompletion( fenceValue, fenceEvent );
	assert( SUCCEEDED( hr ) && "Failed to set event on completion" );
	[[maybe_unused]] const bool success = WaitForSingleObject( fenceEvent, INFINITE ) != WAIT_FAILED;
	assert( success && "Failed to wait for fence event" );
#pragma endregion

#pragma region Root Signature
	CD3DX12_ROOT_PARAMETER rootParams[1]{};
	rootParams[0].InitAsConstants( sizeof( XMMATRIX ) / 4u, 0u, 0u, D3D12_SHADER_VISIBILITY_VERTEX );

	D3D12_ROOT_SIGNATURE_FLAGS rsflags{
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
	};

	CD3DX12_ROOT_SIGNATURE_DESC rsd{};
	rsd.Init( static_cast<UINT>(std::size( rootParams )), rootParams, 0u, nullptr, rsflags );

	ComPtr<ID3DBlob> d3dSignatureBlob;
	ComPtr<ID3DBlob> d3dErrorBlob;

	hr = D3D12SerializeRootSignature( &rsd, D3D_ROOT_SIGNATURE_VERSION_1, &d3dSignatureBlob, &d3dErrorBlob );
	if( FAILED( hr ) )
	{
		[[maybe_unused]] auto errorText = static_cast<const char*>(d3dErrorBlob->GetBufferPointer());
		LERROR( errorText );
		assert( SUCCEEDED( hr ) && "Failed to serialize root signature" );
	}

	hr = d3dDevice->CreateRootSignature( 0u, d3dSignatureBlob->GetBufferPointer(), d3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS( &d3dRootSignature ) );
	assert( SUCCEEDED( hr ) && "Failed to create a root signature" );
#pragma endregion

#pragma region Pipeline State Object
	constexpr D3D12_INPUT_ELEMENT_DESC ilds[]{
		{ "POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u },
		{ "COLOR", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u },
	};

	ComPtr<ID3DBlob> d3dVSBlob;
	ComPtr<ID3DBlob> d3dPSBlob;

	hr = D3DReadFileToBlob( L"VertexShader.cso", &d3dVSBlob );
	assert( SUCCEEDED( hr ) && "Failed to read vertex shader to blob" );
	hr = D3DReadFileToBlob( L"PixelShader.cso", &d3dPSBlob );
	assert( SUCCEEDED( hr ) && "Failed to read pixel shader to blob" );

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_VS VertexShader;
		CD3DX12_PIPELINE_STATE_STREAM_PS PixelShader;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopology;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
	} pipelineStateStream;

	pipelineStateStream.RootSignature = d3dRootSignature.Get();
	pipelineStateStream.VertexShader = CD3DX12_SHADER_BYTECODE( d3dVSBlob.Get() );
	pipelineStateStream.PixelShader = CD3DX12_SHADER_BYTECODE( d3dPSBlob.Get() );
	pipelineStateStream.InputLayout = { ilds, std::size( ilds ) };
	pipelineStateStream.PrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.RTVFormats = {
		.RTFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
		.NumRenderTargets = 1u,
	};
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	const D3D12_PIPELINE_STATE_STREAM_DESC pssd{ sizeof( PipelineStateStream ), &pipelineStateStream };

	hr = d3dDevice->CreatePipelineState( &pssd, IID_PPV_ARGS( &d3dPipelineState ) );
	assert( SUCCEEDED( hr ) && "Failed to create the PSO" );
#pragma endregion


	LINFO( "Graphics started." );
}

void HCR::Graphics::Shutdown() noexcept
{
	LINFO( "Shutting Graphics..." );

	HRESULT hr = S_OK;

#pragma region Fence Signaling Event
	hr = d3dCommandQueue->Signal( d3dFence.Get(), fenceValue );
	assert( SUCCEEDED( hr ) && "Failed to signal the fence value" );
	hr = d3dFence->SetEventOnCompletion( fenceValue, fenceEvent );
	assert( SUCCEEDED( hr ) && "Failed to set event on completion" );
	[[maybe_unused]] const bool success = WaitForSingleObject( fenceEvent, 2000 ) != WAIT_FAILED;
	assert( success && "Failed to wait for the fence event (> 2 sec)" );
#pragma endregion

	LINFO( "Graphics shut." );
}

void HCR::Graphics::Render() noexcept
{
	HRESULT hr = S_OK;

	UINT curBackBufferIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
	auto& curBackBuffer = backBuffers[static_cast<INT>(curBackBufferIndex)];

	hr = d3dCommandAllocator->Reset();
	assert( SUCCEEDED( hr ) && "Failed to reset the command allocator" );
	hr = d3dCommandList->Reset( d3dCommandAllocator.Get(), nullptr );
	assert( SUCCEEDED( hr ) && "Failed to reset the command list" );

	const auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition( curBackBuffer.Get(),
																D3D12_RESOURCE_STATE_PRESENT,
																D3D12_RESOURCE_STATE_RENDER_TARGET );
	d3dCommandList->ResourceBarrier( 1u, &barrier1 );

	const auto ms = static_cast<float>(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count()) * 0.001f;

	const FLOAT clearColor[] = { std::sinf( ms ), 0.0f, 0.0f, 1.0f };

	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{
		d3dRTVDescHeap->GetCPUDescriptorHandleForHeapStart(),
		static_cast<INT>(curBackBufferIndex), d3dRTVDescHeapSize
	};

	d3dCommandList->ClearRenderTargetView( rtv, clearColor, 0u, nullptr );
	d3dCommandList->ClearDepthStencilView( dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0u, 0u, nullptr );

	d3dCommandList->SetGraphicsRootSignature( d3dRootSignature.Get() );
	d3dCommandList->SetPipelineState( d3dPipelineState.Get() );

	d3dCommandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	d3dCommandList->IASetVertexBuffers( 0u, 1u, &vertexBufferView );
	d3dCommandList->IASetIndexBuffer( &indexBufferView );

	d3dScissorRect = CD3DX12_RECT{ 0l, 0l, LONG_MAX, LONG_MAX };
	d3dViewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<FLOAT>(g_window.GetWidth()), static_cast<FLOAT>(g_window.GetHeight()) };

	d3dCommandList->RSSetScissorRects( 1u, &d3dScissorRect );
	d3dCommandList->RSSetViewports( 1u, &d3dViewport );

	d3dCommandList->OMSetRenderTargets( 1u, &rtv, TRUE, &dsvHandle );

#pragma region Constant Buffer Init
	{
		// World
		const auto world = XMMatrixRotationZ( ms ) * XMMatrixRotationY( ms ) * XMMatrixRotationX( ms );//* XMMatrixTranslation()
		// View
		const auto eyePos = XMVectorSet( 0.0f, 0.0f, -2.0f, 1.0f );
		const auto focusPos = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
		const auto upDir = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		const auto view = XMMatrixLookAtLH( eyePos, focusPos, upDir );
		// Projection
		const auto aspectRatio = static_cast<float>(g_window.GetWidth()) / static_cast<float>(g_window.GetHeight());
		const auto proj = XMMatrixPerspectiveFovLH( XMConvertToRadians( 60.0f ), aspectRatio, 0.1f, 100.0f );
		// WVP
		const auto wvp = XMMatrixTranspose( XMMatrixMultiply( world, XMMatrixMultiply( view, proj ) ) );

		d3dCommandList->SetGraphicsRoot32BitConstants( 0u, sizeof( wvp ) / 4, &wvp, 0u );
	}
#pragma endregion

	d3dCommandList->DrawIndexedInstanced( nIndices, 1u, 0u, 0u, 0u );

#pragma region Constant Buffer Init
	{
		// World
		const auto world = XMMatrixScaling( 0.5f, 0.5f, 0.5f ) * (XMMatrixRotationZ( ms * 2 ) * XMMatrixRotationY( ms * 2 ) * XMMatrixRotationX( ms * 2 )) * XMMatrixTranslation( -1.0f, 0.0f, 0.0f );
		// View
		const auto eyePos = XMVectorSet( 0.0f, 0.0f, -2.0f, 1.0f );
		const auto focusPos = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
		const auto upDir = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		const auto view = XMMatrixLookAtLH( eyePos, focusPos, upDir );
		// Projection
		const auto aspectRatio = static_cast<float>(g_window.GetWidth()) / static_cast<float>(g_window.GetHeight());
		const auto proj = XMMatrixPerspectiveFovLH( XMConvertToRadians( 60.0f ), aspectRatio, 0.1f, 100.0f );
		// WVP
		const auto wvp = XMMatrixTranspose( XMMatrixMultiply( world, XMMatrixMultiply( view, proj ) ) );

		d3dCommandList->SetGraphicsRoot32BitConstants( 0u, sizeof( wvp ) / 4, &wvp, 0u );
	}
#pragma endregion

	d3dCommandList->DrawIndexedInstanced( nIndices, 1u, 0u, 0u, 0u );

	const auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
		curBackBuffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	d3dCommandList->ResourceBarrier( 1u, &barrier2 );

	hr = d3dCommandList->Close();
	assert( SUCCEEDED( hr ) && "Failed to close the graphics command list" );
	ID3D12CommandList* const commandLists[] = { d3dCommandList.Get() };
	d3dCommandQueue->ExecuteCommandLists( std::size( commandLists ), commandLists );

	hr = dxgiSwapChain->Present( 1u, 0u );
	assert( SUCCEEDED( hr ) && "Failed to present the backbuffer" );

#pragma region Fence Signaling Event
	hr = d3dCommandQueue->Signal( d3dFence.Get(), ++fenceValue );
	assert( SUCCEEDED( hr ) && "Failed to signal the fence value" );
	hr = d3dFence->SetEventOnCompletion( fenceValue, fenceEvent );
	assert( SUCCEEDED( hr ) && "Failed to set event on completion" );
	[[maybe_unused]] const bool success = WaitForSingleObject( fenceEvent, INFINITE ) != WAIT_FAILED;
	assert( success && "Failed to wait for fence event" );
#pragma endregion

}
