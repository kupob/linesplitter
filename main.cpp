#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <memory>

#include "buffer.h"

constexpr auto str_no_input =       "No input file provided\n";
constexpr auto str_no_output =      "No output file provided\n";
constexpr auto str_usage =          "Usage:\n"
                                    "    wordlist -i <inputfile> -o <outputfile> -r <rulesfile>\n"
                                    "Examples:\n"
                                    "    wordlist -i test.txt -o test_result.txt -r rules.txt";
constexpr auto std_error_file_r =   "Error:\n"
                                    "    Could not read file";
constexpr auto std_error_file_w =   "Error:\n"
                                    "    Could not write file";

std::string getCmdOption(char ** argv, int argc, const std::string & option)
{
    std::string result {};
    bool parseArgument = false;

    for (int i = 0; i < argc; ++i) {
        if (argv[i] == option)
            parseArgument = true;
        else if (strlen(argv[i]) == 2 && argv[i][0] == '-')
            parseArgument = false;
        else if (parseArgument) {
            if (!result.empty())
                result += ' ';
            result += argv[i];
        }
    }

    return result;
}

void print(const std::string& text)
{
    std::cout << text << std::endl;
}

using BufferPtr = std::shared_ptr<Buffer>;

void splitLines(const BufferPtr& buffer, const BufferPtr& bufferErrors, std::vector<std::ifstream>& files)
{
    std::string line {};

    for (auto& file : files) {
        file.clear();
        file.seekg(0);
        while (std::getline(file, line)) {
            if (auto pos = line.find("::", 0); pos != std::string::npos) {
                *buffer << line.substr(pos + 1);
                if (bufferErrors)
                    *bufferErrors << line;
            }
            else if (pos = line.find(':', 0); pos != std::string::npos) {
                *buffer << line.substr(pos + 1);
            }
            else if (pos = line.find(';', 0); pos != std::string::npos) {
                *buffer << line.substr(pos + 1);
                if (bufferErrors)
                    *bufferErrors << line;
            }
        }
    }
}

int main(int argc, char** argv) {
    std::string inputFilesPath = getCmdOption(argv, argc, "-i");
    std::string outputFilePath = getCmdOption(argv, argc, "-o");
    std::string errorsFilePath = getCmdOption(argv, argc, "-e");

    if (inputFilesPath.empty()) {
        print(str_no_input);
        print(str_usage);
        return -1;
    }
    if (outputFilePath.empty()) {
        print(str_no_output);
        print(str_usage);
        return -1;
    }

    std::istringstream iss(inputFilesPath);
    std::vector<std::string> fileNames { std::istream_iterator<std::string>{iss},
                                         std::istream_iterator<std::string>{} };

    std::vector<std::ifstream> inputFiles;

    BufferPtr buffer = std::make_shared<Buffer>(outputFilePath);
    if (!buffer->isValid()) {
        print(std::string(std_error_file_w) + " \"" + outputFilePath + '"');
        return -1;
    }

    BufferPtr bufferWithErrors = nullptr;
    if (!errorsFilePath.empty()) {
        bufferWithErrors = std::make_shared<Buffer>(errorsFilePath, false);
        if (!buffer->isValid()) {
            print(std::string(std_error_file_w) + " \"" + errorsFilePath + '"');
            return -1;
        }
    }

    int64_t lineCountTotal = 0;

    for (const auto& fileName : fileNames) {
        std::ifstream inputFile(fileName);
        if (!inputFile.is_open()) {
            print(std::string(std_error_file_r) + " \"" + fileName + '"');
            return -1;
        }
        inputFile.unsetf(std::ios_base::skipws);

        auto lineCount = std::count(std::istream_iterator<char>(inputFile),
                                    std::istream_iterator<char>(),
                                    '\n') + 1;
        lineCountTotal += lineCount;
        inputFile.clear();
        inputFile.seekg(0);

        inputFiles.push_back(std::move(inputFile));
    }

    buffer->setLineCountTotal(lineCountTotal);

    splitLines(buffer, bufferWithErrors, inputFiles);

    for (auto& inputFile : inputFiles) {
        inputFile.close();
    }
    buffer->close();
    if (bufferWithErrors)
        bufferWithErrors->close();

    print("done!");

    return 0;
}
