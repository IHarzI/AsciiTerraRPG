#pragma once
// SpNet
// Networking experiment project
// IHarzI Zakhar Maslianka
// Inspired by javidx9 networking in c++ videos
// 

#define RING_BUFFER_USE_SIMPLE_ALLOCATOR

// ---

/// -----------------------------------------------------------------------------
/// 
/// BSD 3-Clause License
/// Copyright(c) 2023-2024, (IHarzI) Maslianka Zakhar
/// 
/// -----------------------------------------------------------------------------


#include <vector>
#include <memory.h>

#ifdef RING_BUFFER_DEBUG
#include <cassert>
#define RING_BUFFER_ASSERT(cond) assert(cond)
#define RING_BUFFER_REPORT(msg) std::cout << msg << '\n';
#else
#define RING_BUFFER_ASSERT(cond)
#define RING_BUFFER_REPORT(msg)
#endif

#ifdef RING_BUFFER_USE_SIMPLE_ALLOCATOR
namespace spnet {
	namespace utils {

		class TEST_SIMPLE_ALLOCATOR_FOR_RING_BUFFER
		{

		public:
			using pointer = void*;
			inline pointer Allocate(size_t allocationsize)
			{
				pointer allocation = malloc(allocationsize);
				RING_BUFFER_REPORT(" SIMPLE ALLOCATOR ALLOCATED AT ADDRESS: " << allocation);
				return allocation;
			};

			inline bool Deallocate(void* allocation)
			{
				RING_BUFFER_REPORT(" SIMPLE ALLOCATOR CALL TO DEALLOCATE AT ADDRESS: " << allocation);

				free(allocation);
				return true;
			};
		};
	};
};
#endif

namespace spnet {
	namespace Containers {
		namespace RingBufferImplementation {
			namespace detail {
				inline static void* CopyMemory(void* src, void* dst, size_t size) { memcpy_s(dst, size, src, size);	return dst; };
			};

			// Ring buffer container with dynamic size. Could be used as static, if allocator is static, but resize operation will be limited 
			// by allocation memory size. Allocator Type must have following methods:
			// Allocate(size_t bytes_to_allocate), Deallocate(void* MemoryToDeallocate)
			// and be **Copy/Default Constructable**(to be able to construct/copy construct RingBuffer)
			// Value Type must be Default constructable and movable
#ifdef RING_BUFFER_USE_SIMPLE_ALLOCATOR
			template<typename ValueT, typename AllocatorT = utils::TEST_SIMPLE_ALLOCATOR_FOR_RING_BUFFER>
#else
			template<typename ValueT, typename AllocatorT>
#endif
			class RingBuffer
			{
			public:
				RingBuffer();
				RingBuffer(RingBuffer& Other);
				RingBuffer(RingBuffer&& Other);
				RingBuffer& operator=(RingBuffer& Other);
				RingBuffer& operator=(RingBuffer&& Other);
				RingBuffer(size_t capacity);
				~RingBuffer();

				size_t PushBack(ValueT value);
				size_t EmplaceBack(ValueT&& value);
				size_t PushFront(ValueT value);
				size_t EmplaceFront(ValueT&& value);

				inline void Clear()
				{
					head = InvalidIndex();
					elementsInside = 0;
				}

				// Look at the first front element, don't use a pointer after pushes/emplacements elements inside the ring
				ValueT* PeekFront();

				// Look at the first back element, don't use a pointer after pushes/emplacements elements inside the ring
				ValueT* PeekBack();

				// Look at the first front element, don't use a pointer after pushes/emplacements elements inside the ring
				const	ValueT* PeekFront()	const;

				// Look at the first back element, don't use a pointer after pushes/emplacements elements inside the ring
				const	ValueT* PeekBack()	const;

				ValueT& Front();

				ValueT& Back();

				const ValueT& Front() const;

				const ValueT& Back() const;

				// Pop element from front
				ValueT&& PopFront();

