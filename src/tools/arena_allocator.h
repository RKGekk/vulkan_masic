// #pragma once

// #include <array>
// #include <cstdint>
// #include <list>
// #include <memory>

// class LegionAllocator {
// public:
// 	// 1 free
// 	// 0 occupied
// 	static const uint32_t NUM_SUB_BLOCKS = 32u;
// 	static const uint32_t ALL_FREE = ~0u;

// 	LegionAllocator(const LegionAllocator&) = delete;
// 	void operator=(const LegionAllocator&) = delete;

// 	LegionAllocator();

// 	bool full() const;
// 	bool empty() const;
// 	uint32_t get_longest_run() const;
// 	void allocate(uint32_t num_blocks, uint32_t& out_mask, uint32_t& out_offset);
// 	void free(uint32_t mask);

// private:
//     void update_longest_run();

// 	std::array<uint32_t, NUM_SUB_BLOCKS> m_free_blocks;
// 	uint32_t m_longest_run = 0u;
// };

// class BackingAllocation {};

// class LegionHeap {
// public:
// 	BackingAllocation allocation;
// 	LegionAllocator heap;
// 	std::shared_ptr<LegionHeap> prev = nullptr;
// 	std::shared_ptr<LegionHeap> next = nullptr;
// };

// struct AllocationArena {
// public:
// 	std::array<std::list<LegionHeap>, LegionAllocator::NUM_SUB_BLOCKS> heaps;
// 	std::list<LegionHeap> full_heaps;
// 	uint32_t heap_availability_mask = 0;
// };

// struct SuballocationResult {
// 	uint32_t offset;
// 	uint32_t size;
// 	uint32_t mask;
// };

// class ArenaAllocator {
// public:
// 	using MiniHeap = LegionHeap;

// 	~ArenaAllocator();

// 	void set_sub_block_size(uint32_t size);
// 	uint32_t get_max_allocation_size() const;
// 	uint32_t get_sub_block_size() const;
// 	uint32_t get_block_alignment() const;
// 	bool allocate(uint32_t size, BackingAllocation *alloc);
// 	void free(std::shared_ptr<LegionHeap> itr, uint32_t mask);
// 	void set_object_pool(std::shared_ptr<LegionHeap> object_pool);

// protected:
// 	AllocationArena m_heap_arena;
// 	std::shared_ptr<LegionHeap> m_object_pool = nullptr;

// 	uint32_t m_sub_block_size = 1u;
// 	uint32_t m_sub_block_size_log2 = 0u;

// private:
// 	SuballocationResult suballocate(uint32_t num_blocks, MiniHeap &heap);
// };