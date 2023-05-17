#ifndef JITTER_BUFFER_H_HEADER
#define JITTER_BUFFER_H_HEADER

#include <deque>
#include <mutex>
#include <chrono>
#include <cmath>
#include <pack/audio_packet.h>

#define JERROR -1
#define JSUCCESS 0
#define JNODATA 1
#define JLOSTPACKET 2
#define JBUFFERING 3
#define JLATEPACKET 4
#define JDUPLICATEPACKET 5
#define JFULLSTACK 6

using pack::AudioPacket;

typedef std::chrono::high_resolution_clock Tclock;
typedef std::chrono::high_resolution_clock::time_point Ttimepoint;


namespace jbuf
{
	class JitterBuffer
	{
	private:
		
		uint32_t nominal_jitter_ms; // nominal fixed jitter size in ms;
		uint32_t max_jitter_ms; // max jitter size in ms
		uint32_t interpackets_delay_ms; // info from sender: IPD in ms
		uint32_t payload_ms_per_packet; // size in ms for every packet
		uint64_t max_payload_size_bytes; // info from sender: max size of packet payload
		uint32_t current_buffer_size_ms; // current filled buffer in ms 
		bool is_adaptive; // flag: 0 - fixed, 1 - adaptive
		bool is_circular; // if true - new packets of data will waiting while buffer is full 


		std::deque<AudioPacket> buffer; // jitter buffer for received data
		std::mutex mtx; // mutex for multi-threading access

		bool isFirst_packet = true; // refresh beginning packet id if true;
		uint32_t last_packet_id = 0; // order control

		uint32_t buffering_time_ms; // time for buffering
		Ttimepoint buffering_start; // start time-point for buffering
		bool is_buffering; 

		int64_t last_received_timestamp_ll; // jitter value: save time for last received packet 
		int64_t last_sent_timestamp_ll; // jitter value: save timestamp-value for last received packet
		int jitter_ms; // current jitter value

	


	public:
		JitterBuffer(uint32_t buffering_time_ms,uint32_t max_jitter_ms = 0, bool is_circular = true);
		~JitterBuffer();

	public:
		/* clean the jitter buffer */
		void reset_jitter_buffer();
		
		/* push packet to the queue */
		int push(AudioPacket& packet, Ttimepoint arrived_time);

		/* pop packet from the queue*/
		int pop(AudioPacket& packet);

		/* get jitter value */
		int get_jitter();
		

	private:
		/* calculate jitter after received a packet */
		void calculate_jitter(AudioPacket& new_packet, Ttimepoint arrived_time);
		
	};
}

#endif

