#pragma once

#include <mutex>
#include <thread>
#include <unordered_map>

template<typename T>
class TStatic
{
private:
    std::mutex                              m_mutex;
    std::unordered_map<std::thread::id, T*> m_data;

protected:

public:
    TStatic()
    {
        m_data = std::unordered_map<std::thread::id, T*>();
    }
    TStatic(const TStatic& a_other)
    {
        const std::lock_guard otherG = std::lock_guard(a_other.m_mutex);
        const std::lock_guard g = std::lock_guard(m_mutex);

        m_data = a_other.m_data;
    }
    ~TStatic()
    {
        Clear();
    }

    TStatic& operator =(const TStatic& a_other)
    {
        const std::lock_guard otherG = std::lock_guard(a_other.m_mutex);
        const std::lock_guard g = std::lock_guard(m_mutex);

        for (auto iter = m_data.begin(); iter != m_data.end(); ++iter)
        {
            if (iter->second != nullptr)
            {
                delete iter->second;
            }
        }

        m_data = a_other.m_data;
    }

    inline T& operator*() 
    {
        const std::thread::id id = std::this_thread::get_id();

        const std::lock_guard g = std::lock_guard(m_mutex);
        return *(m_data[id]);
    }
    inline T* operator->() 
    {
        const std::thread::id id = std::this_thread::get_id();

        const std::lock_guard g = std::lock_guard(m_mutex);
        return m_data[id];
    }

    T& Push(const T& a_data)
    {
        const std::thread::id id = std::this_thread::get_id();

        T* d = new T(a_data);
        
        const std::lock_guard g = std::lock_guard(m_mutex);

        auto iter = m_data.find(id);
        if (iter != m_data.end())
        {
            delete iter->second;
            iter->second = d;
        }
        else
        {
            m_data.emplace(id, d);
        }

        return *d;
    }

    T* Get()
    {
        const std::thread::id id = std::this_thread::get_id();

        T* dat = nullptr;

        const std::lock_guard g = std::lock_guard(m_mutex);
        auto iter = m_data.find(id);
        if (iter != m_data.end())
        {
            dat = iter->second;
        }

        return dat;
    }

    void Erase()
    {
        const std::thread::id id = std::this_thread::get_id();

        const std::lock_guard g = std::lock_guard(m_mutex);
        auto iter = m_data.find(id);
        if (iter != m_data.end())
        {
            delete iter->second;

            m_data.erase(iter);
        }
    }

    void Clear()
    {
        const std::lock_guard g = std::lock_guard(m_mutex);

        for (auto iter = m_data.begin(); iter != m_data.end(); ++iter)
        {
            delete iter->second;
        }

        m_data.clear();
    }
};