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
#include <future>
#include <vector>
#include <chrono> 
#include <map>
#include <mutex>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <thread>
#include "FileSizeDisplay.hpp"
#include "SentingReport.hpp"

using namespace std;
typedef map<string, SentingReport> progressReport;
typedef function<void(string fileName,int i,progressReport &fileProgressReport)> callBackFunc;
/*
 * 
 */
void result (string targetURL, int fileCount, vector <string> filename) {       
    cout << "Target URL: " << targetURL << '\n'; 
    cout << "Total " << fileCount << " files:\n";
    for (vector<string>::iterator it = filename.begin() ; it != filename.end(); ++it) {                            //output file names
        cout << *it << '\n'; 
    } 
}
void displayFileInfo (string uploadFile) {
    ifstream file(uploadFile.c_str(), ios::binary|ios::ate);
    double fileSize = file.tellg();
    file.seekg(0, ios::beg);
    char filSig[9];
    file.read(filSig,8);
    FileSizeDisplay result = formatedFileSize(fileSize);
    cout << uploadFile << " | "<<result.fileSizeByte<<result.fileSizeUnit<< " |";
    vector<int> myVector (filSig , filSig+8);
    for (vector<int>::iterator it = myVector.begin(); it!=myVector.end(); ++it){
        if (*it<0){
            *it=*it+256;
        }
        cout << ' ' <<hex<<uppercase<<setw(2)<<setfill('0')<<(short) *it;
    }    
    cout<<endl;
}

void sentProgressReport(string fileName, int i, progressReport &fileProgressReport){ 
    fileProgressReport[fileName] = {i, std::chrono::high_resolution_clock::now()};
}

int copyFile (string source, string dest, callBackFunc report, progressReport &fileProgressReport) { // callback progressReport
    ifstream initialFile(source.c_str(), ios::in|ios::binary);
    if(initialFile.is_open()) {   
        initialFile.seekg(0, ios::end);
        streampos fileSize = initialFile.tellg();       //get the file size
        ofstream outputFile(dest.c_str(), ios::out|ios::binary);        //open out stream
        char buffer [4096];
        initialFile.seekg(0,ios::beg);
        int y = fileSize/4096;
        int x = fileSize%4096; 
        for(int i=1; i<=y+1; i++) {
            if(i<=y){
                initialFile.read(buffer,4096);
                outputFile.write(buffer,4096);
                report(source,i*4096, fileProgressReport);
            }   
            if(i==y+1){
                initialFile.read(buffer,x);
                outputFile.write(buffer,x);
                report(source,(i-1)*4096+x, fileProgressReport);
            }
        }
        initialFile.close();
        outputFile.close();
    }   
    else{   
        cout<<source<<" cannot be opened for copying!\n";
	return 0;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    if (argc < 3) {                         //check argument count
        cout << "Usage: ./upload <target_url> <file1> <file2> ... <fileN>\n";
    }
    else {
        string targetURL(argv[1]);
        int fileCount = argc - 2;
        vector <string> fileName (argv+2, argv+argc);
        struct stat statStruct;
        stat(argv[1], &statStruct);
        if(S_ISDIR(statStruct.st_mode)) {                                 //check directory
            cout << "exists directory " << targetURL <<endl;
        }
        else {                                                              //make directory
            mkdir(argv[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            cout << "Not exists directory " << targetURL << "\n" << targetURL << " is created.\n" ;    
        }
        result (targetURL, fileCount ,fileName);
        cout<<"----------------------------------------------------------------------------"<<endl;
        cout << "Filename | Size | Header (first 8 bytes)" << endl; 
        for (int i = 0; i<fileCount; i++) {                                               //confirm file valid
            ifstream initialFile(fileName[i].c_str(), ios::in|ios::binary);
            if(initialFile.is_open()) { 
            displayFileInfo(fileName[i]);                                                //display file info
            }
        }
        progressReport fileProgressReport;
        vector <future<int>> copyAsync;
        mutex mtx;
        mtx.lock();
        high_resolution_clock::time_point startTime = high_resolution_clock::now();             //timer started
        for (int i = 0; i<fileCount; i++) {
            string outputPath = targetURL + '/' + fileName[i];
            copyAsync.push_back(async(copyFile, fileName[i], outputPath, sentProgressReport, ref(fileProgressReport)));
        }
        cout<<dec<<"----------------------------------------------------------------------------"<<endl;
        cout<<"Progress Report:"<<endl;
        high_resolution_clock::time_point afterWhile;                               //set timer to check progress/ count the report interval; 
        duration<double> time_span;                                                 //count passed time 
        vector <bool> Done (fileCount);                                                //check task done or not;
        int coutingOf3=1;
        for (int n = 1; n <= 6000; n++) {
            for (int i = 0; i<fileCount; i++) {
                if (copyAsync[i].wait_for(chrono::milliseconds(0))==std::future_status::timeout){
                    Done[i]=false;
                }
                else {
                    Done[i]=true;
                }
            }
            if ( find(Done.begin(), Done.end(), false) != Done.end() ){
                this_thread::sleep_for( chrono::milliseconds(100) );
                afterWhile = high_resolution_clock::now(); 
            }
            else if ( find(Done.begin(), Done.end(), false) == Done.end() ){
                afterWhile = high_resolution_clock::now(); 
                cout<<"Upload Completed!"<<endl;
                break;
            }
            time_span = duration_cast<duration<double>>(afterWhile - startTime);
            
            if (time_span.count()>(0.3*coutingOf3)){
                for(auto iter = fileProgressReport.begin(); iter != fileProgressReport.end(); iter++){
                    FileSizeDisplay result = formatedFileSize((iter->second).sentByte); 
                    cout<<iter->first<<" "<<result.fileSizeByte<<result.fileSizeUnit<<'\n';
                }
              coutingOf3++;
            }
        }
        cout<<"--------------------\n"<<"Summary:" <<endl;                                                                         
        for(auto iter = fileProgressReport.begin(); iter != fileProgressReport.end(); iter++){   //summary
            FileSizeDisplay result = formatedFileSize((iter->second).sentByte);        
            cout<<iter->first<<" | "<<result.fileSizeByte<<result.fileSizeUnit<<" | ";
            duration<double> time_span = duration_cast<duration<double>>((iter->second).sentTime - startTime);
            cout<<time_span.count()<<" sec"<<endl;
        }
        cout << "Time Taken: " << time_span.count()<<" sec" <<endl;
        mtx.unlock();
    }
    return 0;
}