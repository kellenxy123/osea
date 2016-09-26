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
#include <boost/thread.hpp>
#include <stdio.h>
//#include <wfdb/wfdb.h>
#include "wfdb.h"
#include "ecgcodes.h"
#include "ecgmap.h"
#include <pthread.h>
#include "stdio.h"
#include "qrsdet.h"		// For sample rate.

#include "inputs.h"	/* list of records to analyze and definitions of
			   ECG_DB_PATH and REC_COUNT */


// External function prototypes.
void wfdb_read(FILE *fd_reader, int pos, int size, short buffer[]);
int putann2(FILE *fp, WFDB_Annotation *annot, long &last_time,int shift_offset);
void wfdb_p16(unsigned int x, FILE *fp);
//void ResetBDAC(void) ;//bdac.c
//int BeatDetectAndClassify(int ecgSample, int *beatType, int *beatMatch) ;//bdac.c

// Local Prototypes.
//int  NextSample(int *vout,int nosig,int ifreq,
//						int ofreq,int init) ;
int gcd(int x, int y);

// Global variables.

//int ADCZero, ADCUnit, InputFileSampleFrequency ;

#ifdef __STDC__
#define MAINTYPE int
#else
#define MAINTYPE void
#endif
//#define ECG_DB_PATH ". /home/healthwe2/mitdb"
#define ECG_DB_PATH	". /mnt/hgfs/Software/mitdb"
class TESTRECORD{
private:
    int m, n, mn, ot, it ;       //use in the NextSample
    int vv[32], v[32], rval ;    //static to class
    int modeltype[MAXTYPES];     //new create
    int modeltypenum[MAXTYPES];  //new create
    int modelnum;
    int pos;                     //positon of DAT file
    FILE* file;                  //use in the NextSample
    long lasttime;               //use in putann
    temp::Template tmp;          //use in modelwrite
    int md, nd, mnd, otd, itd ;       //use in the NextSample
    int vvd[32], vd[32], rvald ;    //static to class
    int posd;
    FILE* filed;                  //use in the NextSample
    short *vv2d;
    int posdcount;
public:
    int Recordnum;
    //int REC_count;

public:
	int TestRecord();

private:
	int  NextSample(int *vout,int nosig,int ifreq,
					int ofreq,int init) ;
    int  NextSample2(int *vout,int nosig,int ifreq,
                    int ofreq,int init) ;
    void Initial();
};

void bankAgent()
{

    int Records[] = {203,205,207,208,209,210,212,213,214,215,217,219};
    int REC_count1 = (sizeof(Records)/sizeof(int));

    for(int i=2;i<REC_count1;i++)//REC_count1
    {
        TESTRECORD line0;
        line0.Recordnum = Records[i];
        line0.TestRecord();
    }
}

void Joe() {

    int Records[] = {100,101,102,103,104,105,106,107,118,119,200,201,202};
    int REC_count1 = (sizeof(Records)/sizeof(int));     //...

    for(int i=0;i<REC_count1;i++)//REC_count1
    {
       TESTRECORD line1;
       line1.Recordnum = Records[i];
       line1.TestRecord();
    }
}

void Joes() {

    int Records[] = {108,109,111,112,113,114,115,116,117,121,122,123,124};
    int REC_count1 = (sizeof(Records)/sizeof(int));     //...

    for(int i=0;i<REC_count1;i++)//;i++)
    {
        TESTRECORD line1;
        line1.Recordnum = Records[i];
        line1.TestRecord();
    }
}


int main()
{
    //boost::thread thread1(bankAgent); // start concurrent execution of bankAgent
    //boost::thread thread2(Joes); // start concurrent execution of Joe
    //boost::thread thread3(Joe); // start concurrent execution of Joes
    //thread1.join();
    //thread2.join();
    //thread3.join();
    TESTRECORD line1;
    line1.Recordnum = 2402;
    line1.TestRecord();
    return 0;
}

void TESTRECORD::Initial()
{
    m = 0, n = 0, mn = 0, ot = 0, it = 0 ;       //use in the NextSample
    //int vv[32], v[32], rval ;    //static to class
    pos = 0;                     //positon of DAT file
    lasttime = 0;               //use in putann
    md = 0, nd = 0, mnd = 0, otd = 0, itd = 0 ;       //use in the NextSample
    posdcount = 0;
    //int vv[32], v[32], rval ;    //static to class
    posd = 0;
    vv2d = new short[2];
    Recordnum;
    modelnum = 0;
    for(int i=0;i<MAXTYPES;i++)
    {
        modeltype[i] = 0;
        modeltypenum[i] = 0;
    }
}