				// Pop element from back
				ValueT&& PopBack();

				// Look at this index in the container, don't use a pointer after pushes/emplacements elements inside the ring
				// NOTE: if index will be out of bounds(more that head index and less that tail index) or incorrect, return will be nullptr
				ValueT* LookAtIndex(size_t index);

				// Look at this index in the container, don't use a pointer after pushes/emplacements elements inside the ring
				// NOTE: if index will be out of bounds(more that head index and less that tail index) or incorrect, return will be nullptr
				const ValueT* LookAtIndex(size_t index) const;

				// Resize container. Could fail, if allocator couldn't allocate enough memory.
				bool Resize(size_t capacity);

				// Get capacity;
				inline size_t GetCapacity() const { return capacity; };

				// Get number of elements inside
				inline size_t GetSize() const { return elementsInside; };

				// Get head index, in case of 0 elements, result will be InvalidIndex 
				inline size_t GetHeadIndex() const { return head; };

				// Get tail index, in case of 0 elements, result will be InvalidIndex 
				size_t GetTailIndex() const;

				inline void EraseAllIfEqual(ValueT value);

				// Stuff for convenient loop and useful operators
				inline ValueT& operator[](size_t index) { RING_BUFFER_ASSERT(index < capacity); return PointToValueAtIndex(index); }
				inline const ValueT& operator[](size_t index) const { RING_BUFFER_ASSERT(index < capacity); return PointToValueAtIndex(index); }

				inline ValueT& at(size_t index) { RING_BUFFER_ASSERT(index < capacity); return PointToValueAtIndex(index); }
				inline const ValueT& at(size_t index) const { RING_BUFFER_ASSERT(index < capacity); return PointToValueAtIndex(index); }

				// Same as GetSize, for ranges
				inline constexpr size_t size() const { return elementsInside; };

				inline constexpr ValueT* data() noexcept {
					return (ValueT*)MemoryBlock;
				};
				inline constexpr const ValueT* data() const noexcept { return (ValueT*)MemoryBlock; };

				inline constexpr ValueT* begin()	const { return (ValueT*)MemoryBlock; };
				inline constexpr ValueT* end()	const { return (ValueT*)(MemoryBlock)+size(); };

				inline const size_t InvalidIndex() const { return size_t(-1); };

			private:
				ValueT* PointToValueAtIndex(size_t index);
				inline ValueT** GetData() { return MemoryBlock; }
				inline size_t GetNextHeadIndex() const;
				inline size_t GetNextTailIndex() const;
				AllocatorT m_InternalAllocator = AllocatorT{};
				ValueT** MemoryBlock;
				size_t capacity = 0;
				size_t head = InvalidIndex();
				size_t elementsInside = 0;
			};

			template<typename ValueT, typename AllocatorT>
			RingBuffer<ValueT, AllocatorT>::RingBuffer()
			{
				head = InvalidIndex();
			};

			template<typename ValueT, typename AllocatorT>
			RingBuffer<ValueT, AllocatorT>::RingBuffer(RingBuffer& Other)
			{
				Resize(Other.capacity);
				if (Other.elementsInside > 0)
				{
					detail::CopyMemory(Other.MemoryBlock, MemoryBlock, capacity);
					head = Other.head;
					elementsInside = Other.elementsInside;
				};
			}

			template<typename ValueT, typename AllocatorT>
			RingBuffer<ValueT, AllocatorT>::RingBuffer(RingBuffer&& Other)
			{
				MemoryBlock = Other.MemoryBlock;
				head = Other.head;
				elementsInside = Other.elementsInside;
				capacity = Other.capacity;
				m_InternalAllocator = Other.m_InternalAllocator;
			}

