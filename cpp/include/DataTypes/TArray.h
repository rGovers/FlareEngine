#pragma once

#include <cstring>
#include <mutex>
#include <vector>

// When in doubt with memory issues write it C style with C++ features
// Using C++ memory features was causing seg-faults and leaks so just done it C style and just manually call the deconstructor when I need to
// All this because I am too lazy to implement constructors
template<typename T>
class TArray
{
private:
    std::mutex m_mutex;
    uint32_t   m_size;
    T*         m_data;

    inline void DestroyData()
    {
        if constexpr (std::is_destructible<T>())
        {
            for (uint32_t i = 0; i < m_size; ++i)
            {
                (&(m_data[i]))->~T();
            }
        }
    }

protected:

public:
    TArray() :
        m_size(0),
        m_data(nullptr) { }
    TArray(const TArray& a_other)
    {
        const std::lock_guard otherG = std::lock_guard(a_other.m_mutex);
        const std::lock_guard g = std::lock_guard(m_mutex);

        m_size = a_other.m_size;
        const uint32_t aSize = sizeof(T) * m_size;
        m_data = (T*)malloc(aSize);
        memcpy(m_data, a_other.m_data, aSize);
    }
    TArray(TArray&& a_other)
    {
        const std::lock_guard otherG = std::lock_guard(a_other.m_mutex);
        const std::lock_guard g = std::lock_guard(m_mutex);

        m_size = a_other.m_size;
        m_data = a_other.m_data;
        a_other.m_size = 0;
        a_other.m_data = nullptr;
    }
    TArray(const T* a_data, uint32_t a_size)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        m_size = a_size;
        const uint32_t aSize = sizeof(T) * m_size;
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_data[i];
        }
    }
    TArray(const T* a_start, const T* a_end)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);
        
        const uint32_t aSize = a_end - a_start;
        m_size = aSize / sizeof(T);
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_start[i];
        }
    }
    explicit TArray(const std::vector<T> a_vec)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        m_size = (uint32_t)a_vec.size();
        const uint32_t aSize = sizeof(T) * m_size;
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_vec[i];
        }
    }
    ~TArray()
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
        }
    }

    TArray operator =(const TArray& a_other) 
    {
        const std::lock_guard otherG = std::lock_guard(a_other.m_mutex);
        const std::lock_guard g = std::lock_guard(m_mutex);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
        }

        m_size = a_other.m_size;
        const uint32_t aSize = m_size * sizeof(T);
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_other.m_data[i];
        }

        return *this;
    }

    std::vector<T> ToVector()
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        if (m_data == nullptr)
        {
            return std::vector<T>();
        }

        return std::vector<T>(m_data, m_data + m_size);
    }

    inline std::mutex& Lock()
    {
        return m_mutex;
    }
    inline uint32_t Size() const
    {
        return m_size;
    }
    inline T* Data() const
    {
        return m_data;
    }
    inline bool Empty() const
    {
        return m_size <= 0;
    }

    T& operator [](uint32_t a_index)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        return m_data[a_index];
    }

    void Push(const T& a_data)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        const uint32_t aSize = (m_size + 1) * sizeof(T);
        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {   
            memcpy(dat, m_data, m_size * sizeof(T));

            free(m_data);
        }

        dat[m_size++] = a_data;
        
        m_data = dat;
    }
    T Pop()
    {
        const std::lock_guard g = std::lock_guard(m_mutex);
        
        T dat = m_data[--m_size];

        if constexpr (std::is_destructible<T>())
        {
            (&(m_data[m_size]))->~T();
        }

        return dat;
    }
    void Erase(uint32_t a_index)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        const uint32_t aSize = (m_size - 1) * sizeof(T);

        if constexpr (std::is_destructible<T>())
        {
            (&(m_data[a_index]))->~T();
        }

        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {
            memcpy(dat, m_data, a_index * sizeof(T));
            memcpy(dat + (a_index * sizeof(T)), m_data + ((a_index + 1) * sizeof(T)), (m_size - a_index - 1) * sizeof(T));

            free(m_data);
        }

        --m_size;
        m_data = dat;
    }
    void Erase(uint32_t a_start, uint32_t a_end)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        const uint32_t diff = a_end - a_start;

        const uint32_t aSize = (m_size - diff) * sizeof(T);

        if constexpr (std::is_destructible<T>())
        {
            for (uint32_t i = a_start; i < a_end; ++i)
            {
                (&(m_data[i]))->~T();
            }
        }

        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {
            memcpy(dat, m_data, a_start * sizeof(T));
            memcpy(dat + (a_start * sizeof(T)), m_data + ((a_end + 1) * sizeof(T)), (m_size - a_start - diff) * sizeof(T));

            free(m_data);
        }

        m_size -= diff;
        m_data = dat;
    }

    void Clear()
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        DestroyData();

        free(m_data);

        m_size = 0;
        m_data = nullptr;
    }
    void UClear()
    {
        DestroyData();

        free(m_data);

        m_size = 0;
        m_data = nullptr;
    }
};