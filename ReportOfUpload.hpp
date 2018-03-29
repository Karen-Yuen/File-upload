/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ReportOfUpload.hpp
 * Author: karenyuen
 *
 * Created on March 12, 2018, 11:57 AM
 */

#ifndef REPORTOFUPLOAD_HPP
#define REPORTOFUPLOAD_HPP

#include <map>
#include <mutex>
#include "SentingReport.hpp"

using namespace std; 
using namespace std::chrono;
typedef map<string, SentingReport> ProgressReport;

class ReportOfUpload {
    public:
        void writeProgressReport(string const& fileName, int i){
            lock_guard<mutex> mLock(mtx);
            report[fileName]={i, std::chrono::high_resolution_clock::now()};
        }
        ProgressReport currentReport (){
            lock_guard<mutex> mLock(mtx);
            return report;
        }
        double currentCopySize (string const& fileName) {
            lock_guard<mutex> mLock(mtx);
            auto iter = report.find(fileName);
            return iter != report.end() ? (iter->second).sentByte : 0;
        }
        
    private:   
        mutex mtx;
        ProgressReport report;
        
};


#endif /* REPORTOFUPLOAD_HPP */

