#pragma once
#include "spNetCommon.h"

namespace spnet
{

	template <typename T>
	struct message_header
	{
		T id{ };
		uint32_t size = 0;
	};

	template <typename T>
	struct message
	{
		message_header<T> header{};
		std::vector<uint8_t> body;
		
		// Return size of message packet in bytes
		size_t size() const
		{
			return body.size(); 
		}

		// Override for standart cout compatability - produces friendly description of message
		friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
		{
			os << "| ID: " << int(msg.header.id) << " | Size: " << msg.header.size << " | ";
			return os;
		}

		// Pushing any POD-like(pointer, int, struct, etc.) data into message buffer
		template <typename DataType>
		friend message<T>& operator << (message<T>& msg, const DataType& data)
		{
			// Check if pushed data is trivialy copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too comples to be pushed into vector");

			size_t i = msg.body.size();

			msg.body.resize(msg.body.size() +sizeof(DataType));
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

			msg.header.size = msg.size();

			return msg;


		}

		template <typename DataType>
		friend message<T>& operator >> (message<T>& msg, DataType& data)
		{
			// Check if pushed data is trivialy copyable
			static_assert(std::is_standard_layout<DataType>::value, "Data is too comples to be pushed into vector");

			size_t i = msg.body.size() - sizeof(DataType);

			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

			msg.body.resize(i);

			msg.header.size = msg.size();

			return msg;
		}

	};

	// forward declaration of connection class
	template<typename T>
	class connection;

	template<typename T>
	struct owned_message
	{
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;

		friend std::ostream& operator << (std::ostream& os, const owned_message<T>& msg)
		{

			os << msg.msg;
			return os;

		}
	};

}