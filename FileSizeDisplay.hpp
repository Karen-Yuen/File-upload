/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FileSizeDisplay.hpp
 * Author: karenyuen
 *
 * Created on March 8, 2018, 1:17 PM
 */
#ifndef FILESIZEDISPLAY_HPP
#define FILESIZEDISPLAY_HPP

#include <string>

using namespace std; 

struct FileSizeDisplay {
    double fileSizeByte;
    string fileSizeUnit;
};

FileSizeDisplay formatedFileSize (double fileSize){
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
    return FileSizeDisplay{formatedSize, sizeUnit};
}


#endif /* FILESIZEDISPLAY_HPP */