			template<typename ValueT, typename AllocatorT>
			inline RingBuffer<ValueT, AllocatorT>& RingBuffer<ValueT, AllocatorT>::operator=(RingBuffer<ValueT, AllocatorT>& Other)
			{
				if (size())
					Clear();

				if (MemoryBlock)
				{
					m_InternalAllocator.Deallocate(MemoryBlock);
				}

				Resize(Other.capacity);
				if (Other.elementsInside > 0)
				{
					detail::CopyMemory(Other.MemoryBlock, MemoryBlock, capacity);
					head = Other.head;
					elementsInside = Other.elementsInside;
				};
				return *this;
			}
			template<typename ValueT, typename AllocatorT>
			inline RingBuffer<ValueT, AllocatorT>& RingBuffer<ValueT, AllocatorT>::operator=(RingBuffer<ValueT, AllocatorT>&& Other)
			{
				if (size())
					Clear();

				if (MemoryBlock)
				{
					m_InternalAllocator.Deallocate(MemoryBlock);
				}

				MemoryBlock = Other.MemoryBlock;
				head = Other.head;
				elementsInside = Other.elementsInside;
				capacity = Other.capacity;
				m_InternalAllocator = Other.m_InternalAllocator;
				return *this;
			}
			;

			template<typename ValueT, typename AllocatorT>
			RingBuffer<ValueT, AllocatorT>::RingBuffer(size_t capacity)
			{
				if (capacity > 0 && capacity != InvalidIndex())
				{
					MemoryBlock = (ValueT**)m_InternalAllocator.Allocate(capacity * (sizeof(ValueT)));
					if (MemoryBlock)
					{
						this->capacity = capacity;
						head = InvalidIndex();
					}
				};
			};
			template<typename ValueT, typename AllocatorT>
			RingBuffer<ValueT, AllocatorT>::~RingBuffer()
			{
				if (MemoryBlock)
				{
					m_InternalAllocator.Deallocate(MemoryBlock);
				}
			};
			template<typename ValueT, typename AllocatorT>
			size_t RingBuffer<ValueT, AllocatorT>::PushBack(ValueT value)
			{
				if (MemoryBlock && capacity > elementsInside)
				{
					size_t IndexForPushedElement = 0;

					if (!(GetTailIndex() == InvalidIndex()))
					{
						IndexForPushedElement = GetNextTailIndex();
					};

					ValueT* ItemAtIndex = PointToValueAtIndex(IndexForPushedElement);

					*ItemAtIndex = value;
					elementsInside++;
					if (head == InvalidIndex())
					{
						head = IndexForPushedElement;
					};
					return IndexForPushedElement;
				}
				return InvalidIndex();
			};
			template<typename ValueT, typename AllocatorT>
			size_t RingBuffer<ValueT, AllocatorT>::EmplaceBack(ValueT&& value)
			{
				if (MemoryBlock && capacity > elementsInside)
				{
					size_t IndexForEmplacedElement = 0;

					if (!(GetTailIndex() == InvalidIndex()))
					{
						IndexForEmplacedElement = GetNextTailIndex();
					};

					std::swap(*PointToValueAtIndex(IndexForEmplacedElement), value);
					elementsInside++;

					// If it's first push/emplace of element, set head by this index
					if (head == InvalidIndex())
					{
						head = IndexForEmplacedElement;
					}

					return IndexForEmplacedElement;
				};
				return InvalidIndex();
			};

			template<typename ValueT, typename AllocatorT>
			size_t RingBuffer<ValueT, AllocatorT>::PushFront(ValueT value)
			{
				if (MemoryBlock && capacity > elementsInside)
				{
					size_t IndexForPushedElement = 0;

					if (!(head == InvalidIndex()))
					{
						IndexForPushedElement = GetNextHeadIndex();
					};

					*PointToValueAtIndex(IndexForPushedElement) = value;
					head = IndexForPushedElement;
					elementsInside++;
					return IndexForPushedElement;
				};
				return InvalidIndex();
			};

			template<typename ValueT, typename AllocatorT>
			size_t RingBuffer<ValueT, AllocatorT>::EmplaceFront(ValueT&& value)
			{
				if (MemoryBlock && capacity > elementsInside)
				{
					size_t IndexForEmplacedElement = 0;

					if (!(head == InvalidIndex()))
					{
						IndexForEmplacedElement = GetNextHeadIndex();
					};

					std::swap(*PointToValueAtIndex(IndexForEmplacedElement), value);
					head = IndexForEmplacedElement;
					elementsInside++;
					return IndexForEmplacedElement;
				};
				return InvalidIndex();
			};

			template<typename ValueT, typename AllocatorT>
			ValueT* RingBuffer<ValueT, AllocatorT>::PeekFront()
			{
				ValueT* result = nullptr;

				if (head != InvalidIndex())
					result = PointToValueAtIndex(head);
				return result;
			};

			template<typename ValueT, typename AllocatorT>
			ValueT* RingBuffer<ValueT, AllocatorT>::PeekBack()
			{
				ValueT* result = nullptr;

				if (GetTailIndex() != InvalidIndex())
					result = PointToValueAtIndex(GetTailIndex());
				return result;
			};

			template<typename ValueT, typename AllocatorT>
			ValueT& RingBuffer<ValueT, AllocatorT>::Front()
			{
				return *PeekBack();
			};

			template<typename ValueT, typename AllocatorT>
			ValueT& RingBuffer<ValueT, AllocatorT>::Back()
			{
				return *PeekFront();
			};

			template<typename ValueT, typename AllocatorT>
			const ValueT& RingBuffer<ValueT, AllocatorT>::Front() const
			{
				return *PeekBack();
			};

			template<typename ValueT, typename AllocatorT>
			const ValueT& RingBuffer<ValueT, AllocatorT>::Back() const
			{
				return *PeekFront();
			};

			template<typename ValueT, typename AllocatorT>
			const ValueT* RingBuffer<ValueT, AllocatorT>::PeekFront() const
			{
				ValueT* result = nullptr;

				if (head != InvalidIndex())
					result = PointToValueAtIndex(head);
				return result;
			};

			template<typename ValueT, typename AllocatorT>
			const ValueT* RingBuffer<ValueT, AllocatorT>::PeekBack() const
			{
				ValueT* result = nullptr;

				if (GetTailIndex() != InvalidIndex())
					result = PointToValueAtIndex(GetTailIndex());
				return result;
			};

			template<typename ValueT, typename AllocatorT>
			ValueT&& RingBuffer<ValueT, AllocatorT>::PopFront()
			{
				ValueT* Result = nullptr;
				if (head != InvalidIndex())
				{
					Result = PointToValueAtIndex(head);
					if (elementsInside > 1)
					{
						elementsInside--;
						// Avoid "underflow?" if head == 0
						if (head == 0)
							head = capacity - 1;
						else
							head = head - 1 % capacity;
					}
					else
					{
						head = InvalidIndex();
						elementsInside = 0;
					};
				}
				if (Result)
				{
					return std::move(*Result);
				};

				return ValueT{};
			};

			template<typename ValueT, typename AllocatorT>
			ValueT&& RingBuffer<ValueT, AllocatorT>::PopBack()
			{
				ValueT* Result = nullptr;
				if (GetTailIndex() != InvalidIndex())
				{
					Result = PointToValueAtIndex(GetTailIndex());
					if (elementsInside > 1)
					{
						elementsInside--;
					}
					else
					{
						head = InvalidIndex();
						elementsInside = 0;
					}
				};

				if (Result)
				{
					return std::move(*Result);
				};

				return ValueT{};
			};

			template<typename ValueT, typename AllocatorT>
			ValueT* RingBuffer<ValueT, AllocatorT>::LookAtIndex(size_t index)
			{
				if (index >= capacity ||
					elementsInside == 0 ||
					index == InvalidIndex() ||
					index < GetTailIndex() && index > GetHeadIndex() ||
					index > GetTailIndex() && index > GetHeadIndex() && GetTailIndex() <= GetHeadIndex())
					return nullptr;
				return (ValueT*)GetData() + index;
			};

