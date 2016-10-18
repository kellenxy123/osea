/*****************************************************************************
FILE:  easytest.cpp
AUTHOR:	Patrick S. Hamilton
REVISED:	5/13/2002 (PSH); 4/10/2003 (GBM)
  ___________________________________________________________________________

easytest.cpp: Use bdac to generate an annotation file.
Copyright (C) 2001 Patrick S. Hamilton
Copyright (C) 1999 George B. Moody

This file is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This software is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Library General Public License for more
details.

You should have received a copy of the GNU Library General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the author by e-mail (pat@eplimited.edu) or postal mail
(Patrick Hamilton, E.P. Limited, 35 Medford St., Suite 204 Somerville,
MA 02143 USA).  For updates to this software, please visit our website
(http://www.eplimited.com).
  __________________________________________________________________________

Easytest.exe is a simple program to help test the performance of our
beat detection and classification software. Data is read from the
indicated ECG file, the channel 1 signal is fed to bdac.c, and the
resulting detections are saved in the annotation file <record>.ate.
<record>.ate may then be compared to <record>.atr to using bxb to
analyze the performance of the the beat detector and classifier detector.

Note that data in the MIT/BIH Arrythmia database file has been sampled
at 360 samples-per-second, but the beat detection and classification
software has been written for data sampled at 200 samples-per-second.
Date is converterted from 360 sps to 200 sps with the function NextSample.
Code for resampling was copied from George Moody's xform utility.  The beat
locations are then adjusted back to coincide with the original sample
rate of 360 samples/second so that the annotation files generated by
easytest can be compared to the "atruth" annotation files.

This file must be linked with object files produced from:
	wfdb software library (source available at www.physionet.org)
	analbeat.cpp
	match.cpp
	rythmchk.cpp
	classify.cpp
	bdac.cpp
	qrsfilt.cpp
	qrsdet.cpp
	postclass.cpp
	noisechk.cpp
  __________________________________________________________________________

  Revisions
	4/13/02:
		Added conditional define statements that allow MIT/BIH or AHA
			records to be processed.
		Normalize input to 5 mV/LSB (200 A-to-D units/mV).

	4/10/03:
		Moved definitions of Record[] array, ECG_DB_PATH, and REC_COUNT
			into "input.h"
*******************************************************************************/
//#include "/home/healthwe2/Downloads/pthreads.2/pthread.h"
#include "string.h"
#include "win.h"

#include <stdio.h>
//#include <wfdb/wfdb.h>
#include "wfdb.h"
#include "ecgcodes.h"
#include "ecgmap.h"
#include <pthread.h>
#include "stdio.h"
#include "qrsdet.h"		// For sample rate.
#include "config.h"
//#include "inputs.h"	/* list of records to analyze and definitions of
//			   ECG_DB_PATH and REC_COUNT */


// External function prototypes.
//void wfdb_read(FILE *fd_reader, int pos, int size, short buffer[]);
int putann2(FILE *fp, WFDB_Annotation *annot, long &last_time,int shift_offset);
void wfdb_p16(unsigned int x, FILE *fp);
//void write_tmp(char* filename,Template* tmp,std::vector< std::vector<int> > m_clusters, std::vector<int> m_type);
//void ResetBDAC(void) ;//bdac.c
//int BeatDetectAndClassify(int ecgSample, int *beatType, int *beatMatch) ;//bdac.c

// Local Prototypes.
char *get_file_name(const char *path);
void remove_extension(char* path);


#ifdef __STDC__
#define MAINTYPE int
#else
#define MAINTYPE void
#endif

/*class TESTRECORD{
private:
    int modeltype[MAXTYPES];     //new create
    int modeltypenum[MAXTYPES];  //new create
    int modelnum;
    long lasttime;               //use in putann
    temp::Template tmp;          //use in modelwrite


public:
    int Recordnum;

public:
	int TestRecord(const char *data_file_path);

private:
    void Initial();
};
*/
/*int main(int argc, char* argv[])
{
    TESTRECORD line;
    line.TestRecord(argv[1]);
    return 0;
}
*/
void TESTRECORD::Initial()
{
    lasttime = 0;               //use in putann
    Recordnum;
    modelnum = 0;
    for(int i=0;i<MAXTYPES;i++)
    {
        modeltype[i] = 0;
        modeltypenum[i] = 0;
    }
}

int TESTRECORD::TestRecord(const char *data_file_path)
	{
    //int _MAX_PATH =500;

    char *data_file_name;
    char date_tmp[_MAX_PATH]; //XXX/201414/
    char ecg_file_name[_MAX_PATH];
    char date_path[_MAX_PATH];
    char ecg_filtered_data_file_path[_MAX_PATH]; //saved file name
    char ecg_annotation_file_path[_MAX_PATH]; //saved file name
    //char ecg_AF_file_path[_MAX_PATH]; //saved file name
    char ecg_head_file_path[_MAX_PATH]; //saved file name           7
    char ecg_tmp_file_path[_MAX_PATH]; //saved file name           7
    //char ecg_write_path[_MAX_PATH];// the path with date stern directory
    char ecg_config_file_path[_MAX_PATH];// the path with config
    string WRITE_PATH,READ_PATH;
    memset(date_tmp, 0, _MAX_PATH);
    data_file_name = get_file_name(data_file_path);
    strncpy(ecg_file_name, data_file_name,_MAX_PATH);
    remove_extension(ecg_file_name);
    strncpy(date_tmp, data_file_path, strlen(data_file_path) - strlen(data_file_name) - 1);
    strncpy(date_path, get_file_name(date_tmp),_MAX_PATH);

        conf::Instance()->Load(SYS_CONF);
        conf::Instance()->Get("write_path", WRITE_PATH);
        conf::Instance()->Get("read_path", READ_PATH);
    snprintf(ecg_filtered_data_file_path,_MAX_PATH, "%s/%s/%s.dat", WRITE_PATH.c_str(), date_path, ecg_file_name);
    snprintf(ecg_annotation_file_path,_MAX_PATH, "%s/%s/%s.bsp", WRITE_PATH.c_str(), date_path, ecg_file_name);
    //sprintf(ecg_AF_file_path, "%s/%s/%s.af", WRITE_PATH.c_str(), date_path, ecg_file_name);
    snprintf(ecg_head_file_path,_MAX_PATH, "%s/%s/%s.hea", WRITE_PATH.c_str(), date_path, ecg_file_name);
    snprintf(ecg_tmp_file_path,_MAX_PATH, "%s/%s/%s.tmp", WRITE_PATH.c_str(), date_path, ecg_file_name);
    snprintf(ecg_config_file_path,_MAX_PATH, "%s/%s/%s.cnf", READ_PATH.c_str(), date_path, ecg_file_name);
    //sprintf(ecg_write_path, "%s/%s/%s", WRITE_PATH.c_str(), date_path,ecg_file_name);

    Initial();

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    //Template tmp;
    temp::BeatTemplate *tmpN=tmp.add_beat_templates();
    temp::BeatTemplate *tmpA=tmp.add_beat_templates();
    temp::BeatTemplate *tmpV=tmp.add_beat_templates();
    tmpA->set_type(BeatTemplate::A);
    tmpN->set_type(BeatTemplate::N);
    tmpV->set_type(BeatTemplate::V);

    int modeltypev[MAXTYPES+1];
    for(int i=0;i<MAXTYPES+1;i++)
    {
        modeltypev[i] = 0;
    }
    std::vector< std::vector<int> > m_clusters;
    std::vector<int> m_type;

   //find QRS
	int delay;
    int InputFileSampleFrequency = 128;
    long SampleCount = 0;
    long DetectionTime = 0;
    int beatType, beatMatch ;
    lasttime =0;

    //write to annot
    WFDB_Annotation annot ;
    FILE *fileann = fopen(ecg_annotation_file_path,"wb");//


         // Open a 1 channel record
       //read record

        FILE* filed = fopen(data_file_path,"r");//
         if(!filed){
             printf("please check the file:%s!",data_file_path);
             return 0;
         }
         fseek(filed,0,2);
         long flend=ftell(filed); // 得到文件大小
         int lengthd = flend/3;
         int posd=0;
         char* lpc = new char[flend];//(char *) malloc(flend);//
         fseek(filed, 0, SEEK_SET);
         fread(lpc, flend,1,filed);
         int dataIN;
         int dataOUT;

         int file2c_lsize = flend*2;
         FILE* fp = fopen(ecg_filtered_data_file_path,"w");//
         if (fp == NULL){
             printf("can't open the output 2-channel file!");
             return 0;
         }

         char *buf = new char[file2c_lsize];//(char *) malloc(file2c_lsize);
         char* lpc2 = (char *) buf;

             //write the head file
         FILE *filehea = fopen(ecg_head_file_path, "w");
         int sNum = 2;
         float sr = 128;
         int res = fprintf(filehea, "%s %d %3.0f %d\n", ecg_file_name, sNum, sr, file2c_lsize);
         int eNum = 0;
         int umv = 549, bits = 212, resolution = 12, zero = 0, crc = 0, firstdata = 0;
         res = fprintf(filehea, "%s.dat %d %d %d %d %d %d %d %s\n", ecg_file_name, bits, umv, resolution, zero, firstdata, crc,
                       0, "v5");
         res = fprintf(filehea, "%s.dat %d %d %d %d %d %d %d %s", ecg_file_name, bits, umv, resolution, zero, firstdata, crc, 0,
                       "v5");
         fclose(filehea);

        // Initialize beat detection and classification.
         BDAC bdac;
         bdac.ResetBDAC() ;                                                   //bdac.c

         // Read data from MIT/BIH file until tre is none left.
         while( posd < lengthd)  //local
         {
            // NextSample2(ecgd,1,InputFileSampleFrequency,SAMPLE_RATE,0 );      //local
             if(SampleCount%2==0){
                 dataIN = MAKEWORD(lpc[posd * 3 ], (lpc[posd * 3 + 1 ] & 0x0f));
             }
             else{
                 dataIN = MAKEWORD(lpc[posd * 3 + 2], (lpc[posd * 3 + 1] & 0xf0) >> 4);
                 posd++;
             }

             if (dataIN & 0x800)
                 dataIN |= ~(0xfff); //negative  data, make all the high bit(12 and after) 1
             else dataIN &= 0xfff;        //positive data, make all the hight bit 0
             //printf("SampleCount:%d\n",SampleCount);
             ++SampleCount ;

             // Pass sample to beat detection and classification.

             delay = bdac.BeatDetectAndClassify(dataIN, &beatType, &beatMatch) ;    //bdac.c

             // If a beat was detected, annotate the beat location
             // and type.
             dataOUT = bdac.qrsdet1.datafilt;
                 lpc2[0] = LOBYTE((short) dataOUT);
                 lpc2[1] = 0;
                 lpc2[1] = HIBYTE((short) dataOUT) & 0x0f;
                 lpc2[2] = LOBYTE((short) dataIN);
                 lpc2[1] |= HIBYTE((short) dataIN) << 4;
                 lpc2 += 3;
            /*      lpc2[0] = LOBYTE((short) dataIN);
                 lpc2[1] = 0;
                 lpc2[1] = HIBYTE((short) dataIN) & 0x0f;
                 lpc2[2] = LOBYTE((short) dataOUT);
                 lpc2[1] |= HIBYTE((short) dataOUT) << 4;
                 lpc2 += 3;
*/
             if(delay != 0)
             {
                 /*if(delay!=76){
                     printf("delay = %d,SampleCount = %d\n",delay,SampleCount);
                 }*/
                 DetectionTime = SampleCount - delay ;

                 // Convert sample count to input file sample
                 // rate.
                 DetectionTime *= InputFileSampleFrequency ;
                 DetectionTime /= SAMPLE_RATE ;
                 annot.time = DetectionTime ;
                 annot.anntyp = beatType ;
                 annot.aux = NULL ;
                 putann2(fileann,&annot,lasttime,0);

                 //BEGIN TO WRITE THE TMP
                 int morphTypenew = bdac.match1.morphType;
                 if(beatType == 13){//Q
                     if(m_clusters.size()<1){
                         m_type.push_back(beatType);
                         std::vector<int> newvec;
                         newvec.push_back(DetectionTime);
                         m_clusters.push_back(newvec);
                     }
                     else
                     {
                         m_clusters[0].push_back(DetectionTime);
                     }
                 }
                 else if(bdac.match1.lastBeatWasNew == 1 )
                 {
                     if(m_type.size()<=MAXTYPES) {
                         modeltype[m_type.size()-1] = beatType;
                         modeltypenum[m_type.size()-1] = m_type.size();
                     }
                     else{
                         modeltype[morphTypenew] = beatType;
                         modeltypenum[morphTypenew] = m_type.size();
                     }
                     m_type.push_back(beatType);
                     std::vector<int> newvec;
                     newvec.push_back(DetectionTime);
                     m_clusters.push_back(newvec);
                 }
                 else {
                     int numtypeout = modeltypenum[morphTypenew];
                     if (m_type[numtypeout]==beatType) {
                        m_clusters[numtypeout].push_back(DetectionTime);
                     }
                     else{
                         if(modeltypev[morphTypenew]==0){
                             m_type.push_back(beatType);
                             modeltypev[morphTypenew]= m_type.size()-1;
                             std::vector<int> newvec;
                             newvec.push_back(DetectionTime);
                             m_clusters.push_back(newvec);
                         }
                         else{
                             numtypeout = modeltypev[morphTypenew];
                             m_clusters[numtypeout].push_back(DetectionTime);
                         }
                     }
                 }
             }
         }

         fwrite( buf, sizeof(char), file2c_lsize, fp);
         fclose(fp);
         //delete[]buf;
         delete[]lpc;

         printf("Record:%s,%d\n",data_file_name,m_type.size());
         wfdb_p16(0, fileann);//STOP WRITE THE ATEST FILE
         fclose(fileann);//CLOSE WRITE THE ATEST FILE
         fclose(filed); //CLOSE THE DAT FILE

         // Write the new address book back to disk.
         int Nnum = 0, Vnum=0, Anum = 0;
         int countN = 0,countV = 0,countA=0;
         bool Nfind1 = false;
         int Nfind1ID = 0;
         bool Vfind1 = false;
         int Vfind1ID = 0;
         bool Afind1 = false;
         int Afind1ID = 0;
         fstream tempfile(ecg_tmp_file_path, ios::out | ios::trunc | ios::binary);
         for (int j = 0; j < m_type.size(); ++j)
         {
             int type = m_type[j];
             //printf("%d\t%d\t%d\n",j,type,m_clusters[j].size());
             Template1 *tmp1;
             int numID = 0;
             if(1 == type)
             {
                 if(m_clusters[j].size()>MERRGENUM){
                     tmp1 = tmpN->add_template1s();
                     numID = Nnum++;
                     countN += m_clusters[j].size();
                 }
                 else
                 {
                     if(!Nfind1){
                         Nfind1 = true;
                         Nfind1ID = j;
                     }
                     else{
                         for (int k = 0; k < MERRGENUM; ++k) {
                             m_clusters[Nfind1ID].push_back(m_clusters[j][k]);
                         }
                     }
                 }
             }
             else if(5 == type){
                 if(m_clusters[j].size()>MERRGENUM)
                 {
                     tmp1 = tmpV->add_template1s();
                     numID = Vnum++;
                     countV += m_clusters[j].size();
                 }
                 else{
                     if(!Vfind1){
                         Vfind1 = true;
                         Vfind1ID = j;
                     }
                     else{
                         for (int k = 0; k < MERRGENUM; ++k) {
                             m_clusters[Vfind1ID].push_back(m_clusters[j][k]);
                         }
                     }
                 }
             }
             else{
                 if(m_clusters[j].size()>MERRGENUM)
                 {
                     tmp1 = tmpA->add_template1s();
                     numID = Anum++;
                     countA += m_clusters[j].size();
                 }
                 else{
                     if(!Afind1){
                         Afind1 = true;
                         Afind1ID = j;
                     }
                     else{
                         for (int k = 0; k < MERRGENUM; ++k) {
                             m_clusters[Afind1ID].push_back(m_clusters[j][k]);
                         }
                     }
                 }
             }
             if(m_clusters[j].size()>MERRGENUM)
             {
                 Template2 *tmp2 = tmp1->add_template2s();
                 tmp2->set_id(numID);
                 for(int k=0;k<m_clusters[j].size();k++)
                 {
                     tmp2->add_positions_of_beats(m_clusters[j][k]);
                 }
             }
         }
         if(Nfind1 == true)
         {
             Template1 *tmp1;
             tmp1 = tmpN->add_template1s();
             int numID = Nnum++;
             countN += m_clusters[Nfind1ID].size();
             Template2 *tmp2 = tmp1->add_template2s();
             tmp2->set_id(numID);
             for(int k=0;k<m_clusters[Nfind1ID].size();k++)
             {
                 tmp2->add_positions_of_beats(m_clusters[Nfind1ID][k]);
             }
         }
        if(Vfind1 == true)
        {
            Template1 *tmp1;
            tmp1 = tmpV->add_template1s();
            int numID = Vnum++;
            countV += m_clusters[Vfind1ID].size();
            Template2 *tmp2 = tmp1->add_template2s();
            tmp2->set_id(numID);
            for(int k=0;k<m_clusters[Vfind1ID].size();k++)
            {
                tmp2->add_positions_of_beats(m_clusters[Vfind1ID][k]);
            }
        }
        if(Afind1 == true)
        {
            Template1 *tmp1;
            tmp1 = tmpA->add_template1s();
            int numID = Anum++;
            countA += m_clusters[Afind1ID].size();
            Template2 *tmp2 = tmp1->add_template2s();
            tmp2->set_id(numID);
            for(int k=0;k<m_clusters[Afind1ID].size();k++)
            {
                tmp2->add_positions_of_beats(m_clusters[Afind1ID][k]);
            }
        }
        printf("all tmp = %d\n", Anum + Vnum +Nnum);
         if (!tmp.SerializeToOstream(&tempfile)) {
             cerr << "Failed to write address book." << endl;
             return -1;
         }
         google::protobuf::ShutdownProtobufLibrary();

         return 0;
         }
void write_tmp(char* filename,Template* tmp)
{

}

     char *get_file_name(const char *path) {
         char *ssc;
         int l = 0;
         char *name = (char *) path;
         ssc = strstr(name, "/");
         while (ssc) {
             l = strlen(ssc) + 1;
             name = &name[strlen(name) - l + 2];
             ssc = strstr(name, "/");
         };
         return name;
     }

     void remove_extension(char* path) {
         for (int i = (int) strlen(path) - 1; i > 0; i--) {
             if (path[i] == '.') {
                 path[i] = 0;
                 return;
             }
         }
     }
