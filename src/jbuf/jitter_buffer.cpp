#include <jbuf/jitter_buffer.h>

namespace jbuf
{

	JitterBuffer::JitterBuffer(uint32_t buffering_time_ms, uint32_t payload_ms_per_packet, uint32_t max_jitter_ms, bool is_circular)
	{

		this->nominal_jitter_ms = buffering_time_ms * 2;
		this->is_circular = is_circular;

		// dont need!
		if (!max_jitter_ms)
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
		reset_jitter_buffer();
	}


	void JitterBuffer::reset_jitter_buffer()
	{
		std::lock_guard<std::mutex> lock(mtx);


		for (auto it = buffer.begin(); it != buffer.end(); it++)
			delete[] it->data;

	
		buffer.clear();
		isFirst_packet = true;
		last_packet_id = 0;

		interpackets_delay_ms = 0;
		payload_ms_per_packet = 0;
		max_payload_size_bytes = 0;
		current_buffer_size_ms = 0;
	}

	

	int JitterBuffer::push(AudioPacket& packet, Ttimepoint arrived_time)
	{
		std::lock_guard<std::mutex> lock(mtx);

		calculate_jitter(packet, arrived_time);

		printf("cur: %u\n", current_buffer_size_ms);
		if (!is_circular)
		{
			return JFULLSTACK;
		}

		else if (current_buffer_size_ms >= max_jitter_ms) // circular overflow
		{
			printf("OVERFLOW\n");
			last_packet_id++;
			current_buffer_size_ms -= buffer.front().data_in_ms;
			delete[] buffer.front().data;
			buffer.pop_front();


		}


		AudioPacket ap(packet); // copy headers info
		ap.data = new char[packet.size]; // bind pointer to data-block
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
					if ((it - 1)->id < ap.id)
					{
						buffer.insert(it, ap);
						break;
					}
					delete[] ap.data;
					printf("DUP\n");
					return JDUPLICATEPACKET;

				}

			}
		}
		else if (ap.id == buffer.front().id || ap.id == buffer.back().id)
		{
			delete[] ap.data;
			printf("DUP\n");
			return JDUPLICATEPACKET;
		}
		else
		{
			delete[] ap.data;
			printf("LATEPACKET\n");
			return JLATEPACKET;
		}




		//printf("IS BUFFERING : %i\n", is_buffering);
		current_buffer_size_ms += packet.data_in_ms;
		if (is_buffering && current_buffer_size_ms >= buffering_time_ms)
			is_buffering = false;

		
		return JSUCCESS;

	}


	int JitterBuffer::pop(AudioPacket& packet)
	{
		std::lock_guard<std::mutex> lock(mtx);



		if (is_buffering)
		{
			return JBUFFERING;
		}


		else if (buffer.empty())
		{
			is_buffering = true;
			return JNODATA;
		}

		AudioPacket& ap = buffer.front();

		if (isFirst_packet)
		{
			isFirst_packet = false;
			last_packet_id = ap.id-1;
		

		}

		if (ap.id != last_packet_id + 1)
		{
			last_packet_id++;
			return JLOSTPACKET;
		}

		packet = ap;
		ap.data = 0;
		current_buffer_size_ms -= ap.data_in_ms;
		last_packet_id++;
		buffer.pop_front();

		return JSUCCESS;


	}


	void JitterBuffer::calculate_jitter(AudioPacket& new_packet, Ttimepoint arrived_time)
	{

		float d;
		int64_t ipd;
		int64_t nrt;


		if (last_sent_timestamp_ll)
		{
			ipd = new_packet.timestamp - last_sent_timestamp_ll;
			nrt = (arrived_time.time_since_epoch().count() - last_received_timestamp_ll);
			d = (nrt - ipd) / (float)1000000;

			jitter_ms = jitter_ms + (std::abs(d) - jitter_ms) / 16;

			// adapt jitter

			//printf("JITTER: %f\n", jitter_ms);

		}

		last_received_timestamp_ll = arrived_time.time_since_epoch().count();
		last_sent_timestamp_ll = new_packet.timestamp;

	}
}