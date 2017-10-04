//
// Created by nsl on 9/23/16.
//

#include "ObjSender.hpp"

ObjSender::ObjSender(AugmentedVR *myAVR, string commPath) : myAVR(myAVR), commPath(commPath) {
    //init http server, need to be run in separate thread

}

ObjSender::~ObjSender() {
//    TcwFile.release();
//
//    PCFile.release();
}


void ObjSender::writeFrameInSeparateFile(){
    writeCurrentFrame();
    writeCurrentTcw();
    writeCurrentTimeStamp();
    writePC();
    writeDynamicPC();
    writeObjectMotionVec();
    writeLowPassObjectMotionVec();
}

char* ObjSender::writeWholeFrameWithFullMetaData(){
    char tmpstr[100];
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    sprintf(tmpstr, "/cam%d/Frame%d.yml", myAVR->CamId, currFrame.frameSeq);
    WholeFrame.open(commPath+tmpstr, cv::FileStorage::WRITE);
    WholeFrame << FRAME << currFrame.FrameLeft;
    WholeFrame << TCW << currFrame.CamMotionMat;
    WholeFrame << TIMESTAMP << (int)currFrame.frameTS;
    WholeFrame << PC << currFrame.pointcloud;
    WholeFrame << DYNAMICPC << currFrame.DynamicPC;
    WholeFrame << MOTIONVEC <<  currFrame.ObjectMotionVec;
    WholeFrame << LOWPASSMOTIONVEC <<  currFrame.LowPass_ObjectMotionVec;
    WholeFrame.release();
    return tmpstr;
}

void ObjSender::writeCurrentFrame(){
    char tmpstr[100];
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    sprintf(tmpstr, "/cam%d/Frame%d.yml", myAVR->CamId, currFrame.frameSeq);
    TcwFile.open(commPath+tmpstr, cv::FileStorage::WRITE);
    TcwFile << FRAME << currFrame.FrameLeft;
    TcwFile.release();
}

void ObjSender::writeCurrentTcw(){
    char tmpstr[100];
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    sprintf(tmpstr, "/cam%d/TcwFrame%d.yml", myAVR->CamId, currFrame.frameSeq);
    TcwFile.open(commPath+tmpstr, cv::FileStorage::WRITE);
    TcwFile << TCW << currFrame.CamMotionMat;
    TcwFile.release();
}

void ObjSender::writeCurrentTimeStamp(){
//    char tmpstr[50];
//    sprintf(tmpstr, "timeFrame%d",myAVR->frameSeq);
//    TcwFile << tmpstr << (int)myAVR->frameTS; //TODO cannot support long??
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    char tmpstr[100];
    sprintf(tmpstr, "/cam%d/timeFrame%d.yml", myAVR->CamId, currFrame.frameSeq);
    TcwFile.open(commPath+tmpstr, cv::FileStorage::WRITE);
    TcwFile << TIMESTAMP << (int)currFrame.frameTS;
    TcwFile.release();
}


void ObjSender::writePC(){
//    char tmpstr[50];
//    sprintf(tmpstr, "fullPCFrame%d",myAVR->frameSeq);
//    PCFile << tmpstr << myAVR->pointcloud;
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    char tmpstr[100];
    sprintf(tmpstr, "/cam%d/fullPCFrame%d.yml", myAVR->CamId, currFrame.frameSeq);
    PCFile.open(commPath+tmpstr, cv::FileStorage::WRITE);
    PCFile << PC << currFrame.pointcloud;
    PCFile.release();
}

void ObjSender::writeDynamicPC(){
//    char tmpstr[50];
//    sprintf(tmpstr, "dynamicPCFrame%d",myAVR->frameSeq);
//    dynamicPCFile << tmpstr << myAVR->DynamicPC;
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    char tmpstr[100];
    sprintf(tmpstr, "/cam%d/dynamicPCFrame%d.yml", myAVR->CamId, currFrame.frameSeq);
    dynamicPCFile.open(commPath+tmpstr, cv::FileStorage::WRITE);
    //TODO investigate 0
    dynamicPCFile << DYNAMICPC << currFrame.DynamicPC;
//    dynamicPCFile << DYNAMICPC << currFrame.DynamicPC;
    dynamicPCFile.release();
}



void ObjSender::writeObjectMotionVec(){
//    char tmpstr[50];
//    sprintf(tmpstr, "dynamicPCFrame%d",myAVR->frameSeq);
//    dynamicPCFile << tmpstr << myAVR->DynamicPC;
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    cout << "sending motion vec\n";
    char tmpstr[100];
    sprintf(tmpstr, "/cam%d/objectMotionVec%d.yml", myAVR->CamId, currFrame.frameSeq);
    motionFile.open(commPath+tmpstr, cv::FileStorage::WRITE);
    //TODO investigate 0
    motionFile << MOTIONVEC <<  currFrame.ObjectMotionVec;
    cout << currFrame.ObjectMotionVec << endl;
    motionFile.release();
}

void ObjSender::writeLowPassObjectMotionVec(){
//    char tmpstr[50];
//    sprintf(tmpstr, "dynamicPCFrame%d",myAVR->frameSeq);
//    dynamicPCFile << tmpstr << myAVR->DynamicPC;
    AVRFrame currFrame = myAVR->getCurrentAVRFrame();
    cout << "sending low pass motion vec\n";
    char tmpstr[100];
    sprintf(tmpstr, "/cam%d/LowPassObjectMotionVec%d.yml", myAVR->CamId, currFrame.frameSeq);
    motionFile.open(commPath+tmpstr, cv::FileStorage::WRITE);
    motionFile << LOWPASSMOTIONVEC <<  currFrame.LowPass_ObjectMotionVec;
    cout << currFrame.LowPass_ObjectMotionVec << endl;
    motionFile.release();
}

//void ObjSender::logFrame(){
//    char tmpstr[100];
//    sprintf(tmpstr, "/cam%d/FrameInfo.txt", myAVR->CamId);
//    logFile.open(commPath+tmpstr, fstream::app);
//    logFile << "Frame " << myAVR->getCurrentAVRFrame()->frameSeq << ": TS: "<< myAVR->getCurrentAVRFrame()->ZEDTS
//            << ": Obj MotionVec: " << myAVR->ObjectMotionVec << ": dist: " << norm(myAVR->ObjectMotionVec)
//            << ": LP_MotionVec: " << myAVR-> Log_LowPassMotionVec << ": dist: " << norm(myAVR->Log_LowPassMotionVec) << endl;
//    logFile.close();
//}