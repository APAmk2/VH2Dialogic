#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <regex>

void convertDialog(const std::filesystem::path &txtFilePath);

std::string formatLine(std::string line);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: ./program_name <txtFilePath>" << std::endl;
        return 1;
    }

    std::filesystem::path txtFilePath = argv[1];

    convertDialog(txtFilePath);

    return 0;
}

void convertDialog(const std::filesystem::path &txtFilePath)
{
    std::ifstream inputFile(txtFilePath);
    std::ofstream outputFile((txtFilePath.stem() += "_out.txt"));

    if (inputFile.is_open() && outputFile.is_open())
    {
        std::string content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        // std::vector<std::string> dialogLines = extractDialogLines(content);

        std::vector<std::string> dialogLines;
        std::string line;
        std::istringstream iss(content);

        while (std::getline(iss, line))
        {

            dialogLines.push_back(formatLine(line));
        }

        for (size_t i = 0; i < dialogLines.size(); i++)
        {

            outputFile << dialogLines[i] << std::endl;
        }

        inputFile.close();
        outputFile.close();

        std::cout << "Conversion completed successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to open input or output file." << std::endl;
    }
}

std::string formatLine(std::string line)
{

    // Remove unnecessary characters or tags from each line
    // (e.g., [E:1], [STOPLIP:], [XS:jilltalk,1], [C:13], etc.)
    // You can use string manipulation functions or regular expressions for this.
    // Add the formatted dialog line to the dialogLines vector.
    std::string formattedLine = line;

    // Remove [E:1] tag
    // formattedLine = std::regex_replace(formattedLine, std::regex("\\[E:\\d+\\]"), "");

    // Remove [STOPLIP:] tag
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[STOPLIP:\\]"), "");

    // Remove [XS:dantalk,1] or [XS:dantalk,0] tag

    formattedLine = std::regex_replace(formattedLine, std::regex("\\[XS:(\\w+)hide,(\\d+)\\]"), "\nleave $1");
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[XS:mix,1\\]"), "\nset {Dialogic.paused} = true\n");
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[XS:[^\\]]+\\]"), "");

    // Remove [C:15] or [C:C] tag
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[C:\\d+\\]"), "");
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[C:C\\]"), "");
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[SHAKE:\\]"), "");
    formattedLine = std::regex_replace(formattedLine, std::regex("#"), "");
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[SHOW:\\]"), "");

    formattedLine = std::regex_replace(formattedLine, std::regex("\\[DOOR:\\]"), "\n[sound path=\"res://resources/Exported_Sounds/audiogroup_default/door.ogg\" volume=\"0.0\" bus=\"SFX\"]\n");
    formattedLine = std::regex_replace(formattedLine, std::regex("\\[BANG:\\]"), "\n[sound path=\"res://resources/Exported_Sounds/audiogroup_default/plomamentazon.ogg\" volume=\"0.0\" bus=\"SFX\"]\n");

    std::regex wait_re("\\[P:(\\d+)\\]");
    std::regex show_re("\\[SHOW:(\\d+),sprite_(\\w+)\\]");
    std::smatch match;

    if (std::regex_search(formattedLine, match, show_re))
    {
        std::string number_str = match[1]; // match[1] contains the number
        int number = std::stoi(number_str);
        if (number == 185)
        {
            number = 1;
        }
        else if (number < 185)
        {
            number = 2;
        }
        else if (number > 185)
        {
            number = 3;
        }

        formattedLine = std::regex_replace(formattedLine, show_re, "join $2 " + std::to_string(number) + "\n");
    }

    if (std::regex_search(formattedLine, match, wait_re))
    {
        std::string number_str = match[1]; // match[1] contains the number
        // float number = std::stof(number_str) / 30;

        formattedLine = std::regex_replace(formattedLine, wait_re, "\n[wait time=\"" + std::to_string(std::stof(number_str) / 30) + "\"]\n");
    }
    return formattedLine;
}
