#ifndef _WRITE_JPEG_HPP
#define _WRITE_JPEG_HPP

#include <boost/dynamic_bitset_fwd.hpp>
#include <vector>

class Bitstream
{
    public:
    std::unique_ptr< boost::dynamic_bitset<uint8_t>> m_bit_stream;    
    Bitstream(size_t size)
    {
        m_bit_stream = std::make_unique<boost::dynamic_bitset<uint8_t>>(size);
    }
    Bitstream(const Bitstream& other) = default;
    Bitstream& operator=(const Bitstream& other) = default;
    Bitstream(Bitstream&& other) = default;
    Bitstream& operator=(Bitstream&& other) = default;
    ~Bitstream()= default;
    void append(bool val);
    void append(uint8_t val);
};

// represent a single Huffman code
struct BitCode
{
  BitCode() = default; // undefined state, must be initialized at a later time
  BitCode(uint16_t code_, uint8_t numBits_)
  : code(code_), numBits(numBits_) {}
  uint16_t code;       // JPEG's Huffman codes are limited to 16 bits
  uint8_t  numBits;    // number of valid bits
};


class Bytestream
{
    public:
    std::vector< uint8_t> m_byte_stream;    
    Bytestream(size_t size):m_byte_stream(size) { }
    Bytestream(const Bytestream& other) = default;
    Bytestream& operator=(const Bytestream& other) = default;
    Bytestream(Bytestream&& other) = default;
    Bytestream& operator=(Bytestream&& other) = default;
    ~Bytestream()= default;
    void append(uint8_t val) { m_byte_stream.push_back(val); }
    // start a new JFIF block
    void addMarker(uint8_t id, uint16_t length)
    {
        m_byte_stream.push_back(0xFF); 
        m_byte_stream.push_back(id);     // ID, always preceded by 0xFF
        m_byte_stream.push_back(uint8_t(length >> 8)); // length of the block (big-endian, includes the 2 length bytes as well)
        m_byte_stream.push_back(uint8_t(length & 0xFF));
    }

};


#endif// _WRITE_JPEG_HPP
