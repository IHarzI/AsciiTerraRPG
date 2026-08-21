#pragma once
/// -----------------------------------------------------------------------------
/// 
/// BSD 3-Clause License
/// Copyright(c) 2023-2024, (IHarzI) Maslianka Zakhar
/// 
/// -----------------------------------------------------------------------------

#include <vector>
#include <memory.h>
#include <intrin.h>

#ifdef RING_BUFFER_DEBUG
#include <cassert>
#define RING_BUFFER_ASSERT(cond) if(!(cond)) __debugbreak();
#define RING_BUFFER_REPORT(msg) std::cout << msg << '\n';
#else
#define RING_BUFFER_ASSERT(cond)
#define RING_BUFFER_REPORT(msg)
#endif


#pragma once
#define RING_BUFFER_USE_SIMPLE_ALLOCATOR

namespace spnet {
	namespace Containers {
		namespace Iterators
		{
			enum class EIndexedAccessIteratorPosition
			{
				Begin,
				End,
				InRange,
				Invalid
			};

			template<typename ContainerT, typename ValueT, typename SizeType, bool IsConstAccessOnly>
			class TIndexedIteratorBase
			{
			protected:
				SizeType Index;
				EIndexedAccessIteratorPosition Position;
				ContainerT* Container;
			public:

				TIndexedIteratorBase(const ContainerT& InContainer, SizeType StartIndex = InContainer.GetBeginIndex(),
					EIndexedAccessIteratorPosition Pos = EIndexedAccessIteratorPosition::Begin);

				TIndexedIteratorBase& operator=(const TIndexedIteratorBase& Other)
				{
					Index = Other.Index;
					Container = Other.Container;
					Position = Other.Position;
					return *this;
				};

				TIndexedIteratorBase(const TIndexedIteratorBase&) = default;

				TIndexedIteratorBase(TIndexedIteratorBase&&) = default;

				~TIndexedIteratorBase()
				{
				};

				inline const ValueT& operator* () const
				{
					RING_BUFFER_ASSERT(GetContainerRef().IsIndexValid(Index));
					return *(GetContainerRef().PointToValueAtIndex(Index));
				}

				inline const ValueT* operator->() const
				{
					RING_BUFFER_ASSERT(GetContainerRef().IsIndexValid(Index));
					return &GetContainerRef().PointToValueAtIndex(Index);
				}

				template <typename = std::enable_if<!IsConstAccessOnly>::type> ValueT& operator* ()
					//inline ValueT& operator* ()
				{
					RING_BUFFER_ASSERT(GetContainerRef().IsIndexValid(Index));
					return *(GetContainerRef().PointToValueAtIndex(Index));
				}

				template <typename = std::enable_if<!IsConstAccessOnly>::type> ValueT* operator->()
					//inline ValueT* operator->()
				{
					RING_BUFFER_ASSERT(GetContainerRef().IsIndexValid(Index));
					return &GetContainerRef().PointToValueAtIndex(Index);
				}

				void Increment()
				{
					switch (Position)
					{
					case EIndexedAccessIteratorPosition::Begin:
					{
						Position = EIndexedAccessIteratorPosition::InRange;
						Index = GetContainerRef().GetBeginIndex();
						//break;
					}
					case EIndexedAccessIteratorPosition::InRange:
					{
						Index = GetContainerRef().GetNextIndexIter(Index);
						if (Index == GetContainerRef().InvalidIndex())
							SetToEnd();
						break;
					}
					case EIndexedAccessIteratorPosition::End:
					{
						//Index = GetContainerRef().InvalidIndex();
						//Position = EIndexedAccessIteratorPosition::Invalid;
						SetToEnd();
						break;
					}
					};
				}

				void Increment(SizeType Offset)
				{
					if (!Offset)
						return;
					switch (Position)
					{
					case EIndexedAccessIteratorPosition::Begin:
					{
						Position = EIndexedAccessIteratorPosition::InRange;
						Index = GetContainerRef().GetBeginIndex();
						// continue in range scope
					}
					case EIndexedAccessIteratorPosition::InRange:
					{
						Index = GetContainerRef().GetNextIndexIter(Index, Offset);

						if (Index == GetContainerRef().InvalidIndex())
							SetToEnd();

						break;
					}
					case EIndexedAccessIteratorPosition::End:
					{
						//Index = GetContainerRef().InvalidIndex();
						//Position = EIndexedAccessIteratorPosition::Invalid;
						SetToEnd();
						break;
					}
					};
				}

				void Decrement()
				{
					switch (Position)
					{
					case EIndexedAccessIteratorPosition::Begin:
					{
						//Position = EIndexedAccessIteratorPosition::Invalid;
						//Index = GetContainerRef().InvalidIndex();
						SetToEnd(); // make iterator invalid by setting to end, as ranges use "end" for validity
						break;
					}
					case EIndexedAccessIteratorPosition::End:
					{
						Position = EIndexedAccessIteratorPosition::InRange;
						Index = GetContainerRef().GetEndIndex();
						break;
					}
					case EIndexedAccessIteratorPosition::InRange:
					{
						Index = GetContainerRef().GetPreviousIndexIter(Index);
						break;
					}
					};
				}

				void Decrement(SizeType Offset)
				{
					if (!Offset)
						return;

					switch (Position)
					{
					case EIndexedAccessIteratorPosition::Begin:
					{
						//Position = EIndexedAccessIteratorPosition::Invalid;
						//Index = GetContainerRef().InvalidIndex();
						SetToEnd();
						break;
					}
					case EIndexedAccessIteratorPosition::End:
					{
						Position = EIndexedAccessIteratorPosition::InRange;
						Index = GetContainerRef().GetEndIndex();
						// continue in range index
						Offset -= 1;

						if (!Offset)
							break;
					}
					case EIndexedAccessIteratorPosition::InRange:
					{
						Index = GetContainerRef().GetPreviousIndexIter(Index, Offset);
						if (Index == GetContainerRef().InvalidIndex())
							SetToEnd();
						break;
					}
					};
				}

				TIndexedIteratorBase& operator++()
				{
					Increment();

					//if (Index == GetContainerRef().InvalidIndex())
					//	Position = EIndexedAccessIteratorPosition::End;
					RING_BUFFER_ASSERT(*this == GetContainerRef().end() || Index != GetContainerRef().InvalidIndex());

					return *this;
				}

				TIndexedIteratorBase& operator--()
				{
					Decrement();

					//if (Index == GetContainerRef().InvalidIndex())
					//	SetToEnd();
					RING_BUFFER_ASSERT(*this == GetContainerRef().end() || Index != GetContainerRef().InvalidIndex());

					return *this;
				}

				TIndexedIteratorBase& operator--(int)
				{
					Decrement();

					//if (Index == GetContainerRef().InvalidIndex())
					//	SetToEnd();
					RING_BUFFER_ASSERT(*this == GetContainerRef().end() || Index != GetContainerRef().InvalidIndex());

					return *this;
				}

				TIndexedIteratorBase& operator++(int)
				{
					Increment();

					//if (Index == GetContainerRef().InvalidIndex())
					//	Position = EIndexedAccessIteratorPosition::End;
					RING_BUFFER_ASSERT(*this == GetContainerRef().end() || Index != GetContainerRef().InvalidIndex());

					return *this;
				}

				/** iterator arithmetic support */
				TIndexedIteratorBase& operator+=(SizeType Offset)
				{
					Increment(Offset);

					RING_BUFFER_ASSERT(*this == GetContainerRef().end() || Index != GetContainerRef().InvalidIndex());

					return *this;
				}

				TIndexedIteratorBase operator+(SizeType Offset) const
				{
					TIndexedIteratorBase Tmp(*this);
					Tmp += Offset;
					return Tmp;
				}

				TIndexedIteratorBase& operator-=(SizeType Offset)
				{
					Decrement(Offset);

					//if (Index == GetContainerRef().InvalidIndex())
					//	Position = EIndexedAccessIteratorPosition::Begin;
					RING_BUFFER_ASSERT(*this == GetContainerRef().end() || Index != GetContainerRef().InvalidIndex());

					return *this;
				}

				TIndexedIteratorBase operator-(SizeType Offset) const
				{
					TIndexedIteratorBase Tmp(*this);
					Tmp -= Offset;
					return Tmp;
				}

				/** conversion to "bool" returning true if the iterator has not reached the last element. */
				inline explicit operator bool() const
				{
					return Position == EIndexedAccessIteratorPosition::InRange && GetContainerRef().IsIndexValid(Index);
				}

				inline bool IsValidIter() const
				{
					return Position == EIndexedAccessIteratorPosition::InRange && GetContainerRef().IsIndexValid(Index) ||
						Position == EIndexedAccessIteratorPosition::Begin;
				}

				/** Returns an index to the current element. */
				SizeType GetIndex() const
				{
					return Position = EIndexedAccessIteratorPosition::InRange ? Index : GetContainerRef().InvalidIndex();
				}

				/** Resets the iterator to the first element. */
				void Reset()
				{
					auto BeginIter = GetContainerRef().begin();
					Index = BeginIter.Index;
					Position = BeginIter.Position;
				}

				/** Sets the iterator to one past the last element. */
				void SetToEnd()
				{
					auto EndIter = GetContainerRef().end();
					Index = EndIter.Index;
					Position = EndIter.Position;
				}

				inline bool operator==(const TIndexedIteratorBase& Rhs) const {
					return Container == Rhs.Container && Index == Rhs.Index && Position == Rhs.Position;
				};

				inline bool operator!=(const TIndexedIteratorBase& Rhs) const {
					return Container != Rhs.Container || Index != Rhs.Index || Position != Rhs.Position;
				};
			protected:

				template <typename = std::enable_if<!IsConstAccessOnly>::type> ContainerT& GetContainerRef()
				{
					RING_BUFFER_ASSERT(Container);
					return *Container;
				};

				const ContainerT& GetContainerRef() const
				{
					RING_BUFFER_ASSERT(Container);
					return *Container;
				};

			};

			template<typename ContainerT, typename ValueT, typename SizeType, bool IsConstAccessOnly>
			inline TIndexedIteratorBase<ContainerT, ValueT, SizeType, IsConstAccessOnly>::TIndexedIteratorBase(const ContainerT& InContainer, SizeType StartIndex, EIndexedAccessIteratorPosition Pos)
				: Container((const_cast<ContainerT*>(&InContainer)))
				, Index(StartIndex)
				, Position(Pos)
			{
			}

		};
	};
};

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

				using IndexedIterator = Iterators::TIndexedIteratorBase<RingBuffer, ValueT, size_t, false>;
				using ConstIndexedIterator = Iterators::TIndexedIteratorBase<RingBuffer, ValueT, size_t, true>;

				RingBuffer();
				RingBuffer(RingBuffer& Other);
				RingBuffer(RingBuffer&& Other);
				RingBuffer& operator=(RingBuffer& Other);
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
				IndexedIterator PeekFront();

				// Look at the first back element, don't use a pointer after pushes/emplacements elements inside the ring
				IndexedIterator PeekBack();

				// Look at the first front element, don't use a pointer after pushes/emplacements elements inside the ring
				ConstIndexedIterator PeekFront()	const;

				// Look at the first back element, don't use a pointer after pushes/emplacements elements inside the ring
				ConstIndexedIterator PeekBack()	const;

				const ValueT& Front() const { return *PeekFront(); };
				const ValueT& Back() const { return *PeekBack(); };

				ValueT& Front() { return *PeekFront(); };
				ValueT& Back() { return *PeekBack(); };

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

				bool Contains(const ValueT& Value) const
				{
					for (unsigned int Index = 0; Index < size(); Index++)
					{
						if (*PointToValueAtRingIndex(Index) == Value)
						{
							return true;
						}
					}
					return false;
				}

				// Comparator should take (ValueT lhs, ValueT rhs) and return true, if it's equal
				template<typename Comparator>
				bool Contains(const ValueT& Value) const
				{
					for (unsigned int Index = 0; Index < size(); Index++)
					{
						if (Comparator()(*PointToValueAtRingIndex(Index), Value))
						{
							return true;
						}
					}
					return false;
				}

				// Comparator should take (ValueT lhs, ValueT rhs) and return true, if it's equal
				template<typename Comparator>
				ValueT& Find(const ValueT& Value)
				{
					for (unsigned int Index = 0; Index < size(); Index++)
					{
						if (Comparator()(*PointToValueAtRingIndex(Index), Value))
						{
							return *PointToValueAtRingIndex(Index);
						}
					}
					return *MemoryBlock[0];
				}

				// Comparator should take (ValueT lhs, ValueT rhs) and return index, if found this valud, or InvalidID(max uint32) if not
				template<typename Comparator>
				unsigned int FindIndex(const ValueT& Value)
				{
					for (unsigned int Index = 0; Index < size(); Index++)
					{
						if (Comparator()(*PointToValueAtRingIndex(Index), Value))
						{
							return Index;
						}
					}
					return InvalidID;
				}

						// Stuff for convenient loop and useful operators
				inline ValueT& operator[](size_t index) { RING_BUFFER_ASSERT(index < capacity); return *PointToValueAtRingIndex(index); }
				inline const ValueT& operator[](size_t index) const { RING_BUFFER_ASSERT(index < capacity); return *PointToValueAtRingIndex(index); }

				inline ValueT& at(size_t index) { RING_BUFFER_ASSERT(index < capacity); return *PointToValueAtRingIndex(index); }
				inline const ValueT& at(size_t index) const { RING_BUFFER_ASSERT(index < capacity); return *PointToValueAtRingIndex(index); }

				// Same as GetSize, for ranges
				inline constexpr size_t size() const { return elementsInside; };

				inline constexpr ValueT* data() noexcept {
					return (ValueT*)MemoryBlock;
				};

				inline constexpr const ValueT* data() const noexcept { return (ValueT*)MemoryBlock; };

				inline const size_t InvalidIndex() const { return size_t(-1); };

				inline IndexedIterator begin()
				{
					if (elementsInside)
						return IndexedIterator{ *this, GetBeginIndex(), Iterators::EIndexedAccessIteratorPosition::Begin };

					return end();
				};

				inline IndexedIterator end()
				{
					return IndexedIterator{ *this, InvalidIndex(),Iterators::EIndexedAccessIteratorPosition::End };
				};

				inline ConstIndexedIterator begin() const {
					if (elementsInside)
						return ConstIndexedIterator{ *this, GetBeginIndex(),Iterators::EIndexedAccessIteratorPosition::Begin };

					return end();
				};

				inline ConstIndexedIterator end() const
				{
					return ConstIndexedIterator{ *this, InvalidIndex(),Iterators::EIndexedAccessIteratorPosition::End };
				};

				inline bool IsIndexValid(size_t Index) const
				{
					if (Index >= capacity ||
						elementsInside == 0 ||
						Index == InvalidIndex() ||
						Index < GetTailIndex() && Index > GetHeadIndex() ||
						Index > GetTailIndex() && Index > GetHeadIndex() && GetTailIndex() <= GetHeadIndex())
						return false;
					return true;
				};

			private:
				friend IndexedIterator;
				friend ConstIndexedIterator;

				inline size_t GetBeginIndex() const { return GetTailIndex(); };
				inline size_t GetEndIndex() const { return GetHeadIndex(); };
				inline size_t GetNextIndexIter(size_t index) const
				{
					if (index == InvalidIndex())
					{
						return InvalidIndex();
					}

					if (GetTailIndex() > GetHeadIndex())
					{
						if (index == GetCapacity() - 1)
							index = 0;
						else
							index++;
					}
					else
						index++;

					if (!IsIndexValid(index))
						return InvalidIndex();

					return index;
				};

				inline size_t GetNextIndexIter(size_t index, size_t offset) const
				{
					if (!offset)
						return index;

					if (index == InvalidIndex())
					{
						return InvalidIndex();
					}

					if (GetTailIndex() > GetHeadIndex())
					{
						if (index == GetCapacity() - 1)
						{
							if (offset - 1 > GetHeadIndex())
								return InvalidIndex();

							index = offset - 1; // 0 or 0+offset-1 items, as 0 is first item
						}
						else
						{
							if (index < GetCapacity() - 1)
							{
								size_t TailBackOffset = GetCapacity() - 1 - index;
								if (offset > TailBackOffset)
								{
									size_t BackOffset = offset - TailBackOffset;
									if (BackOffset - 1 > GetHeadIndex())
										return InvalidIndex();
									else
										index = BackOffset - 1;
								}
								else
									index += offset;
							}
							else
							{
								if (offset > GetHeadIndex())
									return InvalidIndex();

								index += offset;
							}
						};
					}
					else
					{
						if (GetTailIndex() + offset > GetHeadIndex())
							return InvalidIndex();

						index += offset;
					}

					if (!IsIndexValid(index))
						return InvalidIndex();

					return index;
				};

				inline size_t GetPreviousIndexIter(size_t index) const
				{
					if (index == InvalidIndex())
					{
						return InvalidIndex();
					}

					if (GetTailIndex() > GetHeadIndex())
					{
						if (index == 0)
							index = GetCapacity() - 1;
						else
							index--;
					}
					else
						index--;

					if (!IsIndexValid(index))
						return InvalidIndex();

					return index;
				};

				inline size_t GetPreviousIndexIter(size_t index, size_t offset) const
				{
					if (!offset)
						return index;

					if (index == InvalidIndex())
					{
						return InvalidIndex();
					}

					if (GetTailIndex() > GetHeadIndex())
					{
						if (index >= 0 && index <= GetHeadIndex())
						{
							size_t HeadBackOffset = index;
							if (offset == HeadBackOffset + 1)
								index = 0;
							else
							{
								if (offset > HeadBackOffset + 1)
								{
									if (GetCapacity() - 1 - GetTailIndex() < offset - HeadBackOffset + 1)
										return InvalidIndex();
									else
										index = GetCapacity() - (offset - HeadBackOffset + 1);
								}
								else
								{
									index -= offset;
								}
							}
						}
						else
						{
							if (offset > index - GetTailIndex())
								return InvalidIndex();
							else
							{
								index = index - offset;
							}
						}
					}
					else
					{
						if (index < offset - 1)
							return InvalidIndex();
						index -= offset;
					}

					if (!IsIndexValid(index))
						return InvalidIndex();

					return index;
				};


			private:

				ValueT* PointToValueAtIndex(size_t index);
				const ValueT* PointToValueAtIndex(size_t index) const;
				ValueT* PointToValueAtRingIndex(size_t index);
				const ValueT* PointToValueAtRingIndex(size_t index) const;
				inline ValueT** GetData() { return MemoryBlock; }
				inline const ValueT** GetData() const { return (const ValueT**)MemoryBlock; }
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
					for (auto& element : const_cast<RingBuffer&>(Other))
					{
						PushFront(element);
					}
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

				Other.MemoryBlock = nullptr;
				Other.head = InvalidIndex();
				Other.elementsInside = 0;
				Other.capacity = 0;
				Other.m_InternalAllocator = {};
			}

			template<typename ValueT, typename AllocatorT>
			inline RingBuffer<ValueT, AllocatorT>& RingBuffer<ValueT, AllocatorT>::operator=(RingBuffer& Other)
			{
				Resize(Other.capacity);
				if (Other.elementsInside > 0)
				{
					for (auto& element : const_cast<RingBuffer&>(Other))
					{
						PushFront(element);
					}
				};
				return *this;
			};

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
					for (auto& element : *this)
					{
						element.~ValueT();
					}
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
			typename RingBuffer<ValueT, AllocatorT>::IndexedIterator RingBuffer<ValueT, AllocatorT>::PeekFront()
			{
				IndexedIterator result = end();

				if (head != InvalidIndex())
					result = IndexedIterator{ *this, head , Iterators::EIndexedAccessIteratorPosition::InRange };
				return result;
			};

			template<typename ValueT, typename AllocatorT>
			typename RingBuffer<ValueT, AllocatorT>::IndexedIterator RingBuffer<ValueT, AllocatorT>::PeekBack()
			{
				IndexedIterator result = end();

				if (GetTailIndex() != InvalidIndex())
					result = IndexedIterator{ *this, GetTailIndex() , Iterators::EIndexedAccessIteratorPosition::InRange };
				return result;
			};

			template<typename ValueT, typename AllocatorT>
			typename RingBuffer<ValueT, AllocatorT>::ConstIndexedIterator RingBuffer<ValueT, AllocatorT>::PeekFront() const
			{
				ConstIndexedIterator result = end();

				if (head != InvalidIndex())
					result = ConstIndexedIterator{ *this, head , Iterators::EIndexedAccessIteratorPosition::InRange };
				return result;
			};

			template<typename ValueT, typename AllocatorT>
			typename RingBuffer<ValueT, AllocatorT>::ConstIndexedIterator RingBuffer<ValueT, AllocatorT>::PeekBack() const
			{
				ConstIndexedIterator result = end();

				if (GetTailIndex() != InvalidIndex())
					result = ConstIndexedIterator{ *this, GetTailIndex() , Iterators::EIndexedAccessIteratorPosition::InRange };
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
			inline const ValueT* RingBuffer<ValueT, AllocatorT>::PointToValueAtIndex(size_t index) const
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

			template<typename ValueT, typename AllocatorT>
			inline ValueT* RingBuffer<ValueT, AllocatorT>::PointToValueAtRingIndex(size_t index)
			{
				if (index >= capacity)
					return nullptr;

				return LookAtIndex((GetTailIndex() + index) % capacity);
			}

			template<typename ValueT, typename AllocatorT>
			inline const ValueT* RingBuffer<ValueT, AllocatorT>::PointToValueAtRingIndex(size_t index) const
			{
				if (index >= capacity)
					return nullptr;

				return LookAtIndex((GetTailIndex() + index) % capacity);
			}

		};

		template <typename T>
		using DynamicArray = RingBufferImplementation::RingBuffer<T>;
	};
};