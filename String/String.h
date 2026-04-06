//
// Created by jianp on 2026/4/6.
//

#include <cstring>
#include <utility>

#ifndef CPPKIT_STRING_H
#define CPPKIT_STRING_H

class String {
public:
    String();
    explicit String(const char* str = nullptr);

    String(const String& other);
    String(String&& other) noexcept;

    String& operator=(const String& other);
    String& operator=(String&& other) noexcept;

    ~String();

private:
    char* m_data;
    int size;
};

String::String() : m_data(nullptr), size(0) {}

String::String(const char *str) {
    if(str == nullptr) {
        m_data = new char[1];
        *m_data = '\0';
        size = 0;
    }else {
        size = strlen(str);
        m_data = new char[size + 1];
        strcpy(m_data, str);
    }
}

String::~String() {
    delete[] m_data;
    m_data = nullptr;
}

String::String(const String &other) {
    size = other.size;
    m_data = new char[size + 1];
    strcpy(m_data, other.m_data);
}

String::String(String &&other) noexcept {
    m_data = other.m_data;
    size = other.size;
    other.m_data = nullptr;
    other.size = 0;
}

String &String::operator=(const String &other) {
    if(this == &other) {
        return *this;
    }

    String temp(other.m_data);
    std::swap(m_data, temp.m_data);
    std::swap(size, temp.size);

    return *this;
}

String& String::operator=(String &&other) noexcept {
    if(this == &other) {
        return *this;
    }

    delete[] m_data;
    m_data = other.m_data;
    size = other.size;

    other.m_data = nullptr;
    other.size = 0;

    return *this;
}

#endif //CPPKIT_STRING_H
