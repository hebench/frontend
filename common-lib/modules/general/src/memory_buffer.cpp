// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "../include/memory_buffer.h"

#include <cstring>
#include <fstream>
#include <new>
#include <stdexcept>
#include <string>

hebench::Common::DeepMemoryBuffer::DeepMemoryBuffer(std::size_t sizeBytes, bool binitialize)
{
    init(nullptr, sizeBytes);
    if (binitialize && m_data)
        std::memset(m_data, 0, m_size);
}

hebench::Common::DeepMemoryBuffer::DeepMemoryBuffer(const void *data, std::size_t sizeBytes)
{
    init(data, sizeBytes);
}

hebench::Common::DeepMemoryBuffer::DeepMemoryBuffer(const std::string &filename)
{
    std::ifstream fNumIn;

    fNumIn.open(filename, std::ifstream::in | std::ifstream::binary);
    if (!fNumIn.is_open())
        throw std::ios_base::failure(std::string(__func__) + ": Cannot access file \"" + filename + "\".");

    try
    {
        init(fNumIn);
    }
    catch (...)
    {
        if (fNumIn.is_open())
            fNumIn.close();

        throw;
    }

    if (fNumIn.is_open())
        fNumIn.close();
}

hebench::Common::DeepMemoryBuffer::DeepMemoryBuffer(std::istream &is)
{
    init(is);
}

hebench::Common::DeepMemoryBuffer::DeepMemoryBuffer(const DeepMemoryBuffer &src) :
    DeepMemoryBuffer(src.getData(), src.getSize()) {}

hebench::Common::DeepMemoryBuffer::DeepMemoryBuffer(DeepMemoryBuffer &&src)
{
    m_size = src.getSize();
    m_data = src.detachData();
}

hebench::Common::DeepMemoryBuffer &hebench::Common::DeepMemoryBuffer::operator=(const DeepMemoryBuffer &src)
{
    if (this != &src)
    {
        DeepMemoryBuffer::releaseBuffer(m_data);
        init(src.getData(), src.getSize());
    }

    return *this;
}

hebench::Common::DeepMemoryBuffer &hebench::Common::DeepMemoryBuffer::operator=(DeepMemoryBuffer &&src)
{
    if (this != &src)
    {
        DeepMemoryBuffer::releaseBuffer(m_data);
        m_size = src.getSize();
        m_data = src.detachData();
    }

    return *this;
}

hebench::Common::DeepMemoryBuffer::~DeepMemoryBuffer()
{
    DeepMemoryBuffer::releaseBuffer(m_data);
}

void hebench::Common::DeepMemoryBuffer::init(const void *data, std::size_t sizeBytes)
{
    m_size = sizeBytes;

    m_data = (m_size > 0 ? DeepMemoryBuffer::allocateBuffer(m_size) : nullptr);

    if (m_data && data)
        std::memcpy(m_data, data, m_size);
}

void hebench::Common::DeepMemoryBuffer::init(std::istream &is)
{
    // read the contents of the stream starting at the current position

    m_data = nullptr;

    // cache the current position
    std::streampos curPos = is.tellg();

    // find the number of bytes to read
    is.seekg(0, is.end);
    m_size = is.tellg() - curPos;

    // reposition the reading pointer
    is.seekg(curPos, is.beg);

    // check if there is data to read
    if (m_size > 0)
    {
        try
        {
            // allocate memory to hold the data
            m_data = DeepMemoryBuffer::allocateBuffer(m_size);

            // read the data
            is.read((char *)m_data, m_size);

            // check if all the data was read successfully
            if ((std::size_t)is.gcount() < m_size)
                throw std::ios_base::failure(std::string(__func__) + ": Unexpected end of stream found.");
        }
        catch (...)
        {
            // clean up if error
            DeepMemoryBuffer::releaseBuffer(m_data);
            m_size = 0;

            throw;
        }
    }
}

std::shared_ptr<void> hebench::Common::DeepMemoryBuffer::detachDataSafe()
{
    return std::shared_ptr<void>(detachData(), [](void *ptr) {
        if (ptr)
            DeepMemoryBuffer::releaseBuffer(ptr);
    });
}

void *hebench::Common::DeepMemoryBuffer::allocateBuffer(std::size_t size)
{
    void *retval = nullptr;

    if (size == 0)
        // TODO: replace with a descendant of bad_alloc.
        throw std::runtime_error(std::string(__func__) + ": Invalid allocation size.");

    retval = (void *)new std::uint8_t[size];

    if (!retval)
        // allocation failed
        // TODO: replace with a descendant of bad_alloc.
        throw std::bad_alloc();

    return retval;
}

void hebench::Common::DeepMemoryBuffer::releaseBuffer(void *&buffer)
{
    if (buffer)
    {
        delete[](std::uint8_t *) buffer;
        buffer = nullptr;
    }
}
