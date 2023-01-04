// nested QR Code generator
// qrcode: module 1 : in black 0x00
//         module 0 : in white 0xFF
//#define DEBUG
#define NOMINMAX
#include "opencv2/opencv.hpp"
#include <opencv2/highgui.hpp>
#include "BitBuffer.hpp"
#include "QrCode.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include <cmath>
#include <string>
#include <codecvt>
#include <locale>
#include <cassert>
#include <fstream>

using namespace cv;
using namespace std;
using namespace qrcodegen;
//using qrcodegen::QrCode;
//using qrcodegen::QrSegment;
int XPOS;
int YPOS;

enum moduleType { MTFinder, MTAlign, MTSeparator, MTTiming, MTFormat, MTVersion,  MTDarkM, MTData};

// Function prototypes
Mat drawBinaryQRCode(const QrCode &qr, int moduleSize, int nqzModules);
int nestQRgenVisual(const QrCode &qrOut, const QrCode &qrIn, Mat &bgImgOut, Mat &bgImgIn, int qZsizeOut, int qZsizeIn, int msizeOut, int msizeIn);


vector<int> getAPPositions(int ver) {
    if (ver == 1)
        return vector<int>();
    else {
        int numAlign = ver / 7 + 2;
        int step;
        if (ver != 32) {
            // ceil((size - 13) / (2*numAlign - 2)) * 2
            step = (ver * 4 + numAlign * 2 + 1) / (2 * numAlign - 2) * 2;
        }
        else  {
            step = 26;
        }

        vector<int> result;
        for (int i = 0, pos = ver * 4 + 10; i < numAlign - 1; i++, pos -= step)
            result.insert(result.begin(), pos);
        result.insert(result.begin(), 6);
        return result;
    }
}


bool isFinderP(int y, int x, int ver)
{
    int modulesPerRow = 17 + ver * 4;

    if ((y < 7) && (x < 7)) return true;      //left top pattern
    if ((y >= modulesPerRow - 7) && (x < 7)) return true; // left bottom
    if ((y < 7) && (x >= modulesPerRow - 7)) return true; //right top

    return false;
}
bool isAlignP(int y, int x, int ver)
{
    int modulesPerRow = 17 + ver * 4;

    if ((x <= 8 && y <= 8) || (x <= 8 && y >= modulesPerRow - 9) || (x >= modulesPerRow - 9 && y <= 8)) {
        return false;
    }

    vector<int> ap = getAPPositions(ver);
    for (int i = 0; i < ap.size(); ++i) {
        if (abs(y - ap[i]) <= 2) {
            for (int j = 0; j < ap.size(); ++j) {
                if (abs(x - ap[j]) <= 2) {
                    return true;
                }
            }
        }
    }
    return false;
}
// check whether a module is a Timing Pattern
bool isTimingP(int y, int x, int ver)
{
    int modulesPerRow = 17 + ver * 4;

    if ((y == 6 && ((x >= 8) && (x <= modulesPerRow - 9)))) return true;
    if ((x == 6 && ((y >= 8) && (y <= modulesPerRow - 9)))) return true;

    return false;
}
bool isFormat(int y, int x, int ver)
{
    int modulesPerRow = 17 + ver * 4;

    if ((y == 8 && (x < 9 || x > modulesPerRow - 9))) return true;
    if ((x == 8 && (y < 9 || y > modulesPerRow - 8))) return true;

    return false;
}
bool isVersion(int y, int x, int ver)
{
    int modulesPerRow = 17 + ver * 4;

    if (ver < 7) return false; // only for version >=7
    if (((y == modulesPerRow - 9) || (y == modulesPerRow - 10) || (y == modulesPerRow - 11)) && (x < 6))
        return true;
    if (((x == modulesPerRow - 9) || (x == modulesPerRow - 10) || (x == modulesPerRow - 11)) && (y < 6))
        return true;

    return false;
}
bool isDarkModule(int y, int x, int ver)
{
    int modulesPerRow = 17 + ver * 4;

    if ((y == modulesPerRow - 8) && (x == 8)) return true;

    return false;
}
bool isSeparator(int y, int x, int ver)
{
    int modulesPerRow = 17 + ver * 4;

    //horizontal
    if ((y == 7) && ((x < 8) || (x > modulesPerRow - 9))) return true;
    if ((y == modulesPerRow - 8) && (x < 8)) return true;

    //vertical
    if ((x == 7) && ((y < 8) || (y > modulesPerRow - 9))) return true;
    if ((x == modulesPerRow - 8) && (y < 8)) return true;

    return false;
}

//************************************************************************************************************
int main()
{
    /* ΩsΩX§§§Â¶r¶bQR Code•Œ
    // QR code parameters
    //unicode to utf8
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;

    wchar_t *sOut = L"https://www.yzu.edu.tw";
    //wchar_t *sOut = L"Nested QR code";
    vector<uint8_t> textOut;
    for (int i = 0; i < wcslen(sOut); ++i) {
        std::string utf8 = convert.to_bytes(sOut[i]);
        for (int j = 0; j < utf8.length(); ++j) {
            textOut.push_back(utf8[j]);
        }
    }

    wchar_t *sIn = L"https://www.youtube.com/watch?v=9i_UQC4znvU";
    //wchar_t *sIn = L"A nested QR code exhibits two QR codes in a square";
    vector<uint8_t> textIn;
    for (int i = 0; i < wcslen(sIn); ++i) {
        std::string utf8 = convert.to_bytes(sIn[i]);
        for (int j = 0; j < utf8.length(); ++j) {
            textIn.push_back(utf8[j]);
        }
    }
*/
    ////實驗一//out:元智大學官網 //in:元智大學資傳系官網 //qsizeOut 3,qsizeIn 1 //ecc都是M等級 //模組大小18*18    6*6
//        char textOut[] = "https://www.yzu.edu.tw/index.php/tw/";
//        char textIn[] = "http://www.infocom.yzu.edu.tw/index.php/zh-tw/";
//        Mat bgImgOut = imread("/Users/rmx/Desktop/桌面 - RmX/image/271233731_2062563473924529_791573236800201000_n.jpeg");
//        Mat bgImgIn = imread("/Users/rmx/Desktop/桌面 - RmX/image/yzuci_logo.jpg");
//        int qZsizeOut = 3;
//        int qZsizeIn = 1;
//        int eccOut = 2; // LOW = 0, MEDIUM = 1, QUARTILE = 2, HIGH = 3  7/15/25/30
//        int eccIn = 2;
//        int moduleSizeTimes =6;  //QR code大小的幾倍pixel值
//        int msizeOut = 3 * moduleSizeTimes;  //  module size of Outer QR Code
//        int msizeIn = 1 * moduleSizeTimes;    //  module size of inner QR
        

        
    //
    ////實驗二//out:桃園觀光局 //in:桃園市政府官網//qsizeOut 3,qsizeIn 1 //ecc都是Q等級 //模組大小90x90  30x30
        //因為textIn與textOut字數不能差太多，所以把textIn變成短網址才能正常被識讀 或是增加ＥＣＣ到Ｈ等級
//        char textOut[] = "https://tour.tycg.gov.tw/zh-tw";
//        //char textIn[] = "https://reurl.cc/OpYgj9";
//        char textIn[] = "https://www.tycg.gov.tw/ch/index.jsp?popflag=Y";
//        Mat bgImgOut = imread("/Users/rmx/Desktop/桌面 - RmX/image/tauyuan_water.jpeg");
//        Mat bgImgIn = imread("/Users/rmx/Desktop/桌面 - RmX/image/tauyuan_city.png");
//        int qZsizeOut = 3;
//        int qZsizeIn = 1;
//        int eccOut = 3; // LOW = 0, MEDIUM = 1, QUARTILE = 2, HIGH = 3  7/15/25/30
//        int eccIn = 3;
//        int moduleSizetimes =6;
//        int msizeOut = 3 * moduleSizetimes;  //  module size of Outer QR Code
//        int msizeIn = 1 * moduleSizetimes;    //  module size of inner QR
    //
        //實驗二//out:DJ Mag官方網站 //in:Martin Garrix字串//qsizeOut 3,qsizeIn 1 //ecc都是Q等級 //模組大小90x90  30x30
        //因為textIn與textOut字數不能差太多，所以把textIn變成短網址才能正常被識讀 或是增加ＥＣＣ到Ｈ等級
    //    char textIn[] = "https://djmag.com";
    //    char textOut[] = "https://www.youtube.com/watch?v=86sFN_2kw5k";
    //    Mat bgImgIn = imread("/Users/rmx/Desktop/桌面 - RmX/image/MG Logo.jpeg");
    //    Mat bgImgOut = imread("/Users/rmx/Desktop/桌面 - RmX/image/MG game over.jpeg");
    //    int qZsizeOut = 3;
    //    int qZsizeIn = 1;
    //    int eccOut = 2; // LOW = 0, MEDIUM = 1, QUARTILE = 2, HIGH = 3  7/15/25/30'
    //    int eccIn = 2;
    //    int moduleSizetimes =6;
    //    int msizeOut = 3 * moduleSizetimes;  //  module size of Outer QR Code
    //    int msizeIn = 1 * moduleSizetimes;    //  module size of inner QR

        //實驗二//out:F1官方網站 //in:Red Bull Racing字串//qsizeOut 3,qsizeIn 1 //ecc都是Q等級 //模組大小90x90  30x30
        //因為textIn與textOut字數不能差太多，所以把textIn變成短網址才能正常被識讀 或是增加ＥＣＣ到Ｈ等級
        char textIn[] = "https://www.formula1.com/en/";
        char textOut[] = "Scuderia AlphaTauri F1 Team";
        Mat bgImgIn = imread("/Users/rmx/Desktop/桌面 - RmX/image/f1.jpg");
        //Mat bgImgIn = imread("/Users/rmx/Desktop/桌面 - RmX/image/RBR.jpg");
        Mat bgImgOut = imread("/Users/rmx/Desktop/桌面 - RmX/image/AT.jpg");
        int qZsizeOut = 3;
        int qZsizeIn = 1;
        int eccOut = 3; // LOW = 0, MEDIUM = 1, QUARTILE = 2, HIGH = 3  7/15/25/30
        int eccIn = 3;
        int moduleSizetimes =6;
        int msizeOut = 3 * moduleSizetimes;  //  module size of Outer QR Code
        int msizeIn = 1 * moduleSizetimes;    //  module size of inner QR
    //
    //    // encoding binary QR Codes LOW = 0, MEDIUM = 1, QUARTILE = 2, HIGH = 3
    //    //const QrCode qrOut = QrCode::encodeBinary(textOut, (QrCode::Ecc) eccOut);
    //    //const QrCode qrIn = QrCode::encodeBinary(textIn, (QrCode::Ecc) eccIn);


    
    // encoding binary QR Codes LOW = 0, MEDIUM = 1, QUARTILE = 2, HIGH = 3
    //const QrCode qrOut = QrCode::encodeBinary(textOut, (QrCode::Ecc) eccOut);
    //const QrCode qrIn = QrCode::encodeBinary(textIn, (QrCode::Ecc) eccIn);
    
    const QrCode qrOut = QrCode::encodeText(textOut, (QrCode::Ecc) eccOut);
    const QrCode qrIn = QrCode::encodeText(textIn, (QrCode::Ecc) eccIn);
    

    //generate nested QR code using simple method
    nestQRgenVisual(qrOut, qrIn, bgImgOut, bgImgIn,  qZsizeOut, qZsizeIn, msizeOut, msizeIn);


    //*** only for count error
/*
    ifstream inFile("qrcode.txt");
    
    int chg1 = 0;
    int chg2 = 0;
    int uchg1 = 0;
    int uchg2 = 0;
    cout << endl << endl;
    for (int y = 0; y < qrOut.getSize(); ++y) {
        for (int x = 0; x < qrOut.getSize(); ++x) {
            char ch;
            while ( inFile.get(ch) ) {
                if ( (ch == '0') || (ch == '1') )
                    break;
            }
            char ch2 = qrIn.getModule(x, y) ? '1' : '0';

            if (ch == ch2) {
                cout << '0';
                if ((y >= (YPOS / 3)) && (y <= ((YPOS + qrIn.getSize() + 2) / 3)) && (x >= (XPOS / 3)) && (x <= ((XPOS + qrIn.getSize() + 2) / 3))) {
                    ++uchg1;
                }
                else {
                    ++uchg2;
                }
            }
            else {
                cout << '1';
                if ((y >= (YPOS / 3)) && (y <= ((YPOS + qrIn.getSize() + 2) / 3)) && (x >= (XPOS / 3)) && (x <= ((XPOS + qrIn.getSize() + 2) / 3))) {
                    ++chg1;
                }
                else {
                    ++chg2;
                }
            }
    
            

            //cout << qrIn.getModule(x, y) ? "0" : "1";
        }
        cout << endl;
    }
    cout << "In chg=" << chg1 << endl;
    cout << "Out chg=" << chg2 << endl;
    cout << "In total=" << chg1 + uchg1 << endl;
    cout << "Out total=" << chg2 + uchg2 << endl;
    cout << "In %=" << (float)chg1/ (chg1 + uchg1) << endl;
    cout << "Out %=" << (float)chg2/(chg2 + uchg2) << endl;
    cout << endl << endl;
*/
    //************************

#ifdef DEBUG
    cout << endl << endl;
    for (int y = 0; y < qrOut.getSize(); ++y) {
        for (int x = 0; x < qrOut.getSize(); ++x) {
            cout << qrOut.getModule(x, y) ? "0" : "1";
        }
        cout << endl;
    }

    cout << endl << endl;
#endif

    waitKey(0);
    return EXIT_SUCCESS;
}


// drawing binary QR Code with Module Size moduleSize
Mat drawBinaryQRCode(const QrCode &qr, int moduleSize, int nqzModules)
{
    int baseQRImgPixels = moduleSize * (qr.getSize() + 2 * nqzModules);   // image size
    Mat qrImg(baseQRImgPixels, baseQRImgPixels, CV_8UC3, Scalar(0xFF, 0xFF, 0xFF));
    for (int y = 0; y < baseQRImgPixels; y++) {
        int my = y / moduleSize;
        for (int x = 0; x < baseQRImgPixels; x++) {
            int mx = x / moduleSize;
            if ((mx < nqzModules) || (mx > qr.getSize() + nqzModules) || (my < nqzModules) || (my > qr.getSize() + nqzModules)) {  // quiet zone pixel
                qrImg.at<Vec3b>(y, x)[0] = 0xFF;
                qrImg.at<Vec3b>(y, x)[1] = 0xFF;
                qrImg.at<Vec3b>(y, x)[2] = 0xFF;
            }
            else {
                uchar color = (qr.getModule(mx - nqzModules, my - nqzModules) ? 0x00 : 0xFF);
                qrImg.at<Vec3b>(y, x)[0] = color;
                qrImg.at<Vec3b>(y, x)[1] = color;
                qrImg.at<Vec3b>(y, x)[2] = color;
            }
        }
    }
    return qrImg;
}
// simple nested QR code

