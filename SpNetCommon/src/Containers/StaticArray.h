
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