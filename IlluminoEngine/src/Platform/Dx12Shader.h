#pragma once

#include "Core/Shader.h"

#include <d3d12.h>
#include <map>

namespace IlluminoEngine
{
	class Dx12Shader : public Shader
	{
	public:
		Dx12Shader(const char* filepath, const BufferLayout& layout);
		virtual ~Dx12Shader() override;

		virtual void Bind() override;

		virtual void UploadBuffer(String&& name, void* data, size_t bytes) override;

	private:
		void SetBufferLayout(const BufferLayout& layout);
		std::string ReadFile(const char* filepath);
		ID3D12Resource* GetConstantBuffer(String&& name, size_t bytes);

	private:
		String m_Filepath;
		ID3D12RootSignature* m_RootSignature;
		ID3D12PipelineState* m_PipelineState;
		std::map<String, ID3D12Resource*> m_ConstantBuffers;
	};
}
