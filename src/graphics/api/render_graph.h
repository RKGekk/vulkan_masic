#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_command_pool_type.h"

#include <memory>
#include <string>

class VulkanDevice;

class RenderPassExternalLockInterface {
public:
	virtual ~RenderPassExternalLockInterface() = default;

	virtual const char *getID() const;
	virtual PoolTypeEnum QueueType() const = 0;

	// If consume queue is different from producer.
	Vulkan::Semaphore external_acquire_semaphore(Vulkan::CommandBuffer::Type type);
	void external_release_semaphore(Vulkan::Semaphore semaphore);

	// If there is a dependency that wants to access the resource on foreign queue, we need.
	// External accesses must be read-only.
	void mark_access_in_queue(Vulkan::CommandBuffer::Type type, VkPipelineStageFlags2 stages,
	                          VkAccessFlags2 access);

protected:
	// Derived class needs to call these.
	// Allocate command buffer and applies sync as necessary.
	Vulkan::CommandBufferHandle acquire_internal(Vulkan::Device &device, VkPipelineStageFlags2 stages,
	                                             VkAccessFlags2 access);

	// Submits work to GPU, adds barriers and/or signals semaphores as necessary.
	void release_internal(Vulkan::CommandBufferHandle &cmd, VkPipelineStageFlags2 stages, VkAccessFlags2 access);

	void device_reset();

private:
	struct StageAccessPair
	{
		VkPipelineStageFlags2 stages;
		VkAccessFlags2 access;
	};

	StageAccessPair inline_queue_invalidate = {};
	StageAccessPair inline_queue_flush = {};

	bool required_foreign_queue_access = false;
	Vulkan::Semaphore acquire_semaphore;
	std::mutex lock;
	Util::SmallVector<Vulkan::Semaphore> release_semaphores;
};

class RenderGraph {
public:
	bool init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name);
    void destroy();
    void reset();

	void add_external_lock_interface(const std::string &name, RenderPassExternalLockInterface *iface);
	RenderPassExternalLockInterface *find_external_lock_interface(const std::string &name) const;

	RenderPass &add_pass(const std::string &name, RenderGraphQueueFlagBits queue);
	RenderPass *find_pass(const std::string &name);
	void set_backbuffer_source(const std::string &name);
	void set_backbuffer_dimensions(const ResourceDimensions &dim)
	{
		swapchain_dimensions = dim;
	}

	const ResourceDimensions &get_backbuffer_dimensions() const
	{
		return swapchain_dimensions;
	}

	ResourceDimensions get_resource_dimensions(const RenderBufferResource &resource) const;
	ResourceDimensions get_resource_dimensions(const RenderTextureResource &resource) const;

	void enable_timestamps(bool enable);

	void bake();
	
	void log();
	void setup_attachments(Vulkan::Device &device, Vulkan::ImageView *swapchain);
	void enqueue_render_passes(Vulkan::Device &device, TaskComposer &composer);

	RenderTextureResource &get_texture_resource(const std::string &name);
	RenderBufferResource &get_buffer_resource(const std::string &name);
	RenderResource &get_proxy_resource(const std::string &name);

	Vulkan::ImageView &get_physical_texture_resource(unsigned index)
	{
		assert(index != RenderResource::Unused);
		assert(physical_attachments[index]);
		return *physical_attachments[index];
	}

	Vulkan::ImageView *get_physical_history_texture_resource(unsigned index)
	{
		assert(index != RenderResource::Unused);
		if (!physical_history_image_attachments[index])
			return nullptr;
		return &physical_history_image_attachments[index]->get_view();
	}

	Vulkan::Buffer &get_physical_buffer_resource(unsigned index)
	{
		assert(index != RenderResource::Unused);
		assert(physical_buffers[index]);
		return *physical_buffers[index];
	}

	Vulkan::ImageView &get_physical_texture_resource(const RenderTextureResource &resource)
	{
		assert(resource.get_physical_index() != RenderResource::Unused);
		return get_physical_texture_resource(resource.get_physical_index());
	}

	Vulkan::ImageView *maybe_get_physical_texture_resource(RenderTextureResource *resource)
	{
		if (resource && resource->get_physical_index() != RenderResource::Unused)
			return &get_physical_texture_resource(*resource);
		else
			return nullptr;
	}

	Vulkan::ImageView *get_physical_history_texture_resource(const RenderTextureResource &resource)
	{
		return get_physical_history_texture_resource(resource.get_physical_index());
	}

	Vulkan::Buffer &get_physical_buffer_resource(const RenderBufferResource &resource)
	{
		assert(resource.get_physical_index() != RenderResource::Unused);
		return get_physical_buffer_resource(resource.get_physical_index());
	}

	Vulkan::Buffer *maybe_get_physical_buffer_resource(RenderBufferResource *resource)
	{
		if (resource && resource->get_physical_index() != RenderResource::Unused)
			return &get_physical_buffer_resource(*resource);
		else
			return nullptr;
	}

	// For keeping feed-back resources alive during rebaking.
	Vulkan::BufferHandle consume_persistent_physical_buffer_resource(unsigned index) const;
	void install_persistent_physical_buffer_resource(unsigned index, Vulkan::BufferHandle buffer);

	// Utility to consume all physical buffer handles and install them.
	std::vector<Vulkan::BufferHandle> consume_physical_buffers() const;
	void install_physical_buffers(std::vector<Vulkan::BufferHandle> buffers);

	static RenderGraphQueueFlagBits get_default_post_graphics_queue()
	{
		return RENDER_GRAPH_QUEUE_GRAPHICS_BIT;
	}

	static RenderGraphQueueFlagBits get_default_compute_queue()
	{
		// Don't use async compute by default due to submit overhead.
		return RENDER_GRAPH_QUEUE_COMPUTE_BIT;
	}

