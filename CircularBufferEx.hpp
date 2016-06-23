#ifndef CIRCULAR_BUFFER_EX_H
#define CIRCULAR_BUFFER_EX_H

#include <cstddef>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

/** This class implements a static circular buffer.

 CirularBuffer class originally by François Berder ( see http://mbed.org/users/feb11/code/CircularBuffer/ )
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
	 * �����ݶ�ȡ���������У���ɾ������������
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
	 * �����ݶ�ȡ���������У���ɾ������������
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
	 * ��Ѱ���������Ƿ��з����������ַ�������Ѱ�������ݻ��Ƴ���
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
	 * �����ݶ�ȡ���������У�����ɾ������������
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

			// ����Ѿ�������������ǰ�������
			if(m_last == m_first)
			{
				increment(m_first);
			}
		}
	}

	/**
	 * ������뻺�����ݡ��绺���Ѿ�������ɾ����ǰ�������
	 * @param item
	 */
	void push_back(T item)
	{
		m_buff[m_last] = item;
		if (++m_last == N)
			m_last = 0;
		// ������ǰ������ݣ���������������⣬�������Ϳ��޷����֡�
		if (m_first == m_last)
		{
			if (++m_first == N)
				m_first = 0;
		}
	}

	/**
	 * ����е�����⣬�򻺳�ֻ�ܷ���N-1������
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
	 * ֻ��N-1���������Ϳ�û�����֣���������һ����С���ֶΡ�
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
	 * �Ƴ�ǰ����������
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
