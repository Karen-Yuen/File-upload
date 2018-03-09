/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SentingReport.hpp
 * Author: karenyuen
 *
 * Created on March 8, 2018, 2:49 PM
 */

#ifndef SENTINGREPORT_HPP
#define SENTINGREPORT_HPP

using namespace std; 

struct SentingReport {
    int sentByte;
    std::chrono::high_resolution_clock::time_point sentTime;
};

#endif /* SENTINGREPORT_HPP */

