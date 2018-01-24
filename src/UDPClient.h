#ifndef UDP_CLIENT
#define UDP_CLIENT

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "DroneState.h"
#include "crc16.h"



using boost::asio::ip::udp;

class UDPClient
{
public:
	UDPClient(
		const std::string& host, 
		const std::string& port
	) : io_service_(io_service_udp), socket_(io_service_udp, udp::endpoint(udp::v4(), 0)) {
		udp::resolver resolver(io_service_);
		udp::resolver::query query(udp::v4(), host, port);
		udp::resolver::iterator iter = resolver.resolve(query);
		endpoint_ = *iter;
	}

	~UDPClient()
	{
		socket_.close();
	}

	void send(boost::asio::mutable_buffers_1 msg) {
		std::size_t bytes_send = socket_.send_to(msg, endpoint_);
		std::cout << "UDP bytes send: " << bytes_send << std::endl;
	}
	
	boost::asio::mutable_buffers_1 serialize(DroneState state) {
		FlatState* data = new FlatState;
		data->timestamp = state.timestamp;
		data->id = state.id;
		data->x = state.pos.x;
		data->y = state.pos.y;
		data->psi = state.psi;

		// Add CRC checksum
		uint8_t * data_arr = (uint8_t *)data;
		data->crc16 = ::crc_16(data_arr, sizeof(FlatState) - 2);

		return boost::asio::buffer((void *) data, sizeof(FlatState));
	}
	
	void send_state(DroneState state) {
		boost::asio::mutable_buffers_1 buf = serialize(state);
	        send(buf);
		free(boost::asio::buffer_cast<void *>(buf));
    }

private:
	boost::asio::io_service io_service_udp;
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	udp::endpoint endpoint_;
	
	struct __attribute__ ((packed)) FlatState {
		uint64_t timestamp;
		double id;
		double x;
		double y;
		double psi;
		uint16_t crc16; // This field must always be the last one. Otherwise serialize is not working properly.
	};
};

#endif
