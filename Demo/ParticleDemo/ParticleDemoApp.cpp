#include "ParticleDemoApp.hpp"

#include <iostream>
#include <random>

ParticleDemoApp::~ParticleDemoApp()
{
	mCommandQueue->waitIdle();
}

void ParticleDemoApp::update(float delta)
{
	const auto speed = 100.0f;
	const auto length = speed * delta;

	for (size_t index = 0; index < mParticles.size(); index++) {
		auto& particle = mParticles[index];
		auto& transform = mTransform[index];
		auto offset = particle.Forward * length;

		particle.reverseIfOut(offset, width(), height());

		transform = glm::translate(glm::mat4x4(1), glm::vec3(offset, 0.0f)) * transform;
	}

	const auto buffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("Transform");

	const auto memory = buffer->mapMemory();
	std::memcpy(memory, mTransform.data(), buffer->size());
	buffer->unmapMemory();
}


void ParticleDemoApp::render(float delta)
{
	const float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const auto frameBuffer = 
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuFrameBuffer>("FrameBuffer");
	const auto descriptorHeap =
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap");

	mCommandQueue->waitIdle();
	mCommandAllocator->reset();
	
	mCommandList->beginRecoding();

	mCommandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	mCommandList->setResourceLayout(mPipelineInfo->graphicsPipeline()->layout());

	mCommandList->setViewPort(frameBuffer->fullViewPort());
	mCommandList->setScissorRect(frameBuffer->fullScissorRect());

	mCommandList->setVertexBuffer(mVertexBuffer);
	mCommandList->setIndexBuffer(mIndexBuffer);

	mCommandList->setDescriptorHeap(descriptorHeap);
	
	mCommandList->beginRenderPass(
		mPipelineInfo->graphicsPipeline()->renderPass(),
		frameBuffer);
	
	mCommandList->drawIndexed(6, particleCount);
	
	mCommandList->endRenderPass();
		
	mCommandList->endRecoding();

	mCommandQueue->execute({ mCommandList });

	mSwapChain->present();

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % maxFrameResources;
}

void ParticleDemoApp::initialize()
{

#ifdef __DIRECTX12__MODE__
	//find all adapters we can use
	const auto systemInfo = std::make_shared<CodeRed::DirectX12SystemInfo>();
	const auto adapters = systemInfo->selectDisplayAdapter();

	//create device with adapter we choose
	//for this demo, we choose the second adapter
	mDevice = std::static_pointer_cast<CodeRed::GpuLogicalDevice>(
		std::make_shared<CodeRed::DirectX12LogicalDevice>(adapters[0])
		);
#else
#ifdef __VULKAN__MODE__
	//find all adapters we can use
	const auto systemInfo = std::make_shared<CodeRed::VulkanSystemInfo>();
	const auto adapters = systemInfo->selectDisplayAdapter();
	
	//create device with adapter we choose
	//for this demo, we choose the second adapter
	mDevice = std::static_pointer_cast<CodeRed::GpuLogicalDevice>(
		std::make_shared<CodeRed::VulkanLogicalDevice>(adapters[0])
		);
#endif
#endif
	
	initializeParticles();
	initializeCommands();
	initializeSwapChain();
	initializeBuffers();
	initializeShaders();
	initializeSamplers();
	initializeTextures();
	initializePipeline();
	initializeDescriptorHeaps();
}

void ParticleDemoApp::initializeParticles()
{
	std::default_random_engine random(0);

	const std::uniform_real_distribution<float> xRange(0.0f, static_cast<float>(width()));
	const std::uniform_real_distribution<float> yRange(0.0f, static_cast<float>(height()));

	const std::uniform_real_distribution<float> forwardRange(-1.0f, 1.0f);
	const std::uniform_real_distribution<float> sizeRange(10.0f, static_cast<float>(maxParticleSize));

	for (size_t index = 0; index < mParticles.size(); index++) {
		auto& particle = mParticles[index];
		auto& transform = mTransform[index];
		
		particle.Position = glm::vec2(xRange(random), yRange(random));
		particle.Forward = glm::normalize(glm::vec2(forwardRange(random), forwardRange(random)));
		particle.Size = glm::vec2(sizeRange(random));

		transform = glm::translate(glm::mat4x4(1), glm::vec3(particle.Position, 0.0f));
		transform = glm::scale(transform, glm::vec3(particle.Size, 1.0f));
	}
}

void ParticleDemoApp::initializeCommands()
{
	//create the command allocator, queue and list
	//command allocator is memory pool for command list
	//command queue is a queue to submit the command list
	//command list is a recoder of draw, copy and so on
	mCommandAllocator = mDevice->createCommandAllocator();
	mCommandQueue = mDevice->createCommandQueue();
	mCommandList = mDevice->createGraphicsCommandList(mCommandAllocator);
}

void ParticleDemoApp::initializeBuffers()
{
	mVertexBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::VertexBuffer(
			sizeof(ParticleVertex),
			4,
			CodeRed::MemoryHeap::Default
		)
	);

	mIndexBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::IndexBuffer(
			sizeof(unsigned),
			6,
			CodeRed::MemoryHeap::Default
		)
	);

	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(glm::mat4x4),
			CodeRed::MemoryHeap::Default
		)
	);

	for (auto& frameResource : mFrameResources) {
		auto buffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(glm::mat4x4),
				particleCount
			)
		);
		
		frameResource.set(
			"Transform",
			buffer
		);
	}

	std::vector<ParticleVertex> vertices(4);
	std::vector<unsigned> indices = {
		0, 1, 2,
		0, 2, 3
	};

	vertices[0] = ParticleVertex(glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
	vertices[1] = ParticleVertex(glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f));
	vertices[2] = ParticleVertex(glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
	vertices[3] = ParticleVertex(glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 0.0f));

	const auto view = glm::orthoLH_ZO(0.0f,
		static_cast<float>(width()),
		static_cast<float>(height()),
		0.0f, 0.0f, 1.0f);

	CodeRed::ResourceHelper::updateBuffer(mDevice, mCommandAllocator, mVertexBuffer, vertices.data());
	CodeRed::ResourceHelper::updateBuffer(mDevice, mCommandAllocator, mIndexBuffer, indices.data());
	CodeRed::ResourceHelper::updateBuffer(mDevice, mCommandAllocator, mViewBuffer, &view);
}