private:
	Vulkan::Device *device = nullptr;
	std::vector<std::unique_ptr<RenderPass>> passes;
	std::vector<std::unique_ptr<RenderResource>> resources;
	std::unordered_map<std::string, unsigned> pass_to_index;
	std::unordered_map<std::string, unsigned> resource_to_index;
	std::unordered_map<std::string, RenderPassExternalLockInterface *> external_lock_interfaces;
	std::string backbuffer_source;

	std::vector<unsigned> pass_stack;

	struct Barrier
	{
		unsigned resource_index;
		VkImageLayout layout;
		VkAccessFlags2 access;
		VkPipelineStageFlags2 stages;
		bool history;
	};

	struct Barriers
	{
		std::vector<Barrier> invalidate;
		std::vector<Barrier> flush;
	};

	std::vector<Barriers> pass_barriers;

	void filter_passes(std::vector<unsigned> &list);
	void validate_passes();
	void build_barriers();

	ResourceDimensions swapchain_dimensions;

	struct ColorClearRequest
	{
		RenderPass *pass;
		VkClearColorValue *target;
		unsigned index;
	};

	struct DepthClearRequest
	{
		RenderPass *pass;
		VkClearDepthStencilValue *target;
	};

	struct ScaledClearRequests
	{
		unsigned target;
		unsigned physical_resource;
	};

	struct MipmapRequests
	{
		unsigned physical_resource;
		VkPipelineStageFlags2 stages;
		VkAccessFlags2 access;
		VkImageLayout layout;
	};

	struct PhysicalPass
	{
		std::vector<unsigned> passes;
		std::vector<unsigned> discards;
		std::vector<Barrier> invalidate;
		std::vector<Barrier> flush;
		std::vector<Barrier> history;
		std::vector<std::pair<unsigned, unsigned>> alias_transfer;

		Vulkan::RenderPassInfo render_pass_info;
		std::vector<Vulkan::RenderPassInfo::Subpass> subpasses;
		std::vector<unsigned> physical_color_attachments;
		unsigned physical_depth_stencil_attachment = RenderResource::Unused;

		std::vector<ColorClearRequest> color_clear_requests;
		DepthClearRequest depth_clear_request = {};

		std::vector<std::vector<ScaledClearRequests>> scaled_clear_requests;
		std::vector<MipmapRequests> mipmap_requests;
		unsigned layers = 1;
	};
	std::vector<PhysicalPass> physical_passes;
	std::vector<unsigned> early_discards;
	void build_physical_passes();
	void build_transients();
	void build_physical_resources();
	void build_physical_barriers();
	void build_render_pass_info();
	void build_aliases();

	bool enabled_timestamps = false;

	std::vector<ResourceDimensions> physical_dimensions;
	std::vector<Vulkan::ImageView *> physical_attachments;
	std::vector<Vulkan::BufferHandle> physical_buffers;
	std::vector<Vulkan::ImageHandle> physical_image_attachments;
	std::vector<Vulkan::ImageHandle> physical_history_image_attachments;

	struct PipelineEvent
	{
		VkPipelineStageFlags2 pipeline_barrier_src_stages = 0;

		// Need two separate semaphores so we can wait in both queues independently.
		// Waiting for a semaphore resets it.
		Vulkan::Semaphore wait_graphics_semaphore;
		Vulkan::Semaphore wait_compute_semaphore;

		// Stages to wait for are stored inside the events.
		VkAccessFlags2 to_flush_access = 0;
		VkAccessFlags2 invalidated_in_stage[64] = {};
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

		bool locked_invalidation = false;
	};

	std::vector<PipelineEvent> physical_events;
	std::vector<PipelineEvent> physical_history_events;
	std::vector<bool> physical_image_has_history;
	std::vector<unsigned> physical_aliases;

	Vulkan::ImageView *swapchain_attachment = nullptr;
	unsigned swapchain_physical_index = RenderResource::Unused;

	void enqueue_scaled_requests(Vulkan::CommandBuffer &cmd, const std::vector<ScaledClearRequests> &requests);
	void enqueue_mipmap_requests(Vulkan::CommandBuffer &cmd, const std::vector<MipmapRequests> &requests);

	void on_swapchain_changed(const Vulkan::SwapchainParameterEvent &e);
	void on_swapchain_destroyed(const Vulkan::SwapchainParameterEvent &e);
	void on_device_created(const Vulkan::DeviceCreatedEvent &e);
	void on_device_destroyed(const Vulkan::DeviceCreatedEvent &e);

	void setup_physical_buffer(Vulkan::Device &device, unsigned attachment);
	void setup_physical_image(Vulkan::Device &device, unsigned attachment);

	void depend_passes_recursive(const RenderPass &pass, const std::unordered_set<unsigned> &passes,
	                             unsigned stack_count, bool no_check, bool ignore_self, bool merge_dependency);

	void traverse_dependencies(const RenderPass &pass, unsigned stack_count);

	std::vector<std::unordered_set<unsigned>> pass_dependencies;
	std::vector<std::unordered_set<unsigned>> pass_merge_dependencies;
	bool depends_on_pass(unsigned dst_pass, unsigned src_pass);

	void reorder_passes(std::vector<unsigned> &passes);
	static bool need_invalidate(const Barrier &barrier, const PipelineEvent &event);

	struct PassSubmissionState
	{
		Util::SmallVector<VkMemoryBarrier2> global_barriers;
		Util::SmallVector<VkBufferMemoryBarrier2> buffer_barriers;
		Util::SmallVector<VkImageMemoryBarrier2> image_barriers;

		Util::SmallVector<VkSubpassContents> subpass_contents;

		Util::SmallVector<Vulkan::Semaphore> wait_semaphores;
		Util::SmallVector<VkPipelineStageFlags2> wait_semaphore_stages;

		Util::SmallVector<RenderPass::AccessedExternalLockInterface> external_locks;

		Vulkan::Semaphore proxy_semaphores[2];
		bool need_submission_semaphore = false;

		Vulkan::CommandBufferHandle cmd;

		Vulkan::CommandBuffer::Type queue_type = Vulkan::CommandBuffer::Type::Count;
		bool graphics = false;
		bool active = false;

		TaskGroupHandle rendering_dependency;

		void emit_pre_pass_barriers();
		void submit();
	};
	std::vector<PassSubmissionState> pass_submission_state;

	void enqueue_render_pass(Vulkan::Device &device, unsigned physical_pass_index, TaskComposer &composer);
	void enqueue_swapchain_scale_pass(Vulkan::Device &device);
	bool physical_pass_requires_work(const PhysicalPass &pass) const;
	void physical_pass_transfer_ownership(const PhysicalPass &pass);
	void physical_pass_invalidate_attachments(const PhysicalPass &pass);
	void physical_pass_invalidate_attachments_early();

	void physical_pass_enqueue_graphics_commands(const PhysicalPass &pass, PassSubmissionState &state);
	void physical_pass_enqueue_compute_commands(const PhysicalPass &pass, PassSubmissionState &state);

	void physical_pass_handle_invalidate_barrier(const Barrier &barrier, PassSubmissionState &state, bool physical_graphics_queue);

	void physical_pass_handle_invalidate_barrier_lookahead(PassSubmissionState &state,
	                                                       bool physical_graphics,
	                                                       const PhysicalPass &future);

	void physical_pass_handle_external_acquire(const PhysicalPass &pass, PassSubmissionState &state);
	void physical_pass_handle_signal(Vulkan::Device &device, const PhysicalPass &pass, PassSubmissionState &state);
	void physical_pass_handle_flush_barrier(const Barrier &barrier, PassSubmissionState &state);
	void physical_pass_handle_cpu_timeline(Vulkan::Device &device, unsigned physical_pass_index, TaskComposer &composer);
	void physical_pass_handle_gpu_timeline(ThreadGroup &group, Vulkan::Device &device,
	                                       const PhysicalPass &pass, PassSubmissionState &state);
};