/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: karenyuen
 *
 * Created on February 20, 2018, 9:34 AM
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

/*
 * 
 */
void result (int argc, char* argv[])

{       cout << "Target URL: " << argv[1] << '\n'; 
        
        //count how many files be uploaded 
        unsigned int count = argc - 2 ;
        cout << "Total " << count << " files:\n";
        
        //output file names
        for (int i=2; i<argc; i++) {
        cout << argv[i] << '\n'; 
        }
        
}


int copy (string source, string dest) // callback progressReport
{
        ifstream initialFile(source.c_str(), ios::in|ios::binary);
	ofstream outputFile(dest.c_str(), ios::out|ios::binary);
	//defines the size of the buffer
	initialFile.seekg(0, ios::end);
	long fileSize = initialFile.tellg();
	//Requests the buffer of the predefined size
           
        if(initialFile.is_open() && outputFile.is_open())
	{         
		short * buffer = new short[fileSize/2];
		//Determine the file's size
		//Then starts from the beginning
		initialFile.seekg(0, ios::beg);
		//Then read enough of the file to fill the buffer
		initialFile.read((char*)buffer, fileSize);
		//And then write out all that was read
		outputFile.write((char*)buffer, fileSize);
		delete[] buffer;
	}
        
        else if(!initialFile.is_open())
	{
		cout<<"I couldn't open "<<source<<" for copying!\n";
		return 0;
	}
        
        else if(!outputFile.is_open())
	{
		cout<<"I couldn't open "<<dest<<" for copying!\n";
		return 0;
	}
       
	initialFile.close();
	outputFile.close();

	return 1;
}

int main(int argc, char* argv[]) {
    
    if (argc < 3) {cout << "Usage: ./upload <target_url> <file1> <file2> ... <fileN>\n";}
    else {
        
        struct stat statStruct;
        stat(argv[1], &statStruct);
        
        if(S_ISDIR(statStruct.st_mode)) {
        cout << "exists directory " << argv[1] <<endl;
        }
        
        else {
                mkdir(argv[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                cout << "Not exists directory " << argv[1] << "\n" << argv[1] << " is created\n" ;
               
        }
           result (argc, argv);
           
           string targetPath(argv[1]);
           for (int i = 2; i<argc; i++) {
               string uploadfile(argv[i]); 
               string outputPath = targetPath + '/' + uploadfile;  
               copy(uploadfile , outputPath);
           }
           
    }
  
	return 0;
}

