#include <jbuf/jitter_buffer.h>

namespace jbuf
{
	
	JitterBuffer::JitterBuffer(uint32_t buffering_time_ms, uint32_t payload_ms_per_packet, uint32_t max_jitter_ms, bool is_circular)
	{

		this->nominal_jitter_ms = buffering_time_ms * 2;
		this->is_circular = is_circular;
		// dont need!
		if(!max_jitter_ms)
		{
			/* used adaptive jitter buffer */
			this->max_jitter_ms = nominal_jitter_ms;
			is_adaptive = true;
		}
		else
		{
			/* used fixed jitter buffer */
			if (max_jitter_ms > nominal_jitter_ms)
				this->max_jitter_ms = max_jitter_ms;
			else
				this->max_jitter_ms = nominal_jitter_ms;

			is_adaptive = false;
		}

	

		this->interpackets_delay_ms = 0;
		this->max_payload_size_bytes = 0;
		this->payload_ms_per_packet = 0;
		this->buffering_time_ms = buffering_time_ms;

		if (buffering_time_ms)
			is_buffering = true;
		else
			is_buffering = false;


		buffering_start = Tclock::now();
		last_received_timestamp_ll = 0;
		last_sent_timestamp_ll = 0;
		current_buffer_size_ms = 0;
		
		jitter_ms = 0;
	}

	JitterBuffer::~JitterBuffer()
	{
		reset_remote_config();
	}


	void JitterBuffer::reset_remote_config()
	{
		std::lock_guard<std::mutex> lock(mtx);

		/*for (auto it = payload_memory_pool.begin(); it != payload_memory_pool.end(); it++)
			delete[] *it;*/

		for (auto it = buffer.begin(); it != buffer.end(); it++)
			delete[] it->data;

		//payload_memory_pool.clear();
		buffer.clear();

		interpackets_delay_ms = 0;
		payload_ms_per_packet = 0;
		max_payload_size_bytes = 0;
		current_buffer_size_ms = 0;
	}
	
	void JitterBuffer::init(uint32_t payload_ms, uint64_t max_payload_size_bytes)
	{
		// need?
		reset_remote_config();

		std::lock_guard<std::mutex> lock(mtx);

		this->payload_ms_per_packet = payload_ms;
		this->max_payload_size_bytes = max_payload_size_bytes;

		int count_ptrs = (int)std::ceil((float)max_jitter_ms / payload_ms);

		/*for (int i = 0; i < count_ptrs; i++)
		{
			payload_memory_pool.push_back(new char[max_payload_size_bytes]);
		}*/

		
	}


	int JitterBuffer::push(AudioPacket& packet, Ttimepoint arrived_time)
	{
		std::lock_guard<std::mutex> lock(mtx);

		calculate_jitter(packet,arrived_time);
		
		if (!is_circular && current_buffer_size_ms >= max_jitter_ms)
		{
			return JFULLSTACK;
		}
		else 	if (current_buffer_size_ms > max_jitter_ms) // circular overflow
		{

			current_buffer_size_ms -= buffer.front().data_in_ms;
			delete[] buffer.front().data;
			buffer.pop_front();


		}

		AudioPacket ap(packet); // copy headers info
		ap.data = new char[packet.size]; // bind data ptr to data block
		memcpy(ap.data, packet.data, packet.size); // copy payload data from arrived packet to block
		


		// 1. if received packet_id > last packet_id : add
		// 2. if received packet_id < first packet_id : drop
		// 3. if last packet_id< received packet_id > first packet_id : find place and add
		// 4. else if received packet_id == placed packet_id : drop

		if (buffer.empty() || ap.id > buffer.back().id)
		{
			buffer.push_back(ap);
			
		}
		else if (ap.id < buffer.back().id && ap.id > buffer.front().id)
		{
			for (auto it = buffer.begin(); it != buffer.end(); it++)
			{
				if (it->id > ap.id)
				{
					if ((it-1)->id < ap.id)
					{
						buffer.insert(it, ap);
						break;
					}
					delete[] ap.data;
					return JDUPLICATEPACKET;
					
				}

			}
		}
		else if (ap.id == buffer.front().id || ap.id == buffer.back().id)
		{
			delete[] ap.data;
			return JDUPLICATEPACKET;
		}
		else
		{
			delete[] ap.data;
			return JLATEPACKET;
		}
		


	


		current_buffer_size_ms += packet.data_in_ms;
		if (is_buffering && current_buffer_size_ms >= buffering_time_ms)
			is_buffering = false;

		return JSUCCESS;

	}


	int JitterBuffer::pop(AudioPacket& packet)
	{
		std::lock_guard<std::mutex> lock(mtx);

		if (is_buffering)
			return JBUFFERING;

		else if (buffer.empty())
		{
			is_buffering = true;
			return JNODATA;
		}

		AudioPacket& ap = buffer.front();
		if (ap.id != last_packet_id + 1)
		{
			last_packet_id++;
			// silence packet ?
			return JLOSTPACKET;
		}
		else
		{
			packet = ap;
			ap.data = 0;
			current_buffer_size_ms -= ap.data_in_ms;
			last_packet_id++;
			buffer.pop_front();
			return JSUCCESS;
		}


		return 0;
	}


	void JitterBuffer::calculate_jitter(AudioPacket& new_packet,Ttimepoint arrived_time)
	{
		
		int d;
		int64_t ipd;
		int64_t nrt;

		if(last_sent_timestamp_ll)
		{
			ipd = new_packet.timestamp - last_sent_timestamp_ll;
			nrt = (arrived_time.time_since_epoch().count() - last_received_timestamp_ll);
			d = (int)std::abs(ipd - nrt) / 1000000;

			jitter_ms = jitter_ms + (d - jitter_ms) / 16;
			
			// adapted jitter

			
		}

		last_received_timestamp_ll = arrived_time.time_since_epoch().count();
		last_sent_timestamp_ll = new_packet.timestamp;

	}
}