#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <regex>

void convertDialog(const std::filesystem::path &txtFilePath);

std::string formatLine(std::string line);

std::string conditionFind(int n);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ./program_name <txtFilePath> <gml_Script_FilePath> (more than one gml_Script_FilePath is accepted)" << std::endl;
        return 1;
    }

    std::filesystem::path txtFilePath = argv[1];

    std::ofstream outFile("tmp.gml");

    if (!outFile)
    {
        std::cerr << "Failed to open output file." << std::endl;
        return 1;
    }

    for (int i = 2; i < argc; i++)
    {
        std::ifstream inFile(argv[i]);
        if (!inFile)
        {
            std::cerr << "Failed to open input file: " << argv[i] << std::endl;
            continue;
        }

        outFile << inFile.rdbuf();
        inFile.close();
    }

    outFile.close();

    convertDialog(txtFilePath);
    if (remove("tmp.gml") != 0)
    {
        std::cerr << "Failed to delete tmp file." << std::endl;
        return 1;
    }
    return 0;
}

void convertDialog(const std::filesystem::path &txtFilePath)
{
    std::ifstream inputFile(txtFilePath);
    std::ofstream outputFile((txtFilePath.stem() += "_out.txt"));

    if (inputFile.is_open() && outputFile.is_open())
    {
        std::string content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

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

    // Remove [STOPLIP:] tag
    line = std::regex_replace(line, std::regex("\\[STOPLIP:\\]"), "");

    // Remove [XS:dantalk,1] or [XS:dantalk,0] tag

    line = std::regex_replace(line, std::regex("\\[XS:(\\w+)hide,\\d+\\]"), "\nleave $1");
    line = std::regex_replace(line, std::regex("\\[XS:mix,1\\]"), "\nset {Dialogic.paused} = true\n");
    line = std::regex_replace(line, std::regex("\\[XS:[^\\]]+\\]"), "");

    // Remove [C:15] or [C:C] tag
    line = std::regex_replace(line, std::regex("\\[C:\\d+\\]"), "");
    line = std::regex_replace(line, std::regex("\\[C:C\\]"), "");
    line = std::regex_replace(line, std::regex("\\[SHAKE:\\]"), "");
    line = std::regex_replace(line, std::regex("#"), "");
    line = std::regex_replace(line, std::regex("\\[SHOW:\\]"), "");

    line = std::regex_replace(line, std::regex("\\[DOOR:\\]"), "\n[sound path=\"res://resources/Exported_Sounds/audiogroup_default/door.ogg\" volume=\"0.0\" bus=\"SFX\"]\n");
    line = std::regex_replace(line, std::regex("\\[BANG:\\]"), "\n[sound path=\"res://resources/Exported_Sounds/audiogroup_default/plomamentazon.ogg\" volume=\"0.0\" bus=\"SFX\"]\n");

    std::smatch match;

    std::regex show_re("\\[SHOW:(\\d+),sprite_(\\w+)\\]");
    if (std::regex_search(line, match, show_re))
    {
        std::string number_str = match[1]; // match[1] contains the number
        int number = std::stoi(match[1]);
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

        line = std::regex_replace(line, show_re, "join $2 " + std::to_string(number) + "\n");
    }

    std::regex wait_re("\\[P:(\\d+)\\]");
    if (std::regex_search(line, match, wait_re))
    {
        std::string number_str = match[1]; // match[1] contains the number
        // float number = std::stof(number_str) / 30;

        line = std::regex_replace(line, wait_re, "\n[wait time=\"" + std::to_string(std::stof(number_str) / 30) + "\"]\n");
    }

    std::regex e_re("\\[E:(\\d+)\\]");
    if (std::regex_search(line, match, e_re))
    {
        line = std::regex_replace(line, e_re, "\n" + conditionFind(std::stoi(match[1])));
    }

    return line;
}

std::string conditionFind(int n)
{
    std::ifstream file("tmp.gml");
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line))
    {
        lines.push_back(line);
        if (line.find("textbox_create(global.demo, " + std::to_string(n) + ", 1)") != std::string::npos)
        {
            break;
        }
    }
    std::vector<std::string>::reverse_iterator rit = lines.rbegin();
    std::string::size_type pos;
    bool casebool = false;
    std::string condition;
    while (rit != lines.rend())
    {
        line = *rit;

        pos = line.find("else if (");
        if (pos == std::string::npos)
        {
            pos = line.find("if (");
        }
        if (pos == std::string::npos)
        {
            pos = line.find("else");
        }
        if (pos == std::string::npos)
        {
            pos = line.find("case \"");
            casebool = true;
        }
        if (pos == std::string::npos)
        {
            rit++;
            continue;
        }

        if (!casebool)
        {
            condition = line.substr(pos, line.find(")") - pos + 1);
        }
        else
        {
            condition = line.substr(pos, line.find(":") - pos + 1);
        }
        break;
    }

    condition = std::regex_replace(condition, std::regex("global.(\\w+)(\\d+)"), "{GlobalVars.$1\[$2\]}");
    condition = std::regex_replace(condition, std::regex("global.(\\w+)"), "{GlobalVars.$1}");
    
    return condition;
}