			template<typename ValueT, typename AllocatorT>
			const ValueT* RingBuffer<ValueT, AllocatorT>::LookAtIndex(size_t index) const
			{
				if (index >= capacity ||
					elementsInside == 0 ||
					index == InvalidIndex() ||
					index < GetTailIndex() && index > GetHeadIndex() ||
					index > GetTailIndex() && index > GetHeadIndex() && GetTailIndex() <= GetHeadIndex())
					return nullptr;
				return (ValueT*)GetData() + index;
			};

			template<typename ValueT, typename AllocatorT>
			bool RingBuffer<ValueT, AllocatorT>::Resize(size_t NewCapacity)
			{
				if (NewCapacity > 0 && NewCapacity != size_t(-1) && NewCapacity >= elementsInside)
				{
					ValueT** NewAllocatedMemory = (ValueT**)m_InternalAllocator.Allocate(NewCapacity * sizeof(ValueT));
					if (NewAllocatedMemory)
					{
						if (MemoryBlock)
						{
							if (elementsInside > 0 && head != InvalidIndex())
							{
								size_t TailIndex = 0;

								if (GetTailIndex() != InvalidIndex())
									TailIndex = GetTailIndex();

								if (TailIndex > head)
								{
									// Just copy value in loop
									size_t StartIndexForTailPart = 0;
									for (size_t copyIndex = TailIndex; copyIndex < capacity; copyIndex++)
									{
										*((ValueT*)NewAllocatedMemory + StartIndexForTailPart++) = *((ValueT*)MemoryBlock + copyIndex);
									};
									for (size_t copyIndex = 0; copyIndex <= head; copyIndex++)
									{
										*((ValueT*)NewAllocatedMemory + StartIndexForTailPart++) = *((ValueT*)MemoryBlock + copyIndex);
									};

									// Update info about container
									head = elementsInside - 1;
								}
								else
								{
									// copy all elements into new container
									detail::CopyMemory(GetData(), NewAllocatedMemory, head + 1 * sizeof(ValueT));
								}
							};
							m_InternalAllocator.Deallocate(GetData());
						};
						capacity = NewCapacity;
						MemoryBlock = NewAllocatedMemory;
						return true;
					};
				};
				return false;
			};

			template<typename ValueT, typename AllocatorT>
			size_t RingBuffer<ValueT, AllocatorT>::GetTailIndex() const
			{
				if (capacity == 0)
				{
					return InvalidIndex();
				};

				if (head == InvalidIndex())
				{
					return InvalidIndex();
				};

				RING_BUFFER_ASSERT(elementsInside > 0);

				if (elementsInside == 1)
				{
					return head;
				};

				return head < elementsInside - 1 ? capacity - (elementsInside - head - 1) : head - (elementsInside - 1);
			};

			template<typename ValueT, typename AllocatorT>
			inline ValueT* RingBuffer<ValueT, AllocatorT>::PointToValueAtIndex(size_t index)
			{
				if (index >= capacity)
					return nullptr;

				return (ValueT*)GetData() + index;
			}

			template<typename ValueT, typename AllocatorT>
			inline size_t RingBuffer<ValueT, AllocatorT>::GetNextHeadIndex() const
			{
				if (capacity == 0 || capacity == elementsInside)
				{
					return InvalidIndex();
				}
				size_t NextIndex = head == capacity - 1 ? 0 : head + 1;
				return NextIndex;
			};

			template<typename ValueT, typename AllocatorT>
			inline size_t RingBuffer<ValueT, AllocatorT>::GetNextTailIndex() const
			{
				if (capacity == 0 || capacity == elementsInside)
				{
					return InvalidIndex();
				}
				size_t NextIndex = GetTailIndex() == 0 ? capacity - 1 : GetTailIndex() - 1;
				return NextIndex;
			};

		};
		template <typename T>
		using DynamicArray = RingBufferImplementation::RingBuffer<T>;
	};
};