#pragma once

#include <cstring>
#include <mutex>
#include <vector>

template<typename T>
class TArray
{
private:
    std::mutex m_mutex;
    uint32_t   m_size;
    T*         m_data;

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
        m_data = new T[m_size];
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_other.m_data[i];
        }
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
        m_data = new T[m_size];
        for (uint32_t i = 0; i < a_size; ++i)
        {
            m_data[i] = a_data[i];
        }
    }
    TArray(const T* a_start, const T* a_end)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);
        
        m_size = a_end - a_start;
        m_data = new T[m_size];
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_start[i];
        }
    }
    explicit TArray(const std::vector<T> a_vec)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        m_size = (uint32_t)a_vec.size();
        m_data = new T[m_size];
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
            free(m_data);
            m_data = nullptr;
        }
    }

    TArray operator =(const TArray& a_other) 
    {
        const std::lock_guard otherG = std::lock_guard(a_other.m_mutex);
        const std::lock_guard g = std::lock_guard(m_mutex);

        if (m_data != nullptr)
        {
            free(m_data);
            m_data = nullptr;
        }

        m_size = a_other.m_size;
        m_data = new T[m_size];
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

    void Push(T a_data)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        const uint32_t aSize = (m_size + 1) * sizeof(T);

        // T* dat = new T[m_size + 1];
        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {   
            for (uint32_t i = 0; i < m_size; ++i)
            {
                dat[i] = m_data[i];
            }

            free(m_data);
        }

        dat[m_size++] = a_data;
        
        m_data = dat;
    }
    T Pop()
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        return m_data[--m_size];
    }
    void Erase(uint32_t a_index)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        --m_size;
        for (uint32_t i = a_index; i < m_size; ++i)
        {
            m_data[i] = m_data[i + 1];
        }
    }
    void Erase(uint32_t a_start, uint32_t a_end)
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        const uint32_t diff = a_end - a_start;
        m_size -= diff;
        for (uint32_t i = a_start; i < a_end; ++i)
        {
            m_data[i] = m_data[i + 1];
        }
    }

    void Clear()
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        m_size = 0;
    }
};