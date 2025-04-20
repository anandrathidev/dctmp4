
#ifndef _MATRIX_HPP
#define _MATRIX_HPP

#include <vector>
#include <cstdint>
//#include <cstddef>

template<class T>
class Matrix
{
public:
    using IMG_DATA_LINEAR_T = std::vector<T>;
protected:   
    const size_t m_cols;
    const size_t m_rows;
    IMG_DATA_LINEAR_T m_data;    
public:
    Matrix(size_t cols, size_t rows, size_t block_size, const T* data ):
    m_cols{cols},
    m_rows{rows}
    {
        m_data.resize(m_cols * m_rows );
        std::copy(data, data + (m_cols * m_rows), m_data.begin());
    }

    Matrix(size_t cols, size_t rows, size_t channels, size_t block_size ):
    m_cols{cols},
    m_rows{rows},
    m_data(cols*rows*channels, 0)
    {}

    Matrix(const Matrix<T>& other) = default;
    Matrix& operator=(const Matrix<T>& other) = default;
    Matrix(Matrix<T>&& other) = default;
    Matrix& operator=(Matrix<T>&& other) = default;
    ~Matrix()= default;

    inline T&  get(size_t row, size_t col)  {return m_data[ row*m_cols + col] ;} ;
    inline T&  operator()(size_t row, size_t col)  {return m_data[ row*m_cols + col] ;} ;
    inline T&  set(size_t row, size_t col, uint8_t val)  {return m_data[ row*m_cols + col]=val;  } ;
    
    const IMG_DATA_LINEAR_T&  data() const  {return m_data;} ;
    size_t width() const { return m_cols; }
    size_t height() const { return m_rows; }
    size_t size() const { return m_data.size(); }
    

    size_t block_count(size_t block_size) const  { return (m_cols * m_rows) / (block_size*block_size); }

    IMG_DATA_LINEAR_T get_block(size_t block_index, size_t block_size) const
    {
        size_t block_width = m_cols / block_size;
        size_t block_height = m_rows / block_size;
        size_t block_x = block_index % block_width;
        size_t block_y = block_index / block_width;

        IMG_DATA_LINEAR_T block_data(block_size * block_size );
        for (size_t y = 0; y < block_size; ++y)
        {
            for (size_t x = 0; x < block_size; ++x)
            {
                //for (size_t c = 0; c < m_channels; ++c)
                {
                    // size_t src_index = ((block_y * block_size + y) * m_cols + (block_x * block_size + x)) * m_channels + c;
                    // size_t dst_index = (y * block_size + x) * m_channels + c;
                    size_t src_index = ((block_y * block_size + y) * m_cols + (block_x * block_size + x)) ;
                    size_t dst_index = (y * block_size + x) ; 
                    block_data[dst_index] = m_data[src_index];
                }
            }
        }
        return block_data;
    }

};


template<class T, size_t ROWST, size_t COLST>
class FixedMatrix
{
public:
    using MATRIX_DATA_LINEAR_T = std::array<T, ROWST*COLST>;
protected:   
    const size_t m_cols;
    const size_t m_rows;
    MATRIX_DATA_LINEAR_T m_data;    
public:
    FixedMatrix(const T* data):
    m_cols{COLST},
    m_rows{ROWST}
    {
        std::copy(data, data + (m_cols * m_rows ), m_data.begin());
    }

    FixedMatrix(size_t cols, size_t rows, size_t channels, size_t val ):
    m_cols{COLST},
    m_rows{ROWST},
    m_data{val} 
    {}

    FixedMatrix(const FixedMatrix<T, ROWST, COLST>& other) = default;
    FixedMatrix& operator=(const FixedMatrix<T, ROWST, COLST>& other) = default;
    FixedMatrix(FixedMatrix<T, ROWST, COLST>&& other) = default;
    FixedMatrix& operator=(FixedMatrix<T, ROWST, COLST>&& other) = default;
    ~FixedMatrix()= default;

    inline T& get(size_t row, size_t col)  {return m_data[ row*m_cols + col] ;} ;
    inline T& operator()(size_t row, size_t col)  {return m_data[ row*m_cols + col] ;} ;
    inline T& set(size_t row, size_t col, uint8_t val)  {return m_data[ row*m_cols + col]=val;  } ;
    
    const std::vector<uint8_t>&  data() const  {return m_data;} ;
    size_t width() const { return m_cols; }
    size_t height() const { return m_rows; }
    size_t size() const { return m_data.size(); }
    

    size_t block_count(int blk_size) const  { return (m_cols * m_rows) / (blk_size*blk_size); }

    MATRIX_DATA_LINEAR_T get_block(size_t block_index, int block_size) const
    {
        size_t block_width = m_cols / block_size;
        size_t block_height = m_rows / block_size;
        size_t block_x = block_index % block_width;
        size_t block_y = block_index / block_width;

        MATRIX_DATA_LINEAR_T block_data(block_size * block_size );
        for (size_t y = 0; y < block_size; ++y)
        {
            for (size_t x = 0; x < block_size; ++x)
            {
                //for (size_t c = 0; c < m_channels; ++c)
                {
                    size_t src_index = ((block_y * block_size + y) * m_cols + (block_x * block_size + x)) ;
                    size_t dst_index = (y * block_size + x) ;
                    block_data[dst_index] = m_data[src_index];
                }
            }
        }
        return block_data;
    }

};

struct Channels
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

using Image =  Matrix<Channels>;
using DCTMatrixT =  Matrix<float>;

#endif // _MATRIX_HPP