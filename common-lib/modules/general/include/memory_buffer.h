// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef __COMMON_Memory_Buffer_H_7e5fa8c2415240ea93eff148ed73539b
#define __COMMON_Memory_Buffer_H_7e5fa8c2415240ea93eff148ed73539b

#include <functional>
#include <istream>
#include <memory>
#include <string>

#include "nocopy.h"

namespace hebench {
namespace Common {

/**
 * @brief Interface that represents a memory buffer.
 */
class IMemoryBuffer
{
public:
    typedef std::shared_ptr<IMemoryBuffer> Ptr;

    IMemoryBuffer() {}
    virtual ~IMemoryBuffer() {}

    /**
     * @brief Retrieves pointer to the underlying data buffer.
     * @return Pointer to the underlying data buffer or null if
     * no underlying data buffer.
     */
    virtual void *getData() const = 0;

    template <class T>
    /**
     * @brief Retrieves the underlying data buffer as a pointer
     * of the specified type.
     * @return Pointer to the underlying data buffer casted to
     * specified type or null if no underlying data buffer.
     */
    T *getDataAs() const
    {
        return reinterpret_cast<T *>(getData());
    }

    /**
     * @brief Retrieves the size of the underlying data buffer.
     * @return Size in bytes of the underlying data buffer or 0
     * if no underlying data buffer.
     */
    virtual std::size_t getSize() const = 0;

    template <class T>
    /**
     * @brief Retrieves the size of the underlying data buffer.
     * @return Size, in number of elements of specified type, of
     * the underlying data buffer or 0 if no underlying data buffer.
     */
    std::size_t getSizeAs() const
    {
        return getSize() / sizeof(T);
    }
};

/**
 * @brief Implementation of common functionality of interface.
 */
class PartialMemoryBuffer : public IMemoryBuffer
{
public:
    PartialMemoryBuffer() :
        m_data(nullptr), m_size(0) {}
    virtual ~PartialMemoryBuffer() {}

    void *getData() const override { return m_data; }

    std::size_t getSize() const override { return m_size; }

protected:
    mutable void *m_data;
    std::size_t m_size;
};

/**
 * @brief Helper class to hold shallow copies of memory buffers.
 * @details Maintaining accuracy of the size of the buffer is client's
 * responsibility.
 *
 * Objects of this class perform no memory allocation, deallocation,
 * checks or maintenance, except calling a client-specified deleter
 * function on destruction.
 */
class ShallowMemoryBuffer : public PartialMemoryBuffer
{
public:
    DISABLE_COPY(ShallowMemoryBuffer)

    /**
     * @brief Smart pointer definition for ShallowMemoryBuffer.
     * @details ShallowMemoryBuffer objects are not copiable. Use the provided
     * smart pointer definition to allow copying of these objects.
     */
    typedef std::shared_ptr<ShallowMemoryBuffer> Ptr;

    /**
     * @brief Returns an empty smart ShallowMemoryBuffer object.
     * @return Smart pointer to the created ShallowMemoryBuffer object.
     */
    static ShallowMemoryBuffer::Ptr create() { return std::make_shared<ShallowMemoryBuffer>(); }

    /**
     * @brief Creates a new ShallowMemoryBuffer object containing a shallow
     * copy of the specified data.
     * @param data Pointer to the data.
     * @param sizeBytes[in] Size in bytes of the data.
     * @param deleter[in] Function used to delete the data pointer on
     * destruction. If null, pointer is not deleted on destruction.
     * @details \p deleter function must not throw any exceptions.
     * @return Smart pointer to the created ShallowMemoryBuffer object.
     */
    static ShallowMemoryBuffer::Ptr create(void *data, std::size_t sizeBytes, std::function<void(void *)> deleter = nullptr)
    {
        return std::make_shared<ShallowMemoryBuffer>(data, sizeBytes, deleter);
    }

    /**
     * @brief Creates a new empty ShallowMemoryBuffer object.
     */
    ShallowMemoryBuffer() :
        ShallowMemoryBuffer(nullptr, 0) {}

    /**
     * @brief Creates a new ShallowMemoryBuffer object containing a shallow
     * copy of the specified data.
     * @param data Pointer to the data.
     * @param sizeBytes[in] Size in bytes of the data.
     * @param deleter[in] Function used to delete the data pointer on
     * destruction. If null, pointer is not deleted on destruction.
     * @details \p deleter function must not throw any exceptions.
     */
    ShallowMemoryBuffer(void *data, std::size_t sizeBytes, std::function<void(void *)> deleter = nullptr) { set(data, sizeBytes, deleter); }

    /**
     * @brief Move constructor.
     */
    ShallowMemoryBuffer(ShallowMemoryBuffer &&src) :
        ShallowMemoryBuffer(src.getData(), src.getSize(), src.getDeleter())
    {
        src.m_data    = nullptr;
        src.m_size    = 0;
        src.m_deleter = nullptr;
    }

    /**
     * @brief Destroys this ShallowMemoryBuffer object.
     * @details If object has an associated deleter, the deleter
     * function is called during destruction.
     */
    ~ShallowMemoryBuffer() override { cleanUp(); }

    /**
     * @brief Move assignment operator.
     */
    ShallowMemoryBuffer &operator=(ShallowMemoryBuffer &&src)
    {
        if (this != &src)
        {
            set(src.getData(), src.getSize(), src.getDeleter());
            src.m_data    = nullptr;
            src.m_size    = 0;
            src.m_deleter = nullptr;
        }
        return *this;
    }

    /**
     * @brief Sets a new shallow copy of the specified data.
     * @param data Pointer to the data.
     * @param sizeBytes[in] Size in bytes of the data.
     * @param deleter[in] Function used to delete the data pointer on
     * destruction. If null, pointer is not deleted on destruction.
     * @details If previous data had an associated deleter, the deleter
     * function is called before setting the new pointers.
     */
    void set(void *data, std::size_t sizeBytes, std::function<void(void *)> deleter = nullptr) noexcept
    {
        cleanUp();

        m_data    = data;
        m_size    = (data ? sizeBytes : 0);
        m_deleter = deleter;
    }

    /**
     * @brief Retrieves the deleter currently associated to this object.
     * @return This function will be called to delete the data pointer
     * associated with this ShallowMemoryBuffer.
     */
    std::function<void(void *)> getDeleter() const noexcept { return m_deleter; }

private:
    void cleanUp()
    {
        if (m_deleter && m_data)
            m_deleter(m_data);
    }

    std::function<void(void *)> m_deleter;
};

/**
 * @brief Helper class to hold the contents of a buffer in memory.
 * Provides helper methods to load data from an input stream and files.
 * @details Clients should be mindful of the size of the data in input streams
 * and files since objects of this class will attempt to load all the data into
 * system memory.
 */
class DeepMemoryBuffer : public PartialMemoryBuffer
{
public:
    /**
     * @brief Smart pointer definition for DeepMemoryBuffer.
     * @details Copying DeepMemoryBuffer objects can be an expensive operation
     * since it is a deep copy. Use the provided smart pointer definition to
     * allow passing pointer copies of these objects.
     */
    typedef std::shared_ptr<DeepMemoryBuffer> Ptr;

    /**
     * @brief Constructs a new MemoryBuffer object with enough memory
     * allocated to hold as many bites as specified.
     * @param sizeBytes[in] Number of bytes to allocate.
     * @param binitialize[in] If true, the buffer is initialized to 0,
     * otherwise, it is left uninitialized.
     * @return Smart pointer to a DeepMemoryBuffer object.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    static DeepMemoryBuffer::Ptr create(std::size_t sizeBytes, bool binitialize = true)
    {
        return std::make_shared<DeepMemoryBuffer>(sizeBytes, binitialize);
    }
    /**
     * @brief Creates a new MemoryBuffer object containing a deep copy of the
     * specified data.
     * @param data[in] Pointer to the data to copy. Must not be null.
     * @param sizeBytes[in] Size in bytes of the data to copy. Cannot be 0.
     * @return Smart pointer to a DeepMemoryBuffer object.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    static DeepMemoryBuffer::Ptr create(const void *data, std::size_t sizeBytes)
    {
        return std::make_shared<DeepMemoryBuffer>(data, sizeBytes);
    }
    /**
     * @brief Constructs a new MemoryBuffer object containing the data read
     * from the specified file.
     * @param filename[in] File to read.
     * @return Smart pointer to a DeepMemoryBuffer object.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    static DeepMemoryBuffer::Ptr create(const std::string &filename) { return std::make_shared<DeepMemoryBuffer>(filename); }
    /**
     * @brief Creates a new MemoryBuffer object containing the data read
     * from the specified binary stream.
     * @param is[in] Binary stream from which to read data. Reading starts at
     * the current position in the stream.
     * @return Smart pointer to a DeepMemoryBuffer object.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    static DeepMemoryBuffer::Ptr create(std::istream &is) { return std::make_shared<DeepMemoryBuffer>(is); }

    /**
     * @brief Default constructor. Constructs a new empty MemoryBuffer object.
     */
    DeepMemoryBuffer() :
        DeepMemoryBuffer(nullptr, 0) {}
    /**
     * @brief Constructs a new MemoryBuffer object with enough memory
     * allocated to hold as many bites as specified.
     * @param sizeBytes[in] Number of bytes to allocate.
     * @param binitialize[in] If true, the buffer is initialized to 0,
     * otherwise, it is left uninitialized.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    DeepMemoryBuffer(std::size_t sizeBytes, bool binitialize = true);
    /**
     * @brief Constructs a new MemoryBuffer object containing a copy of the
     * specified data.
     * @param data[in] Pointer to the data to copy. Can be null to initialize
     * an empty buffer.
     * @param sizeBytes[in] Size in bytes of the data to copy. Ignored if
     * data is null; must not be 0 otherwise.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    DeepMemoryBuffer(const void *data, std::size_t sizeBytes);
    /**
     * @brief Constructs a new MemoryBuffer object containing the data read
     * from the specified file.
     * @param filename[in] File to read. If empty, the buffer is initialized as
     * empty.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    explicit DeepMemoryBuffer(const std::string &filename);
    /**
     * @brief Constructs a new MemoryBuffer object containing the data read
     * from the specified binary stream.
     * @param is[in] Binary stream from which to read data. Reading starts at
     * the current position in the stream. If no data left to read, the buffer
     * is initialized as empty.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    explicit DeepMemoryBuffer(std::istream &is);

    /**
     * @brief Deep copy constructor.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    DeepMemoryBuffer(const DeepMemoryBuffer &src);
    /**
     * @brief Move constructor.
     */
    DeepMemoryBuffer(DeepMemoryBuffer &&src);

    /**
     * @brief Deep copy assignment operator.
     * @throw std::bad_alloc on allocation failure.
     * @throw std::exception (instance of) on any other error.
     */
    DeepMemoryBuffer &operator=(const DeepMemoryBuffer &src);
    /**
     * @brief Move assignment operator.
     */
    DeepMemoryBuffer &operator=(DeepMemoryBuffer &&src);

    ~DeepMemoryBuffer() override;

    /**
     * @brief Detaches the data buffer from this object.
     * @return Pointer to the memory buffer wrapped by this object before
     * detaching. Can be null if no memory was wrapped.
     * @details After calling this member, client is responsible for freeing
     * the buffer after it is no longer needed. Use method ReleaseBuffer() to
     * do so.
     */
    void *detachData()
    {
        void *retval = m_data;
        m_data       = nullptr;
        m_size       = 0;

        return retval;
    }

    /**
     * @brief Detaches the data buffer from this object.
     * @return The pointer to the detached data buffer wrapped in a shared_ptr
     * object that will safely clean up the buffer on destruction.
     * @details Clients must not release this buffer themselves. The returned
     * shared_ptr object takes care of doing so according to the behavior of
     * shared_ptr.
     */
    std::shared_ptr<void> detachDataSafe();

    /**
     * @brief Releases a buffer previously allocated by a DeepMemoryBuffer
     * object.
     * @param buffer[in,out] Buffer to release.
     * @details Use this method only to free buffers detached from
     * DeepMemoryBuffer objects using DetachData(). Buffers that have not been
     * detached or have been detached using DetachDataSafe() are automatically
     * cleaned up by their container on destruction.
     */
    static void releaseBuffer(void *&buffer);

private:
    void init(const void *data, std::size_t sizeBytes);
    void init(std::istream &is);

    static void *allocateBuffer(std::size_t size);
};

} // end namespace Common
} // namespace hebench

#endif // defined __Intel_COMMON_Memory_Buffer_H_7e5fa8c2415240ea93eff148ed73539b
