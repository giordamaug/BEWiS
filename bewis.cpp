#include <bewis.hpp>

using namespace std;
using namespace cv;

char buffer[1024];
const string WinTitle = "BeWIS - BG Estimator";

// Colors
Scalar bgcolor = Scalar(106,117,181);
// Gui settings

void rgb2rgb(Mat in, Mat &out) {
    in.copyTo(out);
}

void rgb2lab(Mat in, Mat &out) {
    cvtColor(in, out, CV_BGR2Lab);
}

void rgb2hsv(Mat in, Mat &out) {
    cvtColor(in, out, CV_BGR2HSV);
}

void lab2rgb(Mat in, Mat &out) {
    cvtColor(in, out, CV_Lab2BGR);
}

void hsv2rgb(Mat in, Mat &out) {
    cvtColor(in, out, CV_HSV2BGR);
}

void rgb2gray(Mat in, Mat &out) {
    cvtColor(in, out, CV_BGR2GRAY);
}

void lab2gray(Mat in, Mat &out) {
    cvtColor(in, out, CV_Lab2BGR);
    cvtColor(out, out, CV_BGR2GRAY);
}

void hsv2gray(Mat in, Mat &out) {
    cvtColor(in, out, CV_HSV2BGR);
    cvtColor(out, out, CV_BGR2GRAY);
}

int getdir(string dir, vector<string> &files, string ext) {
    DIR *dp;
    struct dirent *dirp;
    int cnt=0;
    string fn;
    
    if((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << -1 << ") wrong dir " << dir << endl;
        return -1;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name[0] != '.') {  /* ignore hidden files */
            fn = string(dirp->d_name);
            if(fn.substr(fn.find_last_of(".") + 1) == ext) {
                files.push_back(fn);
                cnt++;
            }
        }
    }
    closedir(dp);
    if (cnt == 0) {
        cout << "Error(" << -2 << ") empty dir " << dir << endl;
        return -2;
    }
    return cnt;
}

Mat& discretize(Mat& I, int dimTics) {
    // accept only char type matrices
    CV_Assert(I.depth() != sizeof(uchar));
    
    int channels = I.channels();
    
    int nRows = I.rows;
    int nCols = I.cols * channels;
    
    if (I.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }
    
    int i,j;
    uchar* p;
    for( i = 0; i < nRows; ++i)
    {
        p = I.ptr<uchar>(i);
        for ( j = 0; j < nCols; ++j)
        {
            p[j] = (unsigned char) ((p[j] / dimTics) * dimTics);
            ;
        }
    }
    return I;
}

void showImages(Mat &dispimg, list< pair < Mat, Rect > > imglist, list< pair < string, Point > > tlist) {
    int baseline;
    Size tsize = getTextSize("dummy",CV_FONT_HERSHEY_PLAIN,1.0,1,&baseline);
    for (std::list< pair < Mat, Rect > >::const_iterator it = imglist.begin(); it != imglist.end(); ++it) {
        (*it).first.copyTo(dispimg((*it).second));
    }
    for (std::list< pair < string, Point > >::const_iterator it = tlist.begin(); it != tlist.end(); ++it) {
        putText(dispimg,(*it).first,(*it).second,CV_FONT_HERSHEY_PLAIN,1.0,Scalar(255,255,255));
    }
    imshow(WinTitle,dispimg);
}

int main(int argc, char** argv) {
    // Parse commands globs
    DIR *dp, *op;   // input dir
    FILE *fp;
    int numframes;
    vector<string> dlist = vector<string>();
    string videoname;
    double incr=1.0, decr=1.0;
    int err;
    int delta;
    bool outflag= false;
    
    // Graphics globs
    Mat frame, frame_orig; //current frame
    int frameidx = 0;
    Mat bgmodel, bgmodel_out; //fg/bg masks  generated by MOG2 method
    Mat frameYCrCb, bgmodelYCrCb; //fg/bg masks  generated by MOG2 method
    Mat tmpMask, deltaimg;
    Mat frameY[3], bgmodelY[3];
    list< pair < Mat, Rect > > imglist; // image/ROI list for display
    list< pair < string, Point > > titlelist; // image/ROI list for display
    void (*convert)(Mat, Mat &);
    void (*backconvert)(Mat, Mat &);
    string titleupleft, titleupright="BG Model", titledownleft="Diff |Input-BG|", titledownright="FG Detection";
    
    // Timing globs
    double fps, rfps;
    struct timeval t0, t1, t2;
    
    // Set Command Line Parser
    bool displayFlag = false;
    bool movieinput = false;
    bool verboseFlag = false;
    bool erosionFlag = false; bool blurFlag = false;
    bool reverseFlag = false;
    int w,h;
    string videofile = "prova.avi";
    string outfilename = "bg.png";
    string extArg = "png";
    string coding = "RGB";
    int nbit = 4, ntics = 256, learntime = 1, cachesize = 20;
    string policy = "1:1";
    double watermark = 0.0, uppermark = 50.0, thresh = 0.75;
    int selectthresh = 2;
    vector<string> args(argv + 1, argv + argc);
    for (vector<string>::iterator i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            cout << "Syntax: bewis -i <infile>" << endl;
            cout << string(CHARSKIP1, ' ') << "video file (or frame filename <path>/video_%d.png))" << endl;
            cout << string(CHARSKIP2, ' ') << "-o <outfile>, --outfile <outfile>" << endl;
            cout << string(CHARSKIP1, ' ') << "BG output filename" << endl;
            cout << string(CHARSKIP2, ' ') << "-m <RGB|Lab|HUV>, --mode <RGB|Lab|HUV>" << endl;
            cout << string(CHARSKIP1, ' ') << "color mode (default: RGB)" << endl;
            cout << string(CHARSKIP2, ' ') << "-p <int:int>, --policy <int:int>" << endl;
            cout << string(CHARSKIP1, ' ') << "NN policy (default: 1:1)" << endl;
            cout << string(CHARSKIP2, ' ') << "-b <int>, --bits <int>" << endl;
            cout << string(CHARSKIP1, ' ') << "NN bit resolution (default: 4)" << endl;
            cout << string(CHARSKIP2, ' ') << "-z <int>, --scale <int>" << endl;
            cout << string(CHARSKIP1, ' ') << "color discretization scale (default: 256)" << endl;
            cout << string(CHARSKIP2, ' ') << "-t <double>, --threshold <double>" << endl;
            cout << string(CHARSKIP1, ' ') << "NN threshold (default: 0.75)" << endl;
            cout << string(CHARSKIP2, ' ') << "-u <double>, --uppermark <double>" << endl;
            cout << string(CHARSKIP1, ' ') << "NN rams saturation limit (default: 50)" << endl;
            cout << string(CHARSKIP2, ' ') << "-w <double>, --watermark <double>" << endl;
            cout << string(CHARSKIP1, ' ') << "NN rams firing threshold (default: 0)" << endl;
            cout << string(CHARSKIP2, ' ') << "-k <int>, --cap <int>" << endl;
            cout << string(CHARSKIP1, ' ') << "color repetition time (default: 2)" << endl;
            cout << string(CHARSKIP2, ' ') << "-l <int>, --learntime <int>" << endl;
            cout << string(CHARSKIP1, ' ') << "pre-learning time (in no of frames) (default: 1)" << endl;
            cout << string(CHARSKIP2, ' ') << "-h, --help" << endl;
            cout << string(CHARSKIP1, ' ') << "display this help" << endl;
            cout << string(CHARSKIP2, ' ') << "-v, --verbose" << endl;
            cout << string(CHARSKIP1, ' ') << "display system and video configuration" << endl;
            cout << string(CHARSKIP2, ' ') << "-x, --display" << endl;
            cout << string(CHARSKIP1, ' ') << "enable graphic display (default: disabled)" << endl;
            cout << string(CHARSKIP2, ' ') << "-r, --reverse" << endl;
            cout << string(CHARSKIP1, ' ') << "process video also in reverse mode (default: disabled)" << endl;
            return 0;
        } else if (*i == "-v" || *i == "--verbose") {
            verboseFlag = true;
        } else if (*i == "-r" || *i == "--reverse") {
            reverseFlag = true;
        } else if (*i == "-x" || *i == "--display") {
            displayFlag = true;
        } else if (*i == "-g" || *i == "--blur") {
            blurFlag = true;
        } else {
        string arg = *i;
        if (++i == args.end()) {cout << "Parse error: wrong argument syntax" << endl; exit(-1);}
        if ((arg == "-i" || arg == "--input")) {
            videofile = *i;
            int pos;
            if ((pos = videofile.find_last_of("/\\")) == videofile.size() - 1) {
                videofile = videofile.substr(0,videofile.size()-1);
                pos = videofile.find_last_of("/\\");
            }
            videoname = videofile.substr(pos+1);
        } else if ((arg == "-o" || arg == "--outfile")) {
            outfilename = *i;
            outflag = true;
        } else if (arg == "-m" || arg == "--mode") {
            coding = *i;
        } else if (arg == "-k" || arg == "--cap") {
            selectthresh = atoi((*i).c_str());
        } else if (arg == "-b" || arg == "--bits") {
            nbit = atoi((*i).c_str());
        } else if (arg == "-w" || arg == "--watermark") {
            watermark = (double)atof((*i).c_str());
        } else if (arg == "-u" || arg == "--uppermark") {
            uppermark = (double)atof((*++i).c_str());
        } else if (arg == "-z" || arg == "--scale") {
            ntics = atoi((*i).c_str());
        } else if (arg == "-l" || arg == "--learntime") {
            learntime = atoi((*i).c_str());
        } else if (*i == "-t" || arg == "--threshold") {
            thresh = (double)atof((*i).c_str());
        } else if (arg == "-p" || arg == "--policy") {
            policy = *i;
            if ((err = parsePolicy(policy,incr,decr)) < 0) {
                cerr << "Parse error: Argument: -w" << endl;
                cerr << string(13, ' ') << "policy setting must be <int>:<int>" << endl;
                exit(-1);
            }
        } else {
            cout << "Parse error: wrong argument syntax" << endl;
            exit(-1);
        }
        }
    }
    
    if (coding == "RGB") {
        convert = &rgb2rgb;
        backconvert = &rgb2rgb;
        titleupleft = "Input(RGB): ";
    } else if (coding == "Lab") {
        convert = &rgb2lab;
        backconvert = &lab2rgb;
        titleupleft = "Input(Lab): ";
    } else if (coding == "HSV") {
        convert = &rgb2hsv;
        backconvert = &hsv2rgb;
        titleupleft = "Input(HSV): ";
    } else {
        cerr << "Parse error: Argument: -m" << endl;
        cerr << string(13, ' ') << "Color space must be RGB|HSV|Lab" << endl;
        exit(-1);
    }
    delta = 256 / ntics;
    // Get sequence info (from frame set or movie file)
    VideoCapture cap(videofile);
    if(!cap.isOpened()) { // check if we succeeded
        cout << "I/O error: problem opening video/sequence" << endl;
        exit(-1);
    }
    w = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
    h = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    numframes = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
    fps = (int)cap.get(CV_CAP_PROP_FPS);
    
    // Prefetch movie in vector of frame
    typedef vector<Mat> FramesVec;
    vector<Mat> frames;
    frames.reserve(cap.get(CV_CAP_PROP_FRAME_COUNT));
    
    while (cap.read(frame_orig)) {
        convert(frame_orig, frame);
        frames.push_back(frame.clone());
    }
    cap.release();
    cout << "Processing Video (" << w << "x" << h << ") with " << numframes << " frames"<< endl;
    
    // Initialize Background Subtractor algorithm
    Ptr<BackgroundSubtractorWIS> Subtractor;
    Subtractor = createBackgroundSubtractorWIS();
    Subtractor->set("noBits", nbit);
    Subtractor->set("noTics", ntics);
    Subtractor->set("trainIncr", incr);
    Subtractor->set("trainDecr", decr);
    Subtractor->set("cacheSize", cachesize);
    Subtractor->set("varWatermark", watermark);
    Subtractor->set("varThreshold", thresh);
    Subtractor->set("varUpWatermark", uppermark);
    Subtractor->set("selectThreshold", selectthresh);
    Subtractor->set("learningStage", learntime);
    Subtractor->initialize(frames.at(0).size(), frames.at(0).type());
    if (verboseFlag) {
        cout << left << setw(MAXWIDTH) << setfill(filler) << "I/O GRAPHICS PARAMS" << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Video Coding" << ": " << coding << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Video Name" << ": " << videoname << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Output file" << ": " << outfilename << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Blur" << ": " << (blurFlag ? "enabled" : "diasbled") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Reverse" << ": " << (reverseFlag ? "enabled" : "diasbled") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "WISARD DETECTOR PARAMS" << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "noBits: " << Subtractor->getInt("noBits") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "noTics: " << Subtractor->getInt("noTics") << "(" << Subtractor->getInt("dimTics") << ")" << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "noRams: " << Subtractor->getInt("noRams") <<  endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Train Policy: " << Subtractor->getDouble("trainIncr") << ":" << Subtractor->getDouble("trainDecr") <<  endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Classification Thresh: " << Subtractor->getDouble("varThreshold") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Selection Thresh: " << Subtractor->getInt("selectThreshold") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Watermark: " << Subtractor->getDouble("varWatermark") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Uppermark: " << Subtractor->getDouble("varUpWatermark") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "LearningStage: " << Subtractor->getInt("learningStage") << endl;
        cout << left << setw(MAXWIDTH) << setfill(filler) << "Cachesize: " << Subtractor->getInt("cacheSize") << endl;
    }
    
    // Create output display and its geometry
    int hskip = 16, wskip = 4;
    int dcols = 3;
    Mat outFrame(h+hskip+wskip,w*dcols+(dcols+1)*wskip,CV_8UC3,bgcolor);
    
    // Create window an put icons
    if (displayFlag) cvNamedWindow(WinTitle.c_str(),CV_WINDOW_AUTOSIZE);
    
    // Video processing loop
    FramesVec::const_iterator begin = frames.begin();
    FramesVec::const_iterator it    = begin;
    FramesVec::const_iterator end   = frames.end();
    bool forward = true, stop = false;
    gettimeofday(&t0,NULL);
    do {
        if (it == begin && forward == false) stop = true;
        gettimeofday(&t1,NULL);
        
        frame = (*it); // get image and blur it
        if (blurFlag) blur(frame,frame,Size(3,3));
        
        Subtractor->apply(frame, tmpMask);  // update Background Model (training)
        Subtractor->getBackgroundImage(bgmodel); // get Background Model
        
        // back convert for display
        backconvert(frame,frame);
        backconvert(bgmodel, bgmodel);
        // difference by luminance (Ycrcb)
        cvtColor(frame,frameYCrCb,CV_BGR2YCrCb);
        cvtColor(bgmodel,bgmodelYCrCb,CV_BGR2YCrCb);
        split(frameYCrCb,frameY);
        split(bgmodelYCrCb,bgmodelY);
        absdiff(frameY[0],bgmodelY[0],deltaimg);
        threshold(deltaimg,deltaimg, 20, 255, CV_THRESH_BINARY);
        cvtColor(deltaimg,deltaimg,CV_GRAY2BGR,3);
        
        // build up display
        if (displayFlag) {
            outFrame.setTo(bgcolor);
            imglist.clear();
            titlelist.clear();
            imglist.push_back(make_pair(frame,Rect(Point(wskip,hskip-2),Size(w,h))));  // original frame (Up left)
            titlelist.push_back(make_pair(titleupleft + format("%05d",frameidx),Point(wskip,hskip-5)));
            imglist.push_back(make_pair(bgmodel,Rect(Point(wskip*2+w,hskip-2),Size(w,h))));  // bgmodel (Up right)
            titlelist.push_back(make_pair(titleupright,Point(wskip*2+w,hskip-5)));
            imglist.push_back(make_pair(deltaimg,Rect(Point(wskip*3+2*w,hskip-2),Size(w,h))));  // bgmodel (Up right)
            titlelist.push_back(make_pair(titledownleft,Point(wskip*3+2*w,hskip-5)));
            showImages(outFrame, imglist, titlelist);
            waitKey(1);
        }
        if (forward) { ++frameidx; ++it; } else { --frameidx; --it;}
        /* If iterator is at the end - check reverse mode (otherwise exit loop) */
        if (it == end) {
            if (reverseFlag) {
                forward = false;
                --frameidx;
                --it;
            } else stop = true;
        }
        gettimeofday(&t2,NULL);
        rfps = (int)(1.0 / ((double) (t2.tv_usec - t1.tv_usec)/1000000 + (double) (t2.tv_sec - t1.tv_sec)));
        cout << '\r' << "Processing Frame " << std::setfill('0') << frameidx << "/" << numframes << std::flush;
    } while (!stop);
    // Print timing statistics and BG
    cout << "\nProcessed at " << rfps << " FPS - Total time " << (t2.tv_sec - t0.tv_sec) << " sec"<< endl;
    if (outflag) imwrite(outfilename, bgmodel);
}