void ParticleDemoApp::initializeShaders()
{
#ifdef __DIRECTX12__MODE__
	//in DirectX 12 mode, we use HLSL and compile them with D3DCompile
	static const auto vertexShaderText =
		"#pragma pack_matrix(row_major)\n"
		"struct Transform\n"
		"{\n"
		"	matrix world;\n"
		"};\n"
		"struct output\n"
		"{\n"
		"	float4 position : SV_POSITION;\n"
		"	float2 texcoord : TEXCOORD;\n"
		"	uint id : SV_INSTANCEID;\n"
		"};\n"
		"struct Project\n"
		"{\n"
		"	matrix project;\n"
		"};\n"
		"ConstantBuffer<Project> project : register(b0);\n"
		"StructuredBuffer<Transform> transforms : register(t1);\n"
		"output main(float2 position : POSITION, float2 texcoord : TEXCOORD, uint id : SV_INSTANCEID)\n"
		"{\n"
		"	output res;\n"
		"	res.position = mul(float4(position, 0, 1), transforms[id].world);\n"
		"	res.position = mul(res.position, project.project);\n"
		"	res.texcoord = texcoord;\n"
		"	res.id = id;\n"
		"	return res;\n"
		"}\n";

	static const auto pixelShaderText =
		"Texture2D particleTexture : register(t2);\n"
		"SamplerState particleSampler : register(s3);\n"
		"float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD, uint id : SV_INSTANCEID) : SV_TARGET\n"
		"{\n"
		"	float alpha = particleTexture.Sample(particleSampler, texcoord).r;\n"
		"	if (alpha <= 0.0f) discard;\n"
		"	return float4(1, 0, 0, 1 * alpha);\n"
		"}\n";

	WRL::ComPtr<ID3DBlob> error;
	WRL::ComPtr<ID3DBlob> vertex;
	WRL::ComPtr<ID3DBlob> pixel;

	CODE_RED_THROW_IF_FAILED(
		D3DCompile(vertexShaderText,
			std::strlen(vertexShaderText),
			nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_1", D3DCOMPILE_DEBUG, 0,
			vertex.GetAddressOf(), error.GetAddressOf()),
		CodeRed::FailedException(CodeRed::DebugType::Create, { "Vertex Shader of HLSL" },
			{ CodeRed::DirectX12::charArrayToString(error->GetBufferPointer(), error->GetBufferSize()) })
	);

	CODE_RED_THROW_IF_FAILED(
		D3DCompile(pixelShaderText,
			std::strlen(pixelShaderText),
			nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "ps_5_1", D3DCOMPILE_DEBUG, 0,
			pixel.GetAddressOf(), error.GetAddressOf()),
		CodeRed::FailedException(CodeRed::DebugType::Create, { "Pixel Shader of HLSL" },
			{ CodeRed::DirectX12::charArrayToString(error->GetBufferPointer(), error->GetBufferSize()) })
	);

	mVertexShaderCode = std::vector<CodeRed::Byte>(vertex->GetBufferSize());
	mPixelShaderCode = std::vector<CodeRed::Byte>(pixel->GetBufferSize());

	std::memcpy(mVertexShaderCode.data(), vertex->GetBufferPointer(), vertex->GetBufferSize());
	std::memcpy(mPixelShaderCode.data(), pixel->GetBufferPointer(), pixel->GetBufferSize());
#else
#ifdef __VULKAN__MODE__
	static const auto vertShaderText =
		"#version 450\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"layout (set = 0, binding = 0) uniform bufferVals {\n"
		"   mat4 project;\n"
		"} myBufferVals;\n"
		"layout (set = 0, binding = 1) buffer transforms {\n"
		"	mat4 world[];\n"
		"} myTransforms;\n"
		"layout (location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec2 tex;\n"
		"layout (location = 1) out vec2 out_tex;\n"
		"void main() {\n"
		"	gl_Position = pos * transpose(myTransforms.world[gl_InstanceIndex]);\n"
		"   gl_Position = gl_Position * transpose(myBufferVals.project);\n"
		"	gl_Position.y = - gl_Position.y;\n"
		"	out_tex = tex;\n"
		"}\n";

	static const auto pixelShaderText =
		"#version 450\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"layout (location = 1) in vec2 tex;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"layout (binding = 2) uniform texture2D texture0;\n"
		"layout (binding = 3) uniform sampler sampler0;\n"
		"void main() {\n"
		"	vec4 alpha = texture(sampler2D(texture0, sampler0), tex);\n"
		"	if (alpha.r <= 0.0f) discard;\n"
		"	outColor = vec4(1, 0, 0, alpha);\n"
		"}\n";


	const auto vertex = CodeRed::ShaderCompiler::compileToSpv(CodeRed::ShaderType::Vertex, vertShaderText);
	const auto pixel = CodeRed::ShaderCompiler::compileToSpv(CodeRed::ShaderType::Pixel, pixelShaderText);

	mVertexShaderCode = std::vector<CodeRed::Byte>((vertex.end() - vertex.begin()) * sizeof(uint32_t));
	mPixelShaderCode = std::vector<CodeRed::Byte>((pixel.end() - pixel.begin()) * sizeof(uint32_t));

	std::memcpy(mVertexShaderCode.data(), &vertex.begin()[0], mVertexShaderCode.size());
	std::memcpy(mPixelShaderCode.data(), &pixel.begin()[0], mPixelShaderCode.size());
#endif
#endif
}

void ParticleDemoApp::initializeSamplers()
{
	mSampler = mDevice->createSampler(
		CodeRed::SamplerInfo(
			CodeRed::FilterOptions::MinLinearMagLinearMipLinear
		)
	);
}

void ParticleDemoApp::initializeSwapChain()
{
	//create the swap chain
	//if we want to write the back buffer(to window)
	//we need use the queue that create the swap chain to submit the draw commands
	mSwapChain = mDevice->createSwapChain(
		mCommandQueue,
		{ width(), height(), handle() },
		CodeRed::PixelFormat::BlueGreenRedAlpha8BitUnknown,
		maxFrameResources
	);

	for (size_t index = 0; index < maxFrameResources; index++) {
		mFrameResources[index].set(
			"FrameBuffer",
			mDevice->createFrameBuffer(
				mSwapChain->buffer(index),
				nullptr
			)
		);
	}
}

void ParticleDemoApp::initializeTextures()
{
	const size_t particleDetailSize = 100;
	const size_t particleDetailLevel = 400;

	mParticleTextureGenerator = std::make_shared<ParticleTextureGenerator>(
		mDevice,
		mCommandAllocator,
		particleDetailLevel,
		particleDetailSize);

	mParticleTextureGenerator->run();
	mParticleTextureGenerator->waitFinish();
}

void ParticleDemoApp::initializePipeline()
{
	//create pipeline info and pipeline factory
	mPipelineInfo = std::make_shared<CodeRed::PipelineInfo>(mDevice);
	mPipelineFactory = mDevice->createPipelineFactory();

	//we use the ParticleVertex as input format
	//so the layout is "POSITION"(0), "TEXCOORD"(1), "COLOR"(2)
	//and the format of vertex buffer is triangle list
	mPipelineInfo->setInputAssemblyState(
		mPipelineFactory->createInputAssemblyState(
			{
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreen32BitFloat),
				CodeRed::InputLayoutElement("TEXCOORD", CodeRed::PixelFormat::RedGreen32BitFloat)
			},
			CodeRed::PrimitiveTopology::TriangleList
		)
	);

	//we only use view buffer
	//so we the layout of resource only have one element
	mPipelineInfo->setResourceLayout(
		mDevice->createResourceLayout(
			{
				CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 0, 0),
				CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 1, 0),
				CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 2,0)
			},
			{ CodeRed::SamplerLayoutElement(mSampler, 3, 0) }
		)
	);

	mPipelineInfo->setVertexShaderState(
		mPipelineFactory->createShaderState(
			CodeRed::ShaderType::Vertex,
			mVertexShaderCode,
			"main"
		)
	);

	//disable depth test
	mPipelineInfo->setDepthStencilState(
		mPipelineFactory->createDetphStencilState(
			false
		)
	);

	mPipelineInfo->setRasterizationState(
		mPipelineFactory->createRasterizationState(
			CodeRed::FrontFace::Clockwise,
			CodeRed::CullMode::None
		)
	);

	mPipelineInfo->setPixelShaderState(
		mPipelineFactory->createShaderState(
			CodeRed::ShaderType::Pixel,
			mPixelShaderCode,
			"main"
		)
	);
	
	mPipelineInfo->setBlendState(
		mPipelineFactory->createBlendState(
			{
				true,
				CodeRed::BlendOperator::Add,
				CodeRed::BlendOperator::Add,
				CodeRed::BlendFactor::InvSrcAlpha,
				CodeRed::BlendFactor::InvSrcAlpha,
				CodeRed::BlendFactor::SrcAlpha,
				CodeRed::BlendFactor::SrcAlpha
			}
		)
	);

	mPipelineInfo->setRenderPass(
		mDevice->createRenderPass(
			CodeRed::Attachment::RenderTarget(mSwapChain->format())
		)
	);
	
	//update state to graphics pipeline
	mPipelineInfo->updateState();
}

void ParticleDemoApp::initializeDescriptorHeaps()
{
	for (auto& frameResource : mFrameResources) {
		auto descriptorHeap = mDevice->createDescriptorHeap(
			mPipelineInfo->graphicsPipeline()->layout()
		);

		auto buffer = frameResource.get<CodeRed::GpuBuffer>("Transform");

		descriptorHeap->bindBuffer(mViewBuffer, 0);
		descriptorHeap->bindBuffer(buffer, 1);
		descriptorHeap->bindTexture(mParticleTextureGenerator->texture(), 2);

		frameResource.set(
			"DescriptorHeap",
			descriptorHeap
		);
	}
}
