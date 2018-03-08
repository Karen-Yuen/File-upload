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

struct fileSizeDisplay {
    double fileSizeByte;
    string fileSizeUnit;
};
 
fileSizeDisplay formatedFileSize (double fileSize){
    double formatedSize;
    string sizeUnit; 
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
    return fileSizeDisplay{formatedSize, sizeUnit};
}

void displayFileInfo (string uploadFile) {
    ifstream file(uploadFile.c_str(), ios::binary|ios::ate);
    double fileSize = file.tellg();
    file.seekg(0, ios::beg);
    char filSig[9];
    file.read(filSig,8);
    fileSizeDisplay result = formatedFileSize(fileSize);
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

struct sentingReport {
    int sentByte;
    std::chrono::high_resolution_clock::time_point sentTime;
};

map<string, sentingReport> progressReport;

void sentProgressReport(string fileName, int i){ 
    progressReport[fileName] = {i, std::chrono::high_resolution_clock::now()};
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
                Report(source,i*4096);
            }   
            if(i==y+1){
                initialFile.read(buffer,x);
                outputFile.write(buffer,x);
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

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    if (argc < 3) {                         //check argument count
        cout << "Usage: ./upload <target_url> <file1> <file2> ... <fileN>\n";
    }
    else {
        struct stat statStruct;
        stat(argv[1], &statStruct);
        if(S_ISDIR(statStruct.st_mode)) {                                 //check directory
            cout << "exists directory " << argv[1] <<endl;
        }
        else {                                                              //make directory
            mkdir(argv[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            cout << "Not exists directory " << argv[1] << "\n" << argv[1] << " is created.\n" ;    
        }
        result (argc, argv);
        cout<<"----------------------------------------------------------------------------"<<endl;
        cout << "Filename | Size | Header (first 8 bytes)" << endl; 
        string targetPath(argv[1]);
        vector <future<int>> copyAsync;
        for (int i = 2; i<argc; i++) {                      
            string uploadfile(argv[i]);                                                 //confirm file valid
            ifstream initialFile(uploadfile.c_str(), ios::in|ios::binary);
            if(initialFile.is_open()) { 
            string uploadfile(argv[i]); 
            displayFileInfo(uploadfile);                                                //display file info
            }
        }
        mtx.lock();
        high_resolution_clock::time_point startTime = high_resolution_clock::now();             //timer started
        for (int i = 2; i<argc; i++) {
            
            string uploadfile(argv[i]); 
            string outputPath = targetPath + '/' + uploadfile;
            copyAsync.push_back(async(copyFile, uploadfile, outputPath, sentProgressReport));
        }
        cout<<dec<<"----------------------------------------------------------------------------"<<endl;
        high_resolution_clock::time_point afterWhile;                               //set timer to count the report interval; 
        duration<double> time_span;                                                 //count passed time 
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
                afterWhile = high_resolution_clock::now(); 
            }
            else if ( find(Done.begin(), Done.end(), false) == Done.end() ){
                afterWhile = high_resolution_clock::now(); 
                break;
            }
            time_span = duration_cast<duration<double>>(afterWhile - startTime);
            if (time_span.count()>(0.3*coutingOf3)){
                for(auto iter = progressReport.begin(); iter != progressReport.end(); iter++){
                    fileSizeDisplay result = formatedFileSize((iter->second).sentByte); 
                    cout<<iter->first<<" "<<result.fileSizeByte<<result.fileSizeUnit<<'\n';
                }
              coutingOf3++;
            }
        }
        cout<<"--------------------\n"<<"Summary:" <<endl;                                                                         
        for(auto iter = progressReport.begin(); iter != progressReport.end(); iter++){   //summary
            fileSizeDisplay result = formatedFileSize((iter->second).sentByte);        
            cout<<iter->first<<" | "<<result.fileSizeByte<<result.fileSizeUnit<<" | ";
            duration<double> time_span = duration_cast<duration<double>>((iter->second).sentTime - startTime);
            cout<<time_span.count()<<" sec"<<endl;
        }
        cout << "Time Taken: " << time_span.count()<<" sec" <<endl;
        mtx.unlock();  
    }
    return 0;
}