#include "write_jpeg.hpp"
#include <boost/dynamic_bitset.hpp>

void Bitstream::append( bool val)
{
    m_bit_stream->push_back(val);
}
