
#ifndef FFMPEG_IMAGE_HPP
#define FFMPEG_IMAGE_HPP

#include <vector>
#include <cstdint>
#include <cstddef>



class dctImage
{
public:
    using IMG_DATA_LINEAR_T = std::vector<uint8_t>;
protected:   
    IMG_DATA_LINEAR_T m_data;    
    const size_t m_width;
    const size_t m_height;
    const size_t m_channels;
    const size_t m_block_size;
public:
    dctImage(size_t width, size_t height, size_t channels, size_t block_size, const uint8_t* data ):
    m_width{width},
    m_height{height},
    m_channels{channels},
    m_block_size{block_size}
    {
        m_data.resize(width * height * channels);
        std::copy(data, data + (width * height * channels), m_data.begin());
    }
    dctImage(const dctImage& other) = default;
    dctImage& operator=(const dctImage& other) = default;
    dctImage(dctImage&& other) = default;
    dctImage& operator=(dctImage&& other) = default;
    ~dctImage()= default;

    const std::vector<uint8_t>&  data() const  {return m_data;} ;
    size_t width() const { return m_width; }
    size_t height() const { return m_height; }
    size_t channels() const { return m_channels; }
    size_t size() const { return m_data.size(); }
    
    size_t block_count() const  { return (m_width * m_height) / (m_block_size*m_block_size); }

    IMG_DATA_LINEAR_T get_block(size_t block_index) const
    {
        size_t block_width = m_width / m_block_size;
        size_t block_height = m_height / m_block_size;
        size_t block_x = block_index % block_width;
        size_t block_y = block_index / block_width;

        IMG_DATA_LINEAR_T block_data(m_block_size * m_block_size * m_channels);
        for (size_t y = 0; y < m_block_size; ++y)
        {
            for (size_t x = 0; x < m_block_size; ++x)
            {
                for (size_t c = 0; c < m_channels; ++c)
                {
                    size_t src_index = ((block_y * m_block_size + y) * m_width + (block_x * m_block_size + x)) * m_channels + c;
                    size_t dst_index = (y * m_block_size + x) * m_channels + c;
                    block_data[dst_index] = m_data[src_index];
                }
            }
        }
        return block_data;
    }

};

#endif
