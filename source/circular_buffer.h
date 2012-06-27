/*
	CIRCULAR_BUFFER.C
	-----------------
*/
#ifndef CIRCULAR_BUFFER_C_
#define CIRCULAR_BUFFER_C_

/*
	class ATOSE_circular_buffer
	---------------------------
*/
template <typename base, long size>
class ATOSE_circular_buffer
{
private:
	base buffer[size];
	long read_index;
	long write_index;

public:
	ATOSE_circular_buffer() { rewind(); }
	virtual ~ATOSE_circular_buffer() {};

	void rewind(void) { read_index = write_index = 0; }
	long is_empty(void) { return read_index == write_index; }
	long is_full() { return (write_index + 1) % size == read_index; }

	long write(base value)
		{
		if (!is_full())
			{
			buffer[write_index] = value;
			write_index = (write_index + 1) % size;
			}
		return 1;
		}

	base read(void)
		{
		if (!is_empty())
			{
			base answer;
			answer = buffer[read_index];
			read_index = (read_index + 1) % size;

			return answer;
			}
		return (base)0;
		}
} ;

#endif /* CIRCULAR_BUFFER_C_ */
