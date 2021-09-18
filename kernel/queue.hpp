#pragma once
#include "error.hpp"

#include <cstddef>
#include <array>
#include <utility>

template <typename T, size_t Size>
class ringqueue{
    public:
        ringqueue<T, Size>(void);
        const T& front(void);
        const T& back(void);
        const T& getmem(size_t num);
        Error pop(void);
        Error push(const T& value);
        bool empty(void){
            return (this->Front == this->Back);
        }
        size_t maxsize(void){
            return Size;
        }
        size_t size(void);
    private:
        size_t Front;
        size_t Back;
        T* data;
};

template <typename T>
class priqueue{
    public:
        priqueue<T>(void);
        const T& front(void);
        const T& back(void);
        Error pop(void);
        Error push(const T& value);
        size_t size(void){
            return Size;
        }
        bool empty(void){
            return (Size == 0);
        }
    private:
        std::pair<T,void*>* first_mem;
        std::pair<T,void*>* last_mem;
        size_t Size;
};





template <typename T, size_t Size>
ringqueue<T, Size>::ringqueue(void){
    this->data = new T[Size];
    this->Front = 0;
    this->Back = 0;
}

template <typename T, size_t Size>
const T& ringqueue<T, Size>::front(void){
    if(this->Front == this->Back){
        return 0;
    }
    return this->data[this->Front];
}

template <typename T, size_t Size>
const T& ringqueue<T, Size>::back(void){
    if(this->Front == this->Back){
        return 0;
    }
    return this->data[this->Back - 1];
}

template <typename T, size_t Size>
const T& ringqueue<T, Size>::getmem(size_t num){
    size_t temp = this->Front + num;
    if(temp >= Size){
        temp -= Size;
    }
    return this->data[Size];
}

template <typename T, size_t Size>
Error ringqueue<T, Size>::pop(void){
    if(this->Front == this->Back){
        return MAKE_ERROR(Error::kEmpty);
    }
    this->Front++;
    if(this->Front == Size){
        this->Front = 0;
    }
    return MAKE_ERROR(Error::kSuccess);
}

template <typename T, size_t Size>
Error ringqueue<T, Size>::push(const T& value){
    if(this->Front == this->Back + 1 | (this->Front == 0 && this->Back + 1 == Size)){
        return MAKE_ERROR(Error::kFull);
    }
    this->Back++;
    if(this->Back == Size){
        this->Back = 0;
    }
    this->data[this->Back] = value;
    return MAKE_ERROR(Error::kSuccess);
}

template <typename T, size_t Size>
size_t ringqueue<T, Size>::size(void){
    if(this->Back > this->Front){
        return this->Back - this->Front;
    }
    return Size + this->Back - this->Front;
}

template <typename T>
priqueue<T>::priqueue(void){
    this->first_mem = nullptr;
    this->last_mem = nullptr;
    this->Size = 0;
}

template <typename T>
const T& priqueue<T>::front(void){
    return this->first_mem->first;
}

template <typename T>
const T& priqueue<T>::back(void){
    return this->last_mem->first;
}

template <typename T>
Error priqueue<T>::pop(void){
    if(this->first_mem == nullptr){
        return MAKE_ERROR(Error::kEmpty);
    }
    std::pair<T,void*>* temp = this->first_mem;
    this->first_mem = reinterpret_cast<std::pair<T,void*>*>(this->first_mem->second);
    delete temp;
    this->Size--;
    return MAKE_ERROR(Error::kSuccess);
}

template <typename T>
Error priqueue<T>::push(const T& value){
    std::pair<T,void*>* temp = first_mem;
    if(!temp){
        auto tempbuf = new char[sizeof(std::pair<T,void*>)];
        auto newone = reinterpret_cast<std::pair<T,void*>*>(tempbuf);
        *newone = std::make_pair(value, nullptr);
        this->first_mem = newone;
        this->last_mem = newone;
    }
    for (int i = 0; i < this->Size; i++){
        if(value < temp->first){
            auto tempbuf = new char[sizeof(std::pair<T,void*>)];
            auto newone = reinterpret_cast<std::pair<T,void*>*>(tempbuf);
            newone->first = value;
            newone->second = temp->second;
            temp->second = newone;
            break;
        } else if(temp == this->last_mem){
            auto tempbuf = new char[sizeof(std::pair<T,void*>)];
            auto newone = reinterpret_cast<std::pair<T,void*>*>(tempbuf);
            newone->first = value;
            newone->second = nullptr;
            temp->second = newone;
            this->last_mem = newone;
        }
        temp = (std::pair<T,void*>*)(temp->second);
    }
    this->Size++;
    return MAKE_ERROR(Error::kSuccess);
}