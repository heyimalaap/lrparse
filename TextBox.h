#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

class TextBox {
public:
    struct xy {
        TextBox& tb;
        int x, y;

        xy operator<< (const std::string& s) const {
            return {tb, x + tb.puts(x, y, s), y};
        }

        xy operator<< (const int i) const {
            std::string s = std::to_string(i);
            return {tb, x + tb.puts(x, y, s), y};
        }

        xy operator<< (const char c) const {
            tb.putc(x, y, c);
            return {tb, x + 1, y};
        }

        xy operator<< (const TextBox& _tb) const {
            tb.puttb(x, y, _tb);
            return {tb, x + _tb.width(), y};
        }
    };

    std::string str() const {
        std::stringstream ss;
        for (auto& s : m_buffer) {
            //ss << s << '\n';
            for (char c : s) {
                if (c > 0b1111)
                    ss << c;
                else {
                    switch (static_cast<int>(c) & 0b1111) {
                        case 0b01:
                            ss << '-';
                            break;
                        case 0b10:
                            ss << '|';
                            break;
                        case 0b11:
                            ss << '+';
                            break;
                        default:
                            ss << c;
                    }
                }
            }
            ss << '\n';
        }
        return ss.str();
    }
    
    TextBox::xy operator() (int x, int y) const {
        return {const_cast<TextBox&>(*this), x, y};
    }
    

    int width() const {
        int w = -1;
        for (auto& s : m_buffer) 
            w = std::max(w, static_cast<int>(s.size()));
        return w;
    }

    int height() const {
        return m_buffer.size();
    }

    void hline(int x, int y, int width) {
        if (y >= m_buffer.size())
            m_buffer.resize(y+1);
        if (x+width >= m_buffer[y].size())
            m_buffer[y].resize(x+width, ' ');

        for (int i = 0; i < width; i++) {
            if (m_buffer[y][x+i] > 0b1111)
                m_buffer[y][x+i] = 0;
            m_buffer[y][x+i] |= 0b1;
        }
    }

    void vline(int x, int y, int height) {
        if (y+height >= m_buffer.size())
            m_buffer.resize(y+height);
        for (int i = 0; i < height; i++) {
            if (x >= m_buffer[y+i].size())
                m_buffer[y+i].resize(x+1, ' ');
            if (m_buffer[y+i][x] > 0b1111)
                m_buffer[y+i][x] = 0;
            m_buffer[y+i][x] |= 0b10;
        }
    }

private:

    void putc(int x, int y, char c) {
        if (y >= m_buffer.size())
            m_buffer.resize(y+1);
        if (x >= m_buffer[y].size())
            m_buffer[y].resize(x+1, ' ');
        m_buffer[y][x] = c;
    }
    
    int puts(int x, int y, const std::string& s) {
        if (s.empty()) return x;
        for (int i = 0; i < s.size(); i++)
            putc(x+i, y, s[i]);
        return x + s.size();
    }
    
    void puttb(int x, int y, const TextBox& tb) {
        for (int i = 0; i < tb.height(); i++) {
            puts(x, y+i, tb.m_buffer[i]);
        }
    }

private:
    std::vector<std::string> m_buffer;
};
