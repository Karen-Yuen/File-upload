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
#include <vector>
#include <chrono> 
#include <map>
#include <mutex>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iomanip>

using namespace std;
mutex mtx;
//typedef function<void(int i)> CallBackFunc;
typedef function<void(string,int i)> CallBackFunc;
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

void displayFileInfo (string uploadFile) {
    ifstream file(uploadFile.c_str(), ios::binary|ios::ate);
    double fileSize = file.tellg();
    file.seekg(0, ios::beg);
    char filSig[9];
    file.read(filSig,8);
    string sizeUnit; 
    double formatedSize;
    if (fileSize<1024){
        formatedSize = fileSize;
        sizeUnit = "bytes";
    }
    if (fileSize>1024 && fileSize<1048576){
        formatedSize = fileSize/1024;
        sizeUnit = "KB";
    }
    if (fileSize>1048576){
        formatedSize = fileSize/1048576;
        sizeUnit = "MB";
    }
    cout << uploadFile << " | "<<formatedSize<<sizeUnit<< " | ";
    
    vector<short> myVector(8);
    copy ( filSig, filSig+7, myVector.begin() );
    for (vector<short>::iterator it = myVector.begin(); it!=myVector.end(); ++it){
        cout << ' ' <<hex<<uppercase<<setw(2)<<setfill('0')<< *it;
    }
    cout<<endl;
}

map<string, int> sentByte;

void progressReport(int i){ 
    cout<< "\e[A"<< i <<" byte is done."<<endl;
}

void ByteReport(string fileName, int i){ 
    sentByte[fileName] = i;
}

int copyFile (string source, string dest, CallBackFunc Report) { // callback progressReport
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
                //Report(i*4096);
                Report(source,i*4096);
            }   
            if(i==y+1){
                initialFile.read(buffer,x);
                outputFile.write(buffer,x);
                //Report((i-1)*4096+x);
                Report(source,(i-1)*4096+x);
            }
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

double count100MilliSec (){
    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    this_thread::sleep_for( chrono::milliseconds(100) ) ;
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    return time_span.count();
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
            cout << "Not exists directory " << argv[1] << "\n" << argv[1] << " is created.\n" ;    
        }
        result (argc, argv);
        cout<<"----------------------------------------------------------------------------"<<endl;
        cout << "Filename | Size | Header (first 8 bytes)" << endl; 
        string targetPath(argv[1]);
        vector <future<int>> copyAsync;
        for (int i = 2; i<argc; i++) {
            string uploadfile(argv[i]); 
            ifstream initialFile(uploadfile.c_str(), ios::in|ios::binary);
            if(initialFile.is_open()) { 
            string uploadfile(argv[i]); 
            displayFileInfo(uploadfile);
            }
        }
        mtx.lock();
        for (int i = 2; i<argc; i++) {
            
            string uploadfile(argv[i]); 
            string outputPath = targetPath + '/' + uploadfile;
            copyAsync.push_back(async(copyFile, uploadfile, outputPath, ByteReport));
        }
        cout<<dec<<"----------------------------------------------------------------------------"<<endl;
        using namespace std::chrono;
        high_resolution_clock::time_point startTime = high_resolution_clock::now();
        high_resolution_clock::time_point afterWhile;
        duration<double> time_span;                                                 //time passed
        vector <bool> Done (argc-2);                                                //check task done or not;
        int coutingOf3=1;
        for (int n = 1; n <= 6000; n++) {
            for (int i = 2; i<argc; i++) {
                if (copyAsync[i-2].wait_for(chrono::milliseconds(0))==std::future_status::timeout){
                    Done[i-2]=false;
                }
                else {
                    Done[i-2]=true;
                }
            }
            if ( find(Done.begin(), Done.end(), false) != Done.end() ){
                this_thread::sleep_for( chrono::milliseconds(100) );
            }
            else if ( find(Done.begin(), Done.end(), false) == Done.end() ){
                break;
            }
            afterWhile = high_resolution_clock::now();
            time_span = duration_cast<duration<double>>(afterWhile - startTime);
            if (time_span.count()>(0.3*coutingOf3)){
              for(auto iter = sentByte.begin(); iter != sentByte.end(); iter++){
                    cout<<iter->first<<" "<<iter->second<<'\n';
                }
              coutingOf3++;
            }
        }
        cout<<"--------------------\n"<<"Summary:" <<endl;                                                 
        for(auto iter = sentByte.begin(); iter != sentByte.end(); iter++){   //summary
                    cout<<iter->first<<" | "<<iter->second<<'\n';
        }
        cout << "Total Time: " << time_span.count() <<endl;
        mtx.unlock();  
    }
    return 0;
}