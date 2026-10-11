
#include "VulkanDevice.h"

namespace Radiant {

	class VulkanPipeline
	{
	public:
		VulkanPipeline(const std::string filepath);
		~VulkanPipeline();

		void Bind(VkCommandBuffer buffer);

	private:

		void CreatePipelineLayout();
		void CreatePipeline();

		const std::string m_Filepath;

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
		VkShaderModule m_ShaderModule;



	};

}