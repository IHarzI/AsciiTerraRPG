
// SpNet
// Networking experiment project
// IHarzI Zakhar Maslianka
// Inspired by javidx9 networking in c++ videos
// 

#pragma once
#include "spNetCommon.h"

namespace spnet {
	namespace Containers {
		template <typename ValueT, size_t ArraySize>
		struct StaticArray
		{
			ValueT InternalStorage[ArraySize];
		
			inline ValueT& operator[](size_t index) { assert(index < ArraySize); return InternalStorage[index]; }
			inline const ValueT& operator[](size_t index) const { assert(index < ArraySize); return InternalStorage[index]; }

			bool Contains(const ValueT& Value) const
			{
				for (unsigned int Index = 0; Index < size(); Index++)
				{
					if (InternalStorage[Index] == Value)
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
					if (Comparator()(InternalStorage[Index],Value))
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
					if (Comparator()(InternalStorage[Index], Value))
					{
						return InternalStorage[Index];
					}
				}
				return InternalStorage[0];
			}

			// Comparator should take (ValueT lhs, ValueT rhs) and return index, if found this valud, or InvalidID(max uint32) if not
			template<typename Comparator>
			unsigned int FindIndex(const ValueT& Value)
			{
				for (unsigned int Index = 0; Index < size(); Index++)
				{
					if (Comparator()(InternalStorage[Index], Value))
					{
						return Index;
					}
				}
				return InvalidID;
			}

			inline constexpr ValueT* data() noexcept { return InternalStorage; };
			inline constexpr const ValueT* data() const noexcept { return InternalStorage; };

			inline constexpr ValueT* begin()	const { return InternalStorage; };
			inline constexpr ValueT* end()	const { return InternalStorage + ArraySize; };

			inline constexpr ValueT* begin() { return InternalStorage; };
			inline constexpr ValueT* end() { return InternalStorage + ArraySize; };

			inline constexpr size_t size() const { return ArraySize; };
		};

		template <size_t ArraySize>
		struct FixedString
		{
			StaticArray<char, ArraySize> InternalStorage;
			FixedString() = default;
			FixedString(const char* cStr)
			{
				if (cStr)
				{
					char* charIncStr = (char*)cStr;
					uint32 i = 0;
					while (i < size())
					{
						InternalStorage[i++] = *charIncStr++;
						if (*charIncStr == '\0')
							break;
					}
				}
			}

			inline char& operator[](size_t index) { assert(index < ArraySize); return InternalStorage[index]; }
			inline const char& operator[](size_t index) const { assert(index < ArraySize); return InternalStorage[index]; }

			inline constexpr char* data() noexcept { return InternalStorage.data(); };
			inline constexpr const char* data() const noexcept { return InternalStorage.data(); };

			inline constexpr char* begin()	const { return InternalStorage.data(); };
			inline constexpr char* end()	const { return InternalStorage.data() + ArraySize; };

			inline constexpr char* begin() { return InternalStorage.data(); };
			inline constexpr char* end() { return InternalStorage.data() + ArraySize; };

			inline constexpr size_t size() const { return ArraySize; };
		};
	}
}