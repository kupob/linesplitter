
#pragma once

#include <fstream>
#include <ostream>

class Buffer
{
public:
    Buffer(const std::string& outputFileName, bool log = true)
        : m_outputFile(outputFileName, std::ios::trunc)
        , m_log(log)
    {

    }

    bool isValid() const
    {
        return m_outputFile.is_open();
    }

    void close()
    {
        if (!m_buff.empty()) {
            m_outputFile << m_buff;
            m_buff.clear();
            m_buffLines = 0;
        }

        m_outputFile.close();
    }

    void setLineCountTotal(int64_t lineCountTotal)
    {
        m_lineCountTotal = lineCountTotal;
        if (!m_lineCountTotal)
            m_lineCountTotal = 1;
        m_logPeriod = std::max(std::min(lineCountTotal / 10, m_buffLinesMax), int64_t(1000));
    }

    Buffer& operator<<(const std::string& str);

private:
    void pushBuffer()
    {
        if (m_buffLines >= m_buffLinesMax) {
            m_outputFile << m_buff;
            m_buff.clear();
            m_buffLines = 0;
        }
    }

    void print(const std::string& text)
    {
        std::cout << text << std::endl;
    }

private:
    std::ofstream m_outputFile;
    bool m_log { true };

    std::string m_buff;

    int m_buffLines { 0 };
    const int64_t m_buffLinesMax { 1000000 };
    int64_t m_logPeriod { 0 };
    int64_t m_lineCountTotal { 0 };
    int64_t m_lineId { 0 };
    int64_t m_lastLoggedLine { 0 };
};

template<class T>
std::string withCommas(T value)
{
    auto s = std::to_string(value);

    int n = s.length() - 3;
    int end = (value >= 0) ? 0 : 1; // Support for negative numbers
    while (n > end) {
        s.insert(n, ",");
        n -= 3;
    }

    return s;
}

Buffer& Buffer::operator<<(const std::string& str)
{
    m_buff += str;
    if (m_log) {
        if (m_lineId - m_lastLoggedLine >= m_logPeriod) {
            print(withCommas(m_lineId) + " of " + withCommas(m_lineCountTotal) +
                  " | " +
                  std::to_string(m_lineId * 100 / m_lineCountTotal) + "%");
            m_lastLoggedLine = m_lineId;
        }
        ++m_lineId;
    }
    ++m_buffLines;
    pushBuffer();
    return *this;
}