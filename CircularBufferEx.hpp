#ifndef CIRCULAR_BUFFER_EX_H
#define CIRCULAR_BUFFER_EX_H

#include <cstddef>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

/** This class implements a static circular buffer.

 CirularBuffer class originally by Fran莽ois Berder ( see http://mbed.org/users/feb11/code/CircularBuffer/ )
 Additions by John Bailey ( http://mbed.org/users/johnb/ )
 */
class CircularBufferEx
{
public:
	typedef unsigned int size_type;
	typedef int difference_type;
	typedef unsigned char T;
	typedef uint16_t index_t;

private:
    void increment(index_t& p) const {
        if (++p == N)
            p = 0;
    }

    void decrement(index_t& p) const {
        if (p == 0)
            p = N-1;
        else
        {
        	--p;
        }
    }

    index_t add(index_t p, difference_type n) const
    {
        return n+p < N? n+p : n+p-N;
    }

    index_t sub(index_t p, difference_type n) const
    {
        return p<n? N + p - n : p - n;
    }


public:
	CircularBufferEx(T* buf, int size) :
		m_buff(buf), N(size), m_first(0), m_last(0)
	{
	}

	/**
	 * 将数据读取到缓冲区中，并删除缓冲内数据
	 * @param data
	 * @param length
	 * @return
	 */
	size_type read(T *data, size_type length)
	{
		uint32_t n = 0;
		while (n < length && size() > 0)
		{
			data[n++] = m_buff[m_first++];
			if (m_first == N)
				m_first = 0;
		}

		return n;
	}

	/**
	 * 将数据读取到缓冲区中，不删除缓冲内数据
	 * @param data
	 * @param length
	 * @return
	 */
	size_type find(T *data, size_type length)
	{
		uint32_t n = 0;
		index_t src = m_first;
		while ((n < length) && (n < size()))
		{
			data[n++] = m_buff[src++];
			if (src == N)
			{
				src = 0;
			}
		}

		return n;
	}

	/**
	 * 搜寻缓冲区内是否有符合条件的字符串，搜寻过的数据会移除。
	 * @param str
	 * @param caseless
	 * @return
	 */
	bool SeekStr(const char* str, bool caseless)
	{
		int i;
		int len = strlen(str);

		while(size() >= len)
		{
			if(caseless)
			{
				for(i = 0; i < len; i++)
				{
					if(tolower(str[i]) != tolower(operator[](i)))
					{
						break;
					}
				}
			}
			else
			{
				for(i = 0; i < len; i++)
				{
					if(str[i] != operator[](i))
					{
						break;
					}
				}
			}

			if(i == len)
			{
				erase_begin(len);
				return(true);
			}
			pop_front();
		}
		return(false);
	}

	/**
	 * 将数据读取到缓冲区中，但不删除缓冲内数据
	 * @param data
	 * @param length
	 * @return
	 */
	size_type peek(T *data, size_type length) const
	{
		uint32_t n = 0;
		index_t src = m_first;
		while ((n < length) && (n < size()))
		{
			data[n++] = m_buff[src++];
			if (src == N)
			{
				src = 0;
			}
		}

		return n;
	}

	void write(T *data, size_type length)
	{
		uint32_t n = 0;
		while (n < length)
		{
			m_buff[m_last++] = data[n++];
			if (m_last == N)
				m_last = 0;

			// 如果已经满，则舍弃最前面的数据
			if(m_last == m_first)
			{
				increment(m_first);
			}
		}
	}

	/**
	 * 处理进入缓冲数据。如缓冲已经满，则删除最前面的数据
	 * @param item
	 */
	void push_back(T item)
	{
		m_buff[m_last] = item;
		if (++m_last == N)
			m_last = 0;
		// 舍弃最前面的数据，处理满溢出的问题，否则满和空无法区分。
		if (m_first == m_last)
		{
			if (++m_first == N)
				m_first = 0;
		}
	}

	/**
	 * 这个有点点问题，因缓冲只能放置N-1个数据
	 * @return
	 */
	size_type capacity() const
	{
		return N-1;
	}

	size_type AvailableSize() const
	{
		return ((m_last >= m_first) ? (N-1-m_last + m_first) : (m_first - m_last - 1));
	}

	size_type size() const
	{
		return ((m_last >= m_first) ? (m_last - m_first) : (N + m_last - m_first));
	}

	T front() const
	{
		return m_buff[m_first];
	}

	T back() const
	{
		return m_buff[m_last > 0 ? m_last - 1 : N - 1];
	}

	bool empty() const
	{
		return size() == 0;
	}

	/**
	 * 只能N-1，否则满和空没法区分，除非增加一个大小的字段。
	 * @return
	 */
	bool full() const
	{
		return size() == N-1;
	}

	void clear()
	{
		m_first = m_last;
	}

	T operator[](size_type p_length) const
	{
		return m_buff[(m_first + p_length) % N];
	}

	/**
	 * 移除前面若干数据
	 * @param length
	 */
	void erase_begin(size_type length)
	{
		if (length >= size())
		{
			m_first = m_last;
		}
		else
		{
			m_first = (m_first + length) % N;
		}
	}

	void pop_front()
	{
		assert(!empty());
		m_first = (m_first + 1) % N;
	}

private:
	T* m_buff;
	const int N;
	index_t m_first, m_last;
};

#endif
