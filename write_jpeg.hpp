#ifndef _WRITE_JPEG_HPP
#define _WRITE_JPEG_HPP

#include <boost/dynamic_bitset_fwd.hpp>
#include <vector>

// represent a single Huffman code
struct BitCode
{
  BitCode() = default; // undefined state, must be initialized at a later time
  BitCode(uint16_t code_, uint8_t numBits_)
  : code(code_), numBits(numBits_) {}
  uint16_t code;       // JPEG's Huffman codes are limited to 16 bits
  uint8_t  numBits;    // number of valid bits
};

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


  // store the most recently encoded bits that are not written yet
  struct BitBuffer
  {
    int32_t data    = 0; // actually only at most 24 bits are used
    uint8_t numBits = 0; // number of valid bits (the right-most bits)
  } buffer;


  // write an array of bytes
  template <typename T, int Size>
  Bytestream& operator<<(T (&manyBytes)[Size])
  {
    for (auto c : manyBytes) {
      m_byte_stream.push_back(c);
    }
    return *this;
  }

  // write Huffman bits stored in BitCode, keep excess bits in BitBuffer
  Bytestream& operator<<(const BitCode& data)
  {
    // append the new bits to those bits leftover from previous call(s)
    buffer.numBits += data.numBits;
    buffer.data   <<= data.numBits;
    buffer.data    |= data.code;

    // write all "full" bytes
    while (buffer.numBits >= 8)
    {
      // extract highest 8 bits
      buffer.numBits -= 8;
      auto oneByte = uint8_t(buffer.data >> buffer.numBits);
      m_byte_stream.push_back(oneByte);

      if (oneByte == 0xFF) // 0xFF has a special meaning for JPEGs (it's a block marker)
        m_byte_stream.push_back(0);   // therefore pad a zero to indicate "nope, this one ain't a marker, it's just a coincidence"

      // note: I don't clear those written bits, therefore buffer.bits may contain garbage in the high bits
      //       if you really want to "clean up" (e.g. for debugging purposes) then uncomment the following line
      //buffer.bits &= (1 << buffer.numBits) - 1;
    }
    return *this;
  }
};

const int16_t CodeWordLimit = 2048; // +/-2^11, maximum value after DCT

class JPEGWriter : public Bytestream
{
    public:
        JPEGWriter(size_t size):Bytestream(size) { }
        JPEGWriter(const JPEGWriter& other) = default;
        JPEGWriter& operator=(const JPEGWriter& other) = default;
        JPEGWriter(JPEGWriter&& other) = default;
        JPEGWriter& operator=(JPEGWriter&& other) = default;
        ~JPEGWriter()= default;
    
        // void writeSOF0Header(Bitstream& bitWriter, uint16_t width, uint16_t height);
        // void writeDQTHeader(Bitstream& bitWriter);
        // void writeDHTHeader(Bitstream& bitWriter);
        // void writeSOSHeader(Bitstream& bitWriter);

      // write the JPEG header
      // this is the first part of the JPEG file, it contains the JFIF header
      
      void writeJPEG(bool isRGB)
      {
        //   write quantization tables
        addMarker(0xDB, 2 + (isRGB ? 2 : 1) * (1 + 8*8)); // length: 65 bytes per table + 2 bytes for this length field

      }


      // JFIF headers
      void writeJFIFHeader()
      {
            const uint8_t HeaderJfif[2+2+16] =
            { 0xFF,0xD8,         // SOI marker (start of image)
            0xFF,0xE0,         // JFIF APP0 tag
            0,16,              // length: 16 bytes (14 bytes payload + 2 bytes for this length field)
            'J','F','I','F',0, // JFIF identifier, zero-terminated
            1,1,               // JFIF version 1.1
            0,                 // no density units specified
            0,1,0,1,           // density: 1 pixel "per pixel" horizontally and vertically
            0,0 };             // no thumbnail (size 0 x 0)
        *this << HeaderJfif;

      }

};


#endif// _WRITE_JPEG_HPP