int nestQRgenVisual(const QrCode &qrOut, const QrCode &qrIn, Mat &bgImgOut, Mat &bgImgIn, int qZsizeOut, int qZsizeIn, int msizeOut, int msizeIn)
{
    Mat outImg, inImg, oInImg;  // Outer, Inner, Combined QR Image
    Mat outBitmap, inBitmap;  // Outer, Inner, Combined QR Module Bits  1: dark 0: light
    Mat outCost, inCost;
    int vIn = qrIn.getVersion(); // qr code version 1- 40
    int vOut = qrOut.getVersion();  // cost matrix


    // get error correction Level of in and out QR code
    QrCode::Ecc eccIn = qrIn.getErrorCorrectionLevel();  // LOW = 0, MEDIUM = 1, QUARTILE = 2, HIGH = 3
    QrCode::Ecc eccOut = qrOut.getErrorCorrectionLevel(); //

    // draw binary QR codes
    outImg = drawBinaryQRCode(qrOut, msizeOut, 0); // print binary QR code
    inImg = drawBinaryQRCode(qrIn, msizeIn, qZsizeIn); // print binary QR code

//#ifdef DEBUG
    imshow("Outer QR Code ", outImg);
    imshow("Inner QR Code", inImg);
//#endif
    inImg.copyTo(oInImg);
    int qrsizeOut = qrOut.getSize();  // module number  per row/column
    int numModuleOut = qrOut.getSize()*qrOut.getSize();  // total modules
    int numfmOut = 0;  // function module number
    int numdmOut = 0;  // data module number

    // calculate number of function modules
    for (int y = 0; y < qrsizeOut; ++y) {
        for (int x = 0; x < qrsizeOut; ++x) {
            if (qrOut.isFunction.at(y).at(x)) {
                ++numfmOut;
            }
        }

    }
    numdmOut = qrsizeOut*qrsizeOut - numfmOut;
#ifdef DEBUG
    cout << "Outer QR V=" << vOut << " ECC=" << (int)eccOut << " Size:" << qrsizeOut << "x" << qrsizeOut << "=" << numModuleOut << " FM/DM=" << numfmOut << "/" << numdmOut << endl;
#endif
    // resampled QR bits to inQR resolution
    outBitmap.create(3 * qrsizeOut, 3 * qrsizeOut, CV_8UC1);
    outBitmap.setTo(0);
    outCost.create(3 * qrsizeOut, 3 * qrsizeOut, CV_32SC1);
    outCost.setTo(0);

    // set cost for function module and data module
    const int outLV = 1000;  // a large Value
    for (int y = 0; y < outBitmap.rows; ++y) {
        for (int x = 0; x < outBitmap.cols; ++x) {

            int yy = y / 3;
            int xx = x / 3;

            outBitmap.at<uchar>(y, x) = qrOut.getModule(xx, yy);

            if (isFinderP(yy, xx, vOut))  {
                outCost.at<int>(y, x) = outLV;
            }
            else if (isAlignP(yy, xx, vOut))  {
                outCost.at<int>(y, x) = 0;
            }
            else if (isSeparator(yy, xx, vOut))  {
                outCost.at<int>(y, x) = outLV;
            }
            else if (isTimingP(yy, xx, vOut))  {
                outCost.at<int>(y, x) = outLV;
            }
            else if (isFormat(yy, xx, vOut))  {
                outCost.at<int>(y, x) = outLV;
            }
            else if (isVersion(yy, xx, vOut))  {
                outCost.at<int>(y, x) = outLV;
            }
            else if (isDarkModule(yy, xx, vOut))  {
                outCost.at<int>(y, x) = outLV;
            }
            else {  // data module
                outCost.at<int>(y, x) = 0;
            }
        }
    }

    // in QR code parameter
    int qrsizeIn = qrIn.getSize();
    int numModuleIn = qrIn.getSize()*qrIn.getSize();
    int numfmIn = 0;
    int numdmIn = 0;
    for (int y = 0; y < qrsizeIn; ++y) {
        for (int x = 0; x < qrsizeIn; ++x) {
            if (qrIn.isFunction.at(y).at(x)) {
                ++numfmIn;
            }
        }

    }
#ifdef DEBUG
    numdmIn = qrsizeIn*qrsizeIn - numfmIn;
    cout << "Inner QR V=" << vIn << " ECC=" << (int)eccIn << " Size:" << qrsizeIn << "x" << qrsizeIn << "=" << numModuleIn << " FM/DM=" << numfmIn << "/" << numdmIn << endl;
#endif

    // inQR with 1 quiet zone
    inBitmap.create(qrsizeIn + 2 * qZsizeIn, qrsizeIn + 2 * qZsizeIn, CV_8UC1); // padding (module 0) white color 0xFF for outter boundary width = 1
    inBitmap.setTo(0);
    inCost.create(qrsizeIn + 2 * qZsizeIn, qrsizeIn + 2 * qZsizeIn, CV_32SC1);
    inCost.setTo(0);

    //set cost for function pattern and data module
    const int inLargeValue = 1000; //a Big value
    for (int y = 0; y < inBitmap.rows; ++y) {
        for (int x = 0; x < inBitmap.cols; ++x) {

            //  salient region
            if ((y < qZsizeIn) || (y >= inBitmap.rows - qZsizeIn) || (x < qZsizeIn) || (x >= inBitmap.cols - qZsizeIn)) {
                inBitmap.at<uchar>(y, x) = 0;
                inCost.at<int>(y, x) = inLargeValue;
            }
            else {
                int yy = y - qZsizeIn;
                int xx = x - qZsizeIn;
                
                inBitmap.at<uchar>(y, x) = qrIn.getModule(xx, yy);

                if (isFinderP(yy, xx, vIn))  {
                    inCost.at<int>(y, x) = inLargeValue;
                }
                else if (isAlignP(yy, xx, vIn))  {
                    inCost.at<int>(y, x) = inLargeValue;
                }
                else if (isSeparator(yy, xx, vIn))  {
                    inCost.at<int>(y, x) = inLargeValue;
                }
                else if (isTimingP(yy, xx, vIn))  {
                    inCost.at<int>(y, x) = inLargeValue;
                }
                else if (isFormat(yy, xx, vIn))  {
                    inCost.at<int>(y, x) = inLargeValue;
                }
                else if (isVersion(yy, xx, vIn))  {
                    inCost.at<int>(y, x) = inLargeValue;
                }
                else if (isDarkModule(yy, xx, vIn))  {
                    inCost.at<int>(y, x) = inLargeValue;
                }
                else {  // data module
                    inCost.at<int>(y, x) = 0;
                }
            }
            
        }
    }
    // find best position to stamp
    int direction = 0;
    int iniPosition = 0; // left top of inner QR: using inner module size
    int iniY = iniPosition; // ini position using inner module size
    int iniX = iniPosition;
    int minCostost = numeric_limits<int>::max();
    int minCosthg = 0;
    Mat inBitmap2, inCost2;
    inBitmap.copyTo(inBitmap2);
    inCost.copyTo(inCost2);

    // k=0,1,2,3 for rotate 0 90 180 270
    for (int k = 0; k < 4; ++k) {
        // Evaluation original QR: search for minimal change
        for (int yInd = iniPosition; yInd < outBitmap.rows - inBitmap2.rows; ++yInd) {
            for (int xInd = iniPosition; xInd < outBitmap.cols - inBitmap2.cols; ++xInd) {
                int costT = 0;
                int evaChg = 0;
                for (int y = 0; y < inBitmap2.rows; ++y) {
                    for (int x = 0; x < inBitmap2.cols; ++x) {
                        // high cost in function patterns
                        if (0 != outCost.at<int>(yInd + y, xInd + x) ){
                            costT += 1000;
                        }

                        if (outBitmap.at<uchar>(yInd + y, xInd + x) != inBitmap2.at<uchar>(y, x)) {
                            if (((yInd + y) % 3 == 1) && ((xInd + x) % 3 == 1)) {  // centroid module color checking

                                if (0 != inCost2.at<int>(y, x)) {
                                    costT += 1;
                                    evaChg += 1;
                                }
                            }
                        }
                    }
                }

                if (costT < minCostost) {
                    minCostost = costT;
                    minCosthg = evaChg;
                    iniY = yInd;
                    iniX = xInd;

                    direction = k;
                }

                //cout << costT << " ";
            }
            //cout << endl;
        }
        flip(inBitmap2, inBitmap2, 0);
        transpose(inBitmap2, inBitmap2);

        flip(inCost2, inCost2, 0);
        transpose(inCost2, inCost2);
    }
    YPOS = iniY;
    XPOS = iniX;
    cout<<"最佳位置的ＸandY位置 ： ("<<XPOS<<","<<YPOS<<")"<<endl;
    cout<<"最佳位置的ＸandY位置的變數名稱 ：(iniX,iniY)"<<endl;
    // set to the best version and position
    inBitmap.copyTo(inBitmap2);
    inCost.copyTo(inCost2);
    for (int i = 0; i < direction; ++i) {
        flip(inImg, inImg, 0);
        transpose(inImg, inImg);

        flip(inBitmap2, inBitmap2, 0);
        transpose(inBitmap2, inBitmap2);

        flip(inCost2, inCost2, 0);
        transpose(inCost2, inCost2);
    }

    resize(bgImgOut, bgImgOut, Size(outImg.rows, outImg.cols), 0, 0, INTER_LINEAR);
    resize(bgImgIn, bgImgIn, Size(inImg.rows, inImg.cols), 0, 0, INTER_LINEAR);
    imshow("this is inImg",inImg);


    cout << "Best position (inner Module scale) Dir=" << direction << endl <<  " Y="<< iniY << " X=" << iniX << " Chg=" << minCostost << endl;
    cout << "Best position (inner Module scale) Dir=" << direction << endl <<  " Y="<< iniY << " X=" << iniX << " Chg=" << minCostost << endl;
#ifdef DEBUG
    imshow("background Out", bgImgOut);
    imshow("background In", bgImgIn);
#endif

    // draw the nested visual QR code
    Mat nestImg;
    bgImgOut.copyTo(nestImg);

    float fnQRratioOut = 0.5;
    float dataQRratioOut = 0;
    for (int y = 0; y < qrOut.getSize(); ++y) {
        for (int x = 0; x < qrOut.getSize(); ++x) {
            //function patterns
            if ( qrOut.isFunction.at(y).at(x) ) {
                for (int m = 0;  m < msizeOut; ++m) {
                    for (int n = 0; n < msizeOut; ++n) {
                        if ( 0 == qrOut.getModule(x, y) ) { //white module
                            
                            int v1 =(int) ( fnQRratioOut * 255 + (1 - fnQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0]);
                            if (v1 > 255) v1 = 255;
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0] =  (unsigned char) (v1);
                            
                            int v2 = (int)(fnQRratioOut * 255 + (1 - fnQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1]);
                            if (v2 > 255) v2 = 255;
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1] = (unsigned char)(v2);

                            int v3 = (int)(fnQRratioOut * 255 + (1 - fnQRratioOut)*bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2]);
                            if (v3 > 255) v3 = 255;
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2] = (unsigned char)(v3);
                        }
                        else {  // black module

                            int v1 = (int)( (1 - fnQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0] );
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0] = (unsigned char)(v1);

                            int v2 = (int)((1 - fnQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1]);
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1] = (unsigned char)(v2);

                            int v3 = (int)((1 - fnQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2]);
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2] = (unsigned char)(v3);
                        }
                        
                    }
                }
            }
            else {
                // centroid region of a module of ourter QR code
                for (int m = msizeOut/3; m < 2*msizeOut/3; ++m) {
                    for (int n = msizeOut/3; n < 2*msizeOut/3; ++n) {
                        if (0 == qrOut.getModule(x, y)) { //white module
                            int v1 = (int)(dataQRratioOut * 255 + (1 - dataQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0]);
                            if (v1 > 255) v1 = 255;
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0] = (unsigned char)(v1);

                            int v2 = (int)(dataQRratioOut * 255 + (1 - dataQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1]);
                            if (v2 > 255) v2 = 255;
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1] = (unsigned char)(v2);

                            int v3 = (int)(dataQRratioOut * 255 + (1 - dataQRratioOut)*bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2]);
                            if (v3 > 255) v3 = 255;
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2] = (unsigned char)(v3);
                        }
                        else {  // black module
                            int v1 = (int)((1 - dataQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0]);
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[0] = (unsigned char)(v1);

                            int v2 = (int)((1 - dataQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1]);
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[1] = (unsigned char)(v2);

                            int v3 = (int)((1 - dataQRratioOut) * bgImgOut.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2]);
                            nestImg.at<Vec3b>(y * msizeOut + m, x * msizeOut + n)[2] = (unsigned char)(v3);
                        }
                    }
                }
            }
            
        }
    }
    //imshow("nested qr code in the original image",nestImg);
    imshow("The Nested Image",nestImg);
    //imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/ori_nestImg.jpg",nestImg);
    // fill the inner QR code
    Mat inQRisFunFlag(qrIn.getSize() * msizeIn, qrIn.getSize() * msizeIn, CV_8UC1);
    for (int y = 0; y < inQRisFunFlag.rows; ++y) {
        for (int x = 0; x < inQRisFunFlag.cols; ++x) {
            if (  qrIn.isFunction.at(y/msizeIn).at(x/msizeIn) ) {
                inQRisFunFlag.at<uchar>(y, x) = 0xFF;
            }
            else {
                inQRisFunFlag.at<uchar>(y, x) = 0x00;
            }
        }
    }
    imshow("inQRisFunFlag",inQRisFunFlag);
    copyMakeBorder(inQRisFunFlag, inQRisFunFlag, msizeIn, msizeIn, msizeIn, msizeIn, BORDER_CONSTANT, Scalar(0xFF));
    for (int d = 0; d < direction; ++d) {
        flip(inQRisFunFlag, inQRisFunFlag, 0);
        transpose(inQRisFunFlag, inQRisFunFlag);
    }

    float fnQRratioIn = 0.5;
    float dataQRratioIn = 0.5;
    for (int y = 0; y < inBitmap2.rows; ++y) {
        for (int x = 0; x < inBitmap2.cols; ++x) {
            for (int m = 0; m < msizeIn; ++m) {
                for (int n = 0; n < msizeIn; ++n) {
                    if ( 0xFF == inQRisFunFlag.at<uchar>( y * msizeIn + m, x * msizeIn + n)) {  //function pattern //function pattern在QRin的透明度
                        if (0xFF == inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0]) { // white
                            int v1 = (int)(fnQRratioIn * 255 + (1 - fnQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);
                            if (v1 > 255) v1 = 255;
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = (unsigned char) v1;

                            int v2 = (int)(fnQRratioIn * 255 + (1 - fnQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);
                            if (v2 > 255) v2 = 255;
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = (unsigned char)v2;

                            int v3 = (int)(fnQRratioIn * 255 + (1 - fnQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);
                            if (v3 > 255) v3 = 255;
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = (unsigned char)v3;
                        }
                        else {  // black
                            int v1 = (int)((1 - fnQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = (unsigned char)v1;

                            int v2 = (int)((1 - fnQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = (unsigned char)v2;

                            int v3 = (int)((1 - fnQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = (unsigned char)v3;
                        }

                    }
                    else {
                        // centroid block of a module of Outer QR code
                        if ((1 == y % 3) && (1 == x % 3))  {
                            if ((m >= msizeIn / 3) && (m < 2 * msizeIn / 3) && (n >= msizeIn / 3) && (n < 2 * msizeIn / 3)) {

                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = inImg.at<Vec3b>(  y * msizeIn + m,  x * msizeIn + n)[0];
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = inImg.at<Vec3b>(  y * msizeIn + m,  x * msizeIn + n)[1];
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = inImg.at<Vec3b>(  y * msizeIn + m,  x * msizeIn + n)[2];
                            }
                            else {
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = outImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0];
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = outImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1];
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = outImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2];
                            }

                        }
                        else {
                            if (0xFF == inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0]) { // white
                                int v1 = (int)(dataQRratioIn * 255 + (1 - dataQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);
                                if (v1 > 255) v1 = 255;


                                int v2 = (int)(dataQRratioIn * 255 + (1 - dataQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[1]);
                                if (v2 > 255) v2 = 255;


                                int v3 = (int)(dataQRratioIn * 255 + (1 - dataQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[2]);
                                if (v3 > 255) v3 = 255;

                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = (unsigned char)v1;
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = (unsigned char)v2;
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = (unsigned char)v3;
                                
                            }
                            else {  // black
                                int v1 = (int)((1 - dataQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[0]);


                                int v2 = (int)((1 - dataQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[1]);


                                int v3 = (int)((1 - dataQRratioIn) * bgImgIn.at<Vec3b>(y * msizeIn + m, x * msizeIn + n)[2]);

                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = (unsigned char)v1;
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = (unsigned char)v2;
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = (unsigned char)v3;
                                
                            }
                        }

                    }

                }
            }
        }
    }
    
/*
    // fill the inner QR code
    for (int y = 0; y < inBitmap2.rows; ++y) {
        for (int x = 0; x < inBitmap2.cols; ++x) {
            // changed blocks
            if (outBitmap.at<uchar>(iniY + y, iniX + x) != inBitmap2.at<uchar>(y, x)) {
                //in centroid block -- fill inner mofules
                if ( (1 == (iniY + y) % 3) && (1 == (iniX + x) % 3))  {
                    // function modules in inner QR
                    if (inCost2.at<int>(y, x) != 0) {
                        for (int m = 0; m < msizeIn; ++m) {
                            for (int n = 0; n < msizeIn; ++n) {
                                //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0];
                                //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[1];
                                //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[2];

                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0];
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[1];
                                nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[2];
                            }
                        }
                    }
                    else {
                        // data modules
                        for (int m = 0; m < msizeIn; ++m) {
                            for (int n = 0; n < msizeIn; ++n) {
                                if ((m >= msizeIn / 3) && (m < 2 * msizeIn / 3) && (n >= msizeIn / 3) && (n < 2 * msizeIn / 3)) { //centroid -- change
                                    //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0];
                                    //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[1];
                                    //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[2];

                                    nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0];
                                    nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[1];
                                    nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[2];
                                }
                            }
                        }

                    }
                }
                else {  // not in centroid blocks
                    for (int m = 0; m < msizeIn; ++m) {
                        for (int n = 0; n < msizeIn; ++n) {
                            //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0];
                            //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[1];
                            //nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = inImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[2];

                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0];
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[1];
                            nestImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = bgImgIn.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[2];
                        }
                    }
                }

            }
        }
    }
*/
    
#ifdef DEBUG
    //draw compare image by setting all inQR black
    Mat compareImg;
    outImg.copyTo(compareImg);  // initialize result
    for (int y = 0; y < inBitmap2.rows; ++y) {
        for (int x = 0; x < inBitmap2.cols; ++x) {
            
            for (int m = 0; m < msizeIn; ++m) {
                for (int n = 0; n < msizeIn; ++n) {
                    compareImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = 0;
                    compareImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = 0;
                    compareImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = 0;
                }
            }
            
        }
    }
    copyMakeBorder(compareImg, compareImg, msizeOut * 4, msizeOut * 4, msizeOut * 4, msizeOut * 4, BORDER_CONSTANT, Scalar(255, 255, 255));

    imshow("compare Image", compareImg);

    // direct oveylay
    Mat overlayImg;
    outImg.copyTo(overlayImg);  // initialize result
    for (int y = 0; y < inBitmap2.rows; ++y) {
        for (int x = 0; x < inBitmap2.cols; ++x) {
            for (int m = 0; m < msizeIn; ++m) {
                for (int n = 0; n < msizeIn; ++n) {
                    overlayImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[0] = oInImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[0];
                    overlayImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[1] = oInImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[1];
                    overlayImg.at<Vec3b>((iniY + y) * msizeIn + m, (iniX + x) * msizeIn + n)[2] = oInImg.at<Vec3b>(y*msizeIn + m, x*msizeIn + n)[2];
                }
            }
        }
    }
    copyMakeBorder(overlayImg, overlayImg, msizeOut * 4, msizeOut * 4, msizeOut * 4, msizeOut * 4, BORDER_CONSTANT, Scalar(255, 255, 255));

    imshow("overlay Image", overlayImg);

    Mat outPartialImg = outImg(Rect(iniX * msizeIn, iniY * msizeIn, inImg.cols, inImg.rows));

    imshow("out partial QR", outPartialImg);

    Mat nestPartialImg = nestImg(Rect(iniX * msizeIn, iniY * msizeIn, inImg.cols, inImg.rows));

    imshow("nested partial QR", nestPartialImg);

    // draw change introduce to outQR
    Mat diffImg1;
    outPartialImg.copyTo(diffImg1);
    diffImg1.setTo(0);

    // draw change introduce to inQR
    Mat diffImg2;
    inImg.copyTo(diffImg2);
    diffImg2.setTo(0);

    //outQR change
    for (int y = 0; y < diffImg1.rows; ++y) {
        for (int x = 0; x < diffImg1.cols; ++x) {
            // out change
            if (nestPartialImg.at<Vec3b>(y, x)[0] != outPartialImg.at<Vec3b>(y, x)[0]) {
                //centroid 1/3 region
                if ((1 == (y / msizeIn) % 3) && (1 == (x / msizeIn) % 3)) {
                    diffImg1.at<Vec3b>(y, x)[0] = 0;
                    diffImg1.at<Vec3b>(y, x)[1] = 0;
                    diffImg1.at<Vec3b>(y, x)[2] = 255;

                
                }
                else {
                    diffImg1.at<Vec3b>(y, x)[0] = 0;
                    diffImg1.at<Vec3b>(y, x)[1] = 255;
                    diffImg1.at<Vec3b>(y, x)[2] = 0;

                
                }
            }
        }
    }

    //inQR change
    for (int y = 0; y < diffImg2.rows; ++y) {
        for (int x = 0; x < diffImg2.cols; ++x) {
            if (nestPartialImg.at<Vec3b>(y, x)[0] != inImg.at<Vec3b>(y, x)[0]) {
                diffImg2.at<Vec3b>(y, x)[0] = 0;
                diffImg2.at<Vec3b>(y, x)[1] = 255;
                diffImg2.at<Vec3b>(y, x)[2] = 0;
            }
        }
    }

    imshow("Outer change", diffImg1);
    imshow("Inner change", diffImg2);

    Mat nestPartialChgImg;
    outPartialImg.copyTo(nestPartialChgImg);
    for (int y = 0; y < nestPartialChgImg.rows / msizeOut; ++y) {
        for (int x = 0; x < nestPartialChgImg.cols / msizeOut; ++x) {
            int blkchg = 0;
            for (int m = 0; m < msizeOut; ++m) {
                for (int n = 0; n < msizeOut; ++n) {
                    if (255 == diffImg1.at<Vec3b>(y*msizeOut + m, x*msizeOut + n)[2]) {
                        blkchg ++;
                    }
                }
            }
            if (blkchg > msizeOut*msizeOut/16) {
                unsigned char setColor;
                if ( 0 == nestPartialChgImg.at<Vec3b>(y*msizeOut + 0, x*msizeOut + 0)[0]) {
                    setColor = 255;
                }
                else {
                    setColor = 128;
                }
                for (int m = 0; m < msizeOut; ++m) {
                    for (int n = 0; n < msizeOut; ++n) {
                        nestPartialChgImg.at<Vec3b>(y*msizeOut + m, x*msizeOut + n)[0] = 0;
                        nestPartialChgImg.at<Vec3b>(y*msizeOut + m, x*msizeOut + n)[1] = 0;
                        nestPartialChgImg.at<Vec3b>(y*msizeOut + m, x*msizeOut + n)[2] = setColor;
                    }
                }
            }
        }
    }

    imshow("nested partial Change QR", nestPartialChgImg);

    cout << "Outer change=" << minCosthg << "/" << numdmOut << "(" << setprecision(.2) << (float)minCosthg * 100 / numdmOut << "%)" << endl;
    cout << "Outer corrupt=" << (qrsizeIn + 2 * qZsizeIn) << "*" << (qrsizeIn + 2 * qZsizeIn) << "/" << numdmOut << "(" << setprecision(.2) << (float)(qrsizeIn + 2 * qZsizeIn)*(qrsizeIn + 2 * qZsizeIn) * 100 / (numdmOut*9) << "%)" << endl;
#endif


    copyMakeBorder(nestImg, nestImg, msizeOut * 4, msizeOut * 4, msizeOut * 4, msizeOut * 4, BORDER_CONSTANT, Scalar(255, 255, 255));

    imshow("Result nested QR", nestImg);
//
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/Partial.jpg", inImg);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/Partial.jpg", outPartialImg);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/putnestedPartial.jpg", nestPartialImg);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/putnestPartialChg.jpg", nestPartialChgImg);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/putinQR.jpg", oInImg);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/putoutQR.jpg", outImg);
//    imwrite("/Users/rmx/Desktop/實驗output/F1andRBR18.jpg", overlayImg);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/putnested.jpg", nestImg);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/putdiffOut.jpg", diffImg1);
//    imwrite("/Users/rmx/Desktop/Visual Nested QR Codes 03/putdiffIn.jpg", diffImg2);
//    imwrite("/Users/rmx/Desktop/ex_output/nestedImg_a=1.jpg",nestImg);
//    imwrite("/Users/rmx/Desktop/ex_output/F1andRBR18_FnRate_0.5.jpg",nestImg);
    waitKey(0);
    return 0;
}