int TESTRECORD::TestRecord()
	{
    Initial();
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    Template tmp;
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

    char record[10], fname[20] ;
    char record2[100];
    //numvout = 0;
	int i, ecg[2], delay;//int recNum ;
    int ADCZero = 0;
    int ADCUnit = 200;
    int InputFileSampleFrequency = 128;

    sprintf(record,"%d", Recordnum);
    //printf("Record %d\n",Recordnum);//Records[recNum]) ;

    long SampleCount = 0, lTemp, DetectionTime ;
    int beatType, beatMatch ;
    lasttime =0;

    WFDB_Annotation annot ;//write to annot
    sprintf(record2,"%d.atestmulti", Recordnum);
    FILE *fileann = fopen(record2,"wb");

    // Open a 2 channel record
    //read record
    //pos = 0;
    sprintf(record2,"%s/%d.dat", "/home/healthwe2/mitdb/240", Recordnum);
    file = fopen(record2,"rb+");
    fseek(file,0,2);
    long flen=ftell(file); // 得到文件大小
    int length = flen/3;//
    // Initialize sampling frequency adjustment.
    NextSample(ecg,2,InputFileSampleFrequency,SAMPLE_RATE,1) ;      //local

        sprintf(record2,"%s/%d.dat", "/home/healthwe2/mitdb/240", 240);
        filed = fopen(record2,"rb+");
        fseek(filed,0,2);
        long flend=ftell(filed); // 得到文件大小
        int lengthd = flend/3;//
        int ecgd[2];
        NextSample2(ecgd,1,InputFileSampleFrequency,SAMPLE_RATE,1) ;      //local
   // Initialize beat detection and classification.
    BDAC bdac;
    bdac.ResetBDAC() ;                                                   //bdac.c
    SampleCount = 0 ;
        //NextSample(ecg,2,InputFileSampleFrequency,SAMPLE_RATE,0);
    // Read data from MIT/BIH file until tre is none left.
    while(NextSample(ecg,2,InputFileSampleFrequency,SAMPLE_RATE,0) >= 0 && pos<=lengthd)  //local
    {
        NextSample2(ecgd,1,InputFileSampleFrequency,SAMPLE_RATE,0 );      //local

        ++SampleCount ;
       // printf("SampleCount %d\n",SampleCount) ;

        // Set baseline to 0 and resolution to 5 mV/lsb (200 units/mV)
        lTemp = ecg[0]-ADCZero ;
       // printf("SampleCount=%d,ecg[0]=%d\n",SampleCount,ecg[0]);
        lTemp *= 200 ;			lTemp /= ADCUnit ;			ecg[0] = lTemp ;
        //printf("ecg[0]---=%d\n",ecg[0]);
        // Pass sample to beat detection and classification.

        delay = bdac.BeatDetectAndClassify(ecg[0], &beatType, &beatMatch) ;    //bdac.c

        // If a beat was detected, annotate the beat location
        // and type.

        if(delay != 0)
        {
            DetectionTime = SampleCount - delay ;

            // Convert sample count to input file sample
            // rate.
            DetectionTime *= InputFileSampleFrequency ;
            DetectionTime /= SAMPLE_RATE ;
            annot.time = DetectionTime ;
            annot.anntyp = beatType ;
            annot.aux = NULL ;
            putann2(fileann,&annot,lasttime,0);

            /*FILE* filetxt = fopen("detect100.txt","a+");
            fprintf(filetxt,"%d\n",DetectionTime);
            fclose(filetxt);*/
            //BEGIN TO WRITE THE TMP
            int morphTypenew = bdac.match1.morphType;
            if(beatType == 13){//Q
                printf("SampleCount = %d, DetectionTime = %d\n", SampleCount, DetectionTime);
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
                    //printf("Record=%d,DetectionTime=%d,\tnumtypeout=%d,\tm_clusters=%d\n",Recordnum,DetectionTime,numtypeout,m_clusters.size());
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
                }//*/
            }
        }
    }
    printf("Record,%d,%d\n",Recordnum,m_type.size());
    wfdb_p16(0, fileann);//STOP WRITE THE ATEST FILE
    fclose(fileann);//CLOSE WRITE THE ATEST FILE
    fclose(file); //CLOSE THE DAT FILE

    sprintf(record2,"%d.tmp", Recordnum);
    // Write the new address book back to disk.
    int Nnum = 0, Vnum=0, Anum = 0;
    int countN = 0,countV = 0,countA=0;
    fstream tempfile(record2, ios::out | ios::trunc | ios::binary);
        for (int j = 0; j < m_type.size(); ++j)
        {
            int type = m_type[j];
            //printf("%d\t%d\t%d\n",j,type,m_clusters[j].size());
            Template1 *tmp1;
            int numID = 0;
            if(1 == type)
            {
                tmp1 = tmpN->add_template1s();
                numID = Nnum++;
                countN += m_clusters[j].size();
            }
            else if(5 == type){
                tmp1 = tmpV->add_template1s();
                numID = Vnum++;
                countV += m_clusters[j].size();
            }
            else{
                tmp1 = tmpA->add_template1s();
                numID = Anum++;
                countA += m_clusters[j].size();
            }

            //tmp1->set_id(0);
            Template2 *tmp2 = tmp1->add_template2s();
            tmp2->set_id(numID);
            for(int k=0;k<m_clusters[j].size();k++){
                //printf("%d\n",m_clusters[j][k]);
                tmp2->add_positions_of_beats(m_clusters[j][k]);
            }
        }
    //printf("Record,%d,norm:%d\tv :%d\t other:%d\n",Recordnum,countN,countV,countA);
    if (!tmp.SerializeToOstream(&tempfile)) {
        cerr << "Failed to write address book." << endl;
        return -1;
    }
    google::protobuf::ShutdownProtobufLibrary();

    // Reset database after record is done.

  #if 0
          /* This code is obsolete.  The annotation files are always
             written into "<record>.ate" in the current directory.
                     They do not need to be copied in order to be read by bxbep,
             if the WFDB path includes both the current current directory
             and the one containing the .atr reference annotation files.
                   */

		// Copy "atest.<record>" to "<record>.ate" for future ascess.
		// (This is necessary for PC files)

		sprintf(fname,"%s.ate",record) ;
		newAnn0 = fopen(fname,"rb") ;
		sprintf(fname,"%s%s.ate",ECG_DB_PATH,record) ;
		newAnn1 = fopen(fname,"wb") ;

		// Copy byte image of annotation file in this
		// directory to a correctly named file in the
		// database directory.

		while(fread(&byte,sizeof(char),1,newAnn0) == 1)
			fwrite(&byte,sizeof(char),1,newAnn1) ;

		fclose(newAnn0) ;
		fclose(newAnn1) ;
#endif
		//}
	}

/**********************************************************************
	NextSample reads MIT/BIH Arrhythmia data from a file of data
	sampled at ifreq and returns data sampled at ofreq.  Data is
	returned in vout via *vout.  NextSample must be initialized by
	passing in a nonzero value in init.  NextSample returns -1 when
   there is no more data left.
***********************************************************************/

int  TESTRECORD::NextSample(int *vout,int nosig,int ifreq,
						int ofreq,int init)
	{
	int i ;short *vv2;
    vv2 = new short[2];

	if(init)
		{
		i = gcd(ifreq, ofreq);
		m = ifreq/i;
		n = ofreq/i;
		mn = m*n;
		ot = it = 0 ;
		//getvec(vv) ;
		//rval = getvec(v) ;
        wfdb_read(file,pos++,1,vv2);
        for(i = 0; i < nosig; ++i)
           vv[i] = int(vv2[i]);

        wfdb_read(file,pos++,1,vv2);
        for(i = 0; i < nosig; ++i)
            v[i] = vv2[i];

            rval = 2;
		}

	else
		{
            //printf("next\n");
		while(ot > it)
			{
	    	for(i = 0; i < nosig; ++i)
                vv[i] = v[i] ;

			//rval = getvec(v) ;
            wfdb_read(file,pos++,1,vv2);
            for(i = 0; i < nosig; ++i)
                v[i] = int(vv2[i]);

		    if (it > mn) { it -= mn; ot -= mn; }
		    it += n;
		    }		
	    for(i = 0; i < nosig; ++i)
            {
            vout[i] = vv[i] + (ot % n) * (v[i] - vv[i]) / n;
            }
		ot += m;

		}
        /*FILE* filevout=fopen("100new.txt","a+");
        fprintf(filevout,"%d\t%d\t%d\n",numvout++,vout[0],vout[1]);
        fclose(filevout);*/
        delete[]vv2;
	return(rval) ;
	}

int  TESTRECORD::NextSample2(int *vout,int nosig,int ifreq,
                            int ofreq,int init)
{
    int i ;
    //short *vv2;
    //vv2 = new short[2];

    if(init)
    {
        i = gcd(ifreq, ofreq);
        md = ifreq/i;
        nd = ofreq/i;
        mnd = md*nd;
        otd = itd = 0 ;
        //getvec(vv) ;
        //rval = getvec(v) ;
        wfdb_read(filed,posd++,1,vv2d);
            vvd[0] = int(vv2d[0]);
        //posdcount++;

        //wfdb_read(filed,posd++,1,vv2);
        //for(i = 0; i < nosig; ++i)
            vd[0] = vv2d[1];
        //posdcount++;

        rvald = 2;
    }

    else
    {
        //printf("next\n");
        while(otd > itd)
        {
            for(i = 0; i < nosig; ++i)
                vvd[i] = vd[i] ;

            //rval = getvec(v) ;
            if(posdcount%2==0) {
                wfdb_read(filed, posd++, 1, vv2d);
                vd[0] = int(vv2d[0]);
            }
            else{
                if (vv2d[1] & 0x800)
                    vv2d[1] |= ~(0xfff); //negative  data, make all the high bit(12 and after) 1
                vd[0] = int(vv2d[1]);
            }

            posdcount++;

            if (itd > mnd) { itd -= mnd; otd -= mnd; }
            itd += nd;
        }
        for(i = 0; i < nosig; ++i)
        {
            vout[i] = vvd[i] + (otd % nd) * (vd[i] - vvd[i]) / nd;
        }
        //posdcount++;
        otd += md;

    }
    /*FILE* filevout=fopen("100new.txt","a+");
    fprintf(filevout,"%d\t%d\t%d\n",numvout++,vout[0],vout[1]);
    fclose(filevout);*/
    return(rvald) ;
}
// Greatest common divisor of x and y (Euclid's algorithm)

int gcd(int x, int y)
	{
	while (x != y) {
		if (x > y) x-=y;
		else y -= x;
		}
	return (x);
	}



