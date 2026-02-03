// #include "arena_allocator.h"

// #include <bit>
// #include <bitset>

// LegionAllocator::LegionAllocator() {
// 	for (uint32_t& v : m_free_blocks) {
// 		v = ALL_FREE;
//     }
// 	m_longest_run = NUM_SUB_BLOCKS;
// }

// bool LegionAllocator::full() const {
// 	return m_free_blocks[0] == 0u;
// }

// bool LegionAllocator::empty() const {
// 	return m_free_blocks[0] == ALL_FREE;
// }

// uint32_t LegionAllocator::get_longest_run() const {
// 	return m_longest_run;
// }

// void LegionAllocator::allocate(uint32_t num_blocks, uint32_t& out_mask, uint32_t& out_offset) {
//     uint32_t block_mask;
//     if (num_blocks == NUM_SUB_BLOCKS) {
//     	block_mask = ~0u;
//     }
//     else {
//     	block_mask = ((1u << num_blocks) - 1u);
//     }

//     uint32_t mask = m_free_blocks[num_blocks - 1];
//     uint32_t b = std::countr_zero(mask);

//     uint32_t sb = block_mask << b;
//     m_free_blocks[0] &= ~sb;
//     update_longest_run();

//     out_mask = sb;
//     out_offset = b;
// }

// void LegionAllocator::free(uint32_t mask) {
//     m_free_blocks[0] |= mask;
//     update_longest_run();
// }

// void LegionAllocator::update_longest_run() {
//     uint32_t f = m_free_blocks[0];
//     m_longest_run = 0u;

//     while (f) {
//     	m_free_blocks[m_longest_run++] = f;
//     	f &= f >> 1;
//     }
// }

// ArenaAllocator::~ArenaAllocator() {
// 	bool error = false;

// 	if (m_heap_arena.full_heaps.size()) {
// 	    error = true;
//     }

//     for (auto &h : m_heap_arena.heaps) {
// 	    if (h.size()) {
// 		    error = true;
//         }
//     }
// }

// uint32_t floor_log2(uint32_t v) {
// 	return 31u - std::countl_zero(v);
// }

// void ArenaAllocator::set_sub_block_size(uint32_t size) {
// 	m_sub_block_size_log2 = floor_log2(size);
// 	m_sub_block_size = size;
// }

// uint32_t ArenaAllocator::get_max_allocation_size() const {
// 	return m_sub_block_size * LegionAllocator::NUM_SUB_BLOCKS;
// }

// uint32_t ArenaAllocator::get_sub_block_size() const {
// 	return m_sub_block_size;
// }

// 	uint32_t ArenaAllocator::get_block_alignment() const {
// 		return get_sub_block_size();
// 	}

// bool ArenaAllocator::allocate(uint32_t size, BackingAllocation *alloc) {
// 	unsigned num_blocks = (size + m_sub_block_size - 1) >> m_sub_block_size_log2;
// 	uint32_t size_mask = (1u << (num_blocks - 1)) - 1;
// 	uint32_t index = std::countr_zero(m_heap_arena.heap_availability_mask & ~size_mask);

// 	if (index < LegionAllocator::NUM_SUB_BLOCKS) {
// 		auto itr = m_heap_arena.heaps[index].begin();

// 		auto &heap = *itr;
// 		static_cast<DerivedAllocator *>(this)->prepare_allocation(alloc, itr, suballocate(num_blocks, heap));

// 		unsigned new_index = heap.heap.get_longest_run() - 1;

// 		if (heap.heap.full()) {
// 			m_heap_arena.full_heaps.move_to_front(heap_arena.heaps[index], itr);
// 			if (!heap_arena.heaps[index].begin())
// 				heap_arena.heap_availability_mask &= ~(1u << index);
// 		}
// 		else if (new_index != index) {
// 			auto &new_heap = m_heap_arena.heaps[new_index];
// 			new_heap.move_to_front(heap_arena.heaps[index], itr);
// 			heap_arena.heap_availability_mask |= 1u << new_index;
// 			if (!heap_arena.heaps[index].begin())
// 				heap_arena.heap_availability_mask &= ~(1u << index);
// 		}

// 		return true;
// 	}

// 	// We didn't find a vacant heap, make a new one.
// 	auto *node = m_object_pool->allocate();
// 	if (!node) {
// 		return false;
// 	}

// 	auto &heap = *node;

// 	if (!static_cast<DerivedAllocator *>(this)->allocate_backing_heap(&heap.allocation)) {
// 		m_object_pool->free(node);
// 		return false;
// 	}

// 	// This cannot fail.
// 	static_cast<DerivedAllocator *>(this)->prepare_allocation(alloc, node, suballocate(num_blocks, heap));

// 	if (heap.heap.full()) {
// 		m_heap_arena.full_heaps.insert_front(node);
// 	}
// 	else {
// 		unsigned new_index = heap.heap.get_longest_run() - 1;
// 		m_heap_arena.heaps[new_index].insert_front(node);
// 		m_heap_arena.heap_availability_mask |= 1u << new_index;
// 	}

// 	return true;
// }

// void LegionAllocator::free(typename IntrusiveList<MiniHeap>::Iterator itr, uint32_t mask) {
// 	auto *heap = itr.get();
// 	auto &block = heap->heap;
// 	bool was_full = block.full();

// 	unsigned index = block.get_longest_run() - 1;
// 	block.free(mask);
// 	unsigned new_index = block.get_longest_run() - 1;

// 	if (block.empty())
// 	{
// 		static_cast<DerivedAllocator *>(this)->free_backing_heap(&heap->allocation);

// 		if (was_full)
// 			heap_arena.full_heaps.erase(heap);
// 		else
// 		{
// 			heap_arena.heaps[index].erase(heap);
// 			if (!heap_arena.heaps[index].begin())
// 				heap_arena.heap_availability_mask &= ~(1u << index);
// 		}

// 		object_pool->free(heap);
// 	}
// 	else if (was_full)
// 	{
// 		heap_arena.heaps[new_index].move_to_front(heap_arena.full_heaps, heap);
// 		heap_arena.heap_availability_mask |= 1u << new_index;
// 	}
// 	else if (index != new_index)
// 	{
// 		heap_arena.heaps[new_index].move_to_front(heap_arena.heaps[index], heap);
// 		heap_arena.heap_availability_mask |= 1u << new_index;
// 		if (!heap_arena.heaps[index].begin())
// 			heap_arena.heap_availability_mask &= ~(1u << index);
// 	}
// }

// void LegionAllocator::set_object_pool(ObjectPool<MiniHeap> *object_pool_) {
// 	m_object_pool = object_pool_;
// }

// SuballocationResult LegionAllocator::suballocate(uint32_t num_blocks, ArenaAllocator::MiniHeap &heap) {
// 	SuballocationResult res = {};
// 	res.size = num_blocks << sub_block_size_log2;
// 	heap.heap.allocate(num_blocks, res.mask, res.offset);
// 	res.offset <<= sub_block_size_log2;
// 	return res;
// }