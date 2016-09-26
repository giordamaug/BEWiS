//
//  cwisard.hpp
//  
//
//  Created by Maurizio Giordano on 25/05/15.
//
//

#ifndef _cwisard_hpp
#define _cwisard_hpp

#include <stdio.h>
#include <list>
#include <iostream>
#include <sys/time.h>
#include <iostream>
#include <getopt.h>
#include <dirent.h>
#include <iomanip>

// OpenCV include
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>

#include <bgwis.hpp>

using namespace cv;
using namespace std;

int MAXWIDTH = 30;
char filler = ' ';
// String Tokenizer
vector<string> tokenizer(string str, char delimiter) {
    vector<string> v;
    istringstream buf(str);
    for(string token; getline(buf, token, delimiter); ) {
        v.push_back(token);
    }
    return v;
}

int parsePolicy(string settings,int &incr, int &decr) {
    vector<string> args = tokenizer(settings, ':') ;
    if (args.size() != 2) {
        return -1;
    } else {
        incr = atoi(args[0].c_str());
        decr = atoi(args[1].c_str());
        if (incr < 1 || decr < 0) {
            return -1;
        }
        return 0;
    }
}
#endif
