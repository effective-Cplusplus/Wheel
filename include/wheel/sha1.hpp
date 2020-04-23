#ifndef sha1_h__
#define sha1_h__

namespace wheel {

	class sha1
	{
	public:
		sha1(){
			reset();
		}

		virtual ~sha1(){
			// The destructor does nothing
		}

		void reset(){
			length_low_ = 0;
			length_high_ = 0;
			message_block_Index_ = 0;

			h_[0] = 0x67452301;
			h_[1] = 0xEFCDAB89;
			h_[2] = 0x98BADCFE;
			h_[3] = 0x10325476;
			h_[4] = 0xC3D2E1F0;

			computed_ = false;
			corrupted_ = false;
		}

		bool result(unsigned* message_digest_array){
			if (corrupted_){
				return false;
			}

			if (!computed_){
				pad_message();
				computed_ = true;
			}

			for (int i = 0; i < 5; i++){
				message_digest_array[i] = h_[i];
			}

			return true;
		}

		void input(const unsigned char* message_array,
			unsigned            length){
			if (!length){
				return;
			}

			if (computed_ || corrupted_){
				corrupted_ = true;
				return;
			}

			while (length-- && !corrupted_){
				message_block_[message_block_Index_++] = (*message_array & 0xFF);

				length_low_ += 8;
				length_low_ &= 0xFFFFFFFF;               // Force it to 32 bits
				if (length_low_ == 0){
					length_high_++;
					length_high_ &= 0xFFFFFFFF;          // Force it to 32 bits
					if (length_high_ == 0)
					{
						corrupted_ = true;               // Message is too long
					}
				}

				if (message_block_Index_ == 64){
					process_message_block();
				}

				message_array++;
			}
		}

		void input(const char* message_array,
			unsigned    length){
			input((unsigned char*)message_array, length);
		}


		void input(unsigned char message_element){
			input(&message_element, 1);
		}


		void input(char message_element){
			input((unsigned char*)&message_element, 1);
		}


		sha1& operator<<(const char* message_array){
			const char* p = message_array;

			while (*p){
				input(*p);
				p++;
			}

			return *this;
		}

		sha1& operator<<(const unsigned char* message_array){
			const unsigned char* p = message_array;

			while (*p)	{
				input(*p);
				p++;
			}

			return *this;
		}


		sha1& operator<<(const char message_element){
			input((unsigned char*)&message_element, 1);

			return *this;
		}

		sha1& operator<<(const unsigned char message_element){
			input(&message_element, 1);

			return *this;
		}


	private:
		void process_message_block(){
			const unsigned K[] = {               // Constants defined for SHA-1
										0x5A827999,
										0x6ED9EBA1,
										0x8F1BBCDC,
										0xCA62C1D6
			};
			int         t;                          // Loop counter
			unsigned    temp;                       // Temporary word value
			unsigned    W[80];                      // Word sequence
			unsigned    A, B, C, D, E;              // Word buffers

			/*
			 *  Initialize the first 16 words in the array W
			 */
			for (t = 0; t < 16; t++){
				W[t] = ((unsigned)message_block_[t * 4]) << 24;
				W[t] |= ((unsigned)message_block_[t * 4 + 1]) << 16;
				W[t] |= ((unsigned)message_block_[t * 4 + 2]) << 8;
				W[t] |= ((unsigned)message_block_[t * 4 + 3]);
			}

			for (t = 16; t < 80; t++){
				W[t] = circular_shift_(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
			}

			A = h_[0];
			B = h_[1];
			C = h_[2];
			D = h_[3];
			E = h_[4];

			for (t = 0; t < 20; t++){
				temp = circular_shift_(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
				temp &= 0xFFFFFFFF;
				E = D;
				D = C;
				C = circular_shift_(30, B);
				B = A;
				A = temp;
			}

			for (t = 20; t < 40; t++){
				temp = circular_shift_(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
				temp &= 0xFFFFFFFF;
				E = D;
				D = C;
				C = circular_shift_(30, B);
				B = A;
				A = temp;
			}

			for (t = 40; t < 60; t++){
				temp = circular_shift_(5, A) +
					((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
				temp &= 0xFFFFFFFF;
				E = D;
				D = C;
				C = circular_shift_(30, B);
				B = A;
				A = temp;
			}

			for (t = 60; t < 80; t++){
				temp = circular_shift_(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
				temp &= 0xFFFFFFFF;
				E = D;
				D = C;
				C = circular_shift_(30, B);
				B = A;
				A = temp;
			}

			h_[0] = (h_[0] + A) & 0xFFFFFFFF;
			h_[1] = (h_[1] + B) & 0xFFFFFFFF;
			h_[2] = (h_[2] + C) & 0xFFFFFFFF;
			h_[3] = (h_[3] + D) & 0xFFFFFFFF;
			h_[4] = (h_[4] + E) & 0xFFFFFFFF;

			message_block_Index_ = 0;
		}

		void pad_message(){
			/*
			 *  Check to see if the current message block is too small to hold
			 *  the initial padding bits and length.  If so, we will pad the
			 *  block, process it, and then continue padding into a second block.
			 */
			if (message_block_Index_ > 55){
				message_block_[message_block_Index_++] = 0x80;
				while (message_block_Index_ < 64){
					message_block_[message_block_Index_++] = 0;
				}

				process_message_block();

				while (message_block_Index_ < 56){
					message_block_[message_block_Index_++] = 0;
				}
			}
			else{
				message_block_[message_block_Index_++] = 0x80;
				while (message_block_Index_ < 56)
				{
					message_block_[message_block_Index_++] = 0;
				}

			}

			/*
			 *  Store the message length as the last 8 octets
			 */
			message_block_[56] = (length_high_ >> 24) & 0xFF;
			message_block_[57] = (length_high_ >> 16) & 0xFF;
			message_block_[58] = (length_high_ >> 8) & 0xFF;
			message_block_[59] = (length_high_) & 0xFF;
			message_block_[60] = (length_low_ >> 24) & 0xFF;
			message_block_[61] = (length_low_ >> 16) & 0xFF;
			message_block_[62] = (length_low_ >> 8) & 0xFF;
			message_block_[63] = (length_low_) & 0xFF;

			process_message_block();
		}


		/*
		 *  CircularShift
		 *
		 *  Description:
		 *      This member function will perform a circular shifting operation.
		 *
		 *  Parameters:
		 *      bits: [in]
		 *          The number of bits to shift (1-31)
		 *      word: [in]
		 *          The value to shift (assumes a 32-bit integer)
		 *
		 *  Returns:
		 *      The shifted value.
		 *
		 *  Comments:
		 *
		 */
		unsigned circular_shift_(int bits, unsigned word)
		{
			return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32 - bits));
		}


	private:
		unsigned h_[5];                      // Message digest buffers

		unsigned length_low_;                // Message length in bits
		unsigned length_high_;               // Message length in bits

		unsigned char message_block_[64];    // 512-bit message blocks
		int message_block_Index_;            // Index into message block array

		bool computed_;                      // Is the digest computed?
		bool corrupted_;                     // Is the message digest corruped?

	};

}
#endif // sha1_h__