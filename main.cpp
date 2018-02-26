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
#include <string>
#include <sys/stat.h>
#include <functional>
#include <thread>
#include <future>

using namespace std;
typedef function<void(int i)> CallBackFunc;
/*
 * 
 */
void result (int argc, char* argv[]) {       
    cout << "Target URL: " << argv[1] << '\n'; 
    //count how many files be uploaded 
    unsigned int count = argc - 2 ;
    cout << "Total " << count << " files:\n";
    //output file names
    for (int i=2; i<argc; i++) {
    cout << argv[i] << '\n'; 
    }      
}

void progressReport(int i){ 
    cout<< "\e[A"<< i+1 << "byte is done";
    cout << '\n';
}

int copyFile (string source, string dest, function<void(int i)> Report) { // callback progressReport
    ifstream initialFile(source.c_str(), ios::in|ios::binary);
    if(initialFile.is_open()) {   
        initialFile.seekg(0, ios::end);
        streampos fileSize = initialFile.tellg();       //get the file size
        ofstream outputFile(dest.c_str(), ios::out|ios::binary);        //open out stream
        for(int i=0; i<fileSize; i++){
            initialFile.seekg(i,ios::beg);
            char * buffer = new char [1];
            initialFile.read(buffer,1);
            outputFile.write(buffer,1);
            delete[] buffer;
            Report(i);
            std::chrono::seconds sec(1);
            std::this_thread::sleep_for(sec);
        }
        initialFile.close();
        outputFile.close();
    }
    else{   
        cout<<"I couldn't open "<<source<<" for copying!\n";
	return 0;
    }
    return 1;
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: ./upload <target_url> <file1> <file2> ... <fileN>\n";
    }
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
            copyFile(uploadfile, outputPath, progressReport);
        }
        cout << "upload is completed.\n";
    }
    return 0;
}

