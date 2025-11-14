/**
 * @file stevensFileLib.hpp
 * @author your name (jeff@bucephalusstudios.com)
 * @brief A cool library for working with files
 * @version 0.1
 * @date 2024-02-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "stevensStringLib.h"
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <filesystem>

namespace stevensFileLib
{
    /**
     * @brief Given a filepath, opens a file and returns the input file stream object associated with the opened file.
     *        Performs checks during opening to make sure that the file exists.
     * 
     * @return std::ifstream - The input file stream object representing the file
     */
    std::ifstream openInputFile( const std::string & filePath )
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            //Error if we could not find the file
            throw std::invalid_argument("Failed to find file");
        }
        return file;
    }


    /**
     * @brief Given a path to a file, open the file for output, returning an output filestream object.
     * 
     * @return std::ofstream - The output file stream object representing the file
     */
    std::ofstream openOutputFile( const std::string & filePath )
    {
        std::ofstream file(filePath, std::ios::app);
        if (!file.is_open())
        {
            //Error if we could not find the file
            throw std::invalid_argument("Failed to find file");
        }
        return file;
    }


    /**
     * @brief Given a path to a file and some content to append to it, append the content to the end of the file.
     * 
     * @return void
     */
    template<typename ContentType>
    void appendToFile(  const std::string & filePath,
                        const ContentType & contentToAppend,
                        const bool createIfNonExistent = true )
    {
        //Open a file for output 
        std::ofstream file;
        try
        {
            file = openOutputFile( filePath );
        }
        catch( const std::invalid_argument& )
        {
            if ( createIfNonExistent )
            {
                //Create a new file if we can't find one
                std::ofstream newFile(filePath);
                if (!newFile)
                {
                    throw std::runtime_error("Failed to create file");
                }
                newFile << contentToAppend;
                newFile.close();
                return;
            } 
            else 
            {
                throw; // Re-throw the original exception
            }
        }
        
        //Append the contentToAppend
        file << contentToAppend;

        //Close the file
        file.close();
    }


    /**
     * @brief Given a file, load its contents line-by-line into a vector of strings.
     * 
     * @param filePath - The string path to the file that we want to load into a vector.
     * @param settings - An unordered_map cof strings keys and values of vectors of strings containing settings
     *                   to further specify how you want to load the file into the vector. 
     *                   Possible key-value pairs and their effects are:
     *                   "skip if starts with" : a vector of strings that we will check to see if a line begins with them. If a line begins with any in the vector, the line is skipped.
     *                   "skip if contains" : a vector of strings that we will check to see if they are contained within the line. If a line contains any in the vector, the line is skipped.
     * @param separator - A character that we will use to separate the lines of the file.
     * @param skipEmptyLines - If true, skip lines that are empty.
     * 
     * @return std::vector<std::string> A vector of strings with the data from the file pushed back into it, line-by-line.
     */
    std::vector<std::string> loadFileIntoVector(    const std::string & filePath,
                                                    std::unordered_map< std::string, std::vector<std::string> > settings = {},
                                                    const char separator = '\n',
                                                    bool skipEmptyLines = true  )
    {
        //Open the file with safety checks
        std::ifstream file = stevensFileLib::openInputFile(filePath);

        std::vector<std::string> vec; 
        std::string line;
        bool skip;
        //Iterate through the file by pushing each individual line into vec
        while(getline(file, line, separator))
        {
            skip = false;
            //Check to see if we can add the line to the vector
            if(skipEmptyLines)
            {
                if(line.empty())
                {
                    continue;
                }
            }
            if(settings.contains("skip if starts with"))
            {
                for(int i = 0; i < settings["skip if starts with"].size(); i++)
                {
                    if(stevensStringLib::startsWith(line, settings["skip if starts with"][i]))
                    {
                        skip = true;
                        break;
                    }
                }
                if(skip)
                {
                    continue;
                }
            }
            if(settings.contains("skip if contains"))
            {
                for(int i = 0; i < settings["skip if contains"].size(); i++)
                {
                    if(stevensStringLib::contains(line, settings["skip if contains"][i]))
                    {
                        skip = true;
                        break;
                    }
                }
                if(skip)
                {
                    continue;
                }
            }

            //Add the line to the vector
            vec.push_back(line);
        }

        file.close();

        //Return the vector, now full of content from the file
        return vec;
    }


    /**
     * @brief Given a file, load its contents line-by-line into a vector of ints.
     * 
     * @param filePath - The string path to the file that we want to load into a vector.
     * @param settings - An ma of string keys and values of vectors of strings containing settings
     *                   to further specify how you want to load the file into the vector. 
     * @param skipEmptyLines - If true, skip lines that are empty.
     */
    std::vector<int> loadFileIntoVectorOfInts(  const std::string & filePath,
                                                const std::unordered_map< std::string, std::vector<std::string> > settings = {},
                                                const char separator = '\n',
                                                const bool skipEmptyLines = true)
    {

        //Open the file with safety checks
        std::ifstream file = stevensFileLib::openInputFile( filePath );

        //Declare the vector that we'll be pushing our integer data back into from the file
        std::vector<int> vec;
        //As well as the variable that holds the integer data
        int data;
        //Use the stream operator to take each line of the file and load it into vec
        while(file >> data)
        {
            vec.push_back(data);
        }

        //Complete the cleanup by closing the file
        file.close();

        //Return the vector, now full of content from the file
        return vec;
    }


    /**
     * @brief Given a file name, return the contents of a random line in the named file.
     * 
     * @param filePath The path of the file we want to get the content of a random line from.
     * @param separator The character used to separate lines of the file.
     * 
     * @return A std::string variable containing a random line of file content
     */
    std::string getRandomFileLine(  const std::string & filePath,
                                    const char & separator = '\n' )
    {
        //Load the file into a vector
        std::vector<std::string> fileLines = loadFileIntoVector(filePath, {}, separator, false);
        //Get a random line number in the range of 0 to the line count
        unsigned long long int randomLineNumber = rand() % fileLines.size();
        //Return the content of the random line
        return fileLines[randomLineNumber];
    }


    /**
     * @brief Given a directory path, return a vector of all of the file names in the directory.
     */
    std::vector<std::string> listFiles( const std::string & directoryPath,
                                        const std::unordered_map<std::string, std::string> & settings = {   { "targetFileExtensions",   ""  },
                                                                                                            { "excludeFileExtensions",  ""  },
                                                                                                            { "excludeFiles",  ""           }   } )
    {
        //Check if the directory exists
        if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath))
        {
            throw std::invalid_argument("Directory does not exist: " + directoryPath);
        }

        //Based on the values of the settings map, determine what file extensions we want to include and exclude
        std::unordered_set<std::string> targetFileExtensions;
        if( settings.contains("targetFileExtensions") )
        {
            if( !settings.at("targetFileExtensions").empty() )
            {
                //If the target file extensions are not empty, we will only include files with those extensions
                std::vector<std::string> targetFileExtensionsVec = stevensStringLib::separate(settings.at("targetFileExtensions"), ",");
                targetFileExtensions = std::unordered_set<std::string>(targetFileExtensionsVec.begin(), targetFileExtensionsVec.end());
            }
        }
        std::unordered_set<std::string> excludeFileExtensions;
        if( settings.contains("excludeFileExtensions") )
        {
            if( !settings.at("excludeFileExtensions").empty() )
            {
                //If the exclude file extensions are not empty, we will only include files with those extensions
                std::vector<std::string> excludeFileExtensionsVec = stevensStringLib::separate(settings.at("excludeFileExtensions"), ",");
                excludeFileExtensions = std::unordered_set<std::string>(excludeFileExtensionsVec.begin(), excludeFileExtensionsVec.end());
            }
        }
        //Check if we have any files to exclude
        std::unordered_set<std::string> excludeFiles;
        if( settings.contains("excludeFiles") )
        {
            if( !settings.at("excludeFiles").empty() )
            {
                //If we've specified files to exclude, set that up here
                std::vector<std::string> excludeFilesVec = stevensStringLib::separate(settings.at("excludeFiles"), ",");
                excludeFiles = std::unordered_set<std::string>(excludeFilesVec.begin(), excludeFilesVec.end());
            }
        }

        //Iterate through the directory and get all of the files
        std::vector<std::string> fileNames;
        for (const auto & entry : std::filesystem::directory_iterator(directoryPath))
        {
            if (entry.is_regular_file())
            {
                //First, we see if this is a particular file we want to exclude from our listing
                if( !excludeFiles.empty() )
                {
                    //If we have files to exclude, check if the current file is in the excludeFiles set
                    if( excludeFiles.contains(entry.path().filename().string()) )
                    {
                        continue; //Skip this file
                    }
                }
                //Now we know this file is a candidate for inclusion in our list, check its file extension
                if( !targetFileExtensions.empty() )
                {
                    //If we have target file extensions, check if the current file does not have one of them
                    std::string fileExtension = entry.path().extension().string();
                    if( !targetFileExtensions.contains(fileExtension) )
                    {
                        continue; //Skip this file -- It doesn't have a target file extension
                    }
                }
                else if( !excludeFileExtensions.empty() )
                {
                    //If we have exclude file extensions, check if the current file has one of them
                    std::string fileExtension = entry.path().extension().string();
                    if( excludeFileExtensions.contains(fileExtension) )
                    {
                        continue; //Skip this file -- It has an excluded file extension
                    }
                }
                
                fileNames.push_back(entry.path().filename().string());
            }
        }
        
        return fileNames;
    }


    // /**
    //  * Given the path to a file, count how many lines are in the file and return the integer count.
    //  * 
    //  * @param filePath - The path to the file we want to count the number of lines of.
    //  * 
    //  * @retval int - The integer number of lines that the file contains.
    // */
    // inline int countFileLines(const std::string & filePath)
    // {
    //     //Create an input stream from the file we are trying to print
    //     std::ifstream input_file(filePath);
    //     if (!input_file.is_open())
    //     {
    //         //Error if we cannot successfully print the file
    //         throw std::invalid_argument("Error, could not find file: " + filePath);
    //     }
    //     //Store the text content of the file in a string
    //     std::string fileContent = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    //     input_file.close();

    //     return countLines(fileContent);
    // }

}