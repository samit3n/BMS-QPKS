/* 
 * File:   bms1B.cpp
 */

#include <cstdlib>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>

#include "sndfile.hh"


#define AMPLITUDE (1.0 * 0x7F000000)
#define FCARRIER 1000
#define BAUD_01 1
#define BAUD_00 2
#define BAUD_10 3
#define BAUD_11 4
#define PHASES 4
//experimentally given value
//for time frame detection
#define THRESH 220000


#define DIFF_COEF 10

using namespace std;

/*
 * Returns size of time window which
 * represents number of samples per baud
 * sampleRate / tWindow = baud_rate
 * 
 * detection based on signal difference
 * relative to amplitude
 *
 */

unsigned syncSignal(int * buffer, unsigned sampleRate)
{

    unsigned tWindow = 0,t = 0;
    const double FREQ = 1000.0 / sampleRate;

    int sample, sample2;

    for(int b = 0; b < sampleRate; b++){

        
        sample =  AMPLITUDE * sin(2 * M_PI * FREQ * t + ((BAUD_00 * 2) - 1) * (M_PI / 4) );
        sample2 = AMPLITUDE * sin(2 * M_PI * FREQ * (t+1) + ((BAUD_00 * 2) - 1) * (M_PI / 4) );

        int combDiff = abs(sample - buffer[t]);
       

         if(combDiff > (AMPLITUDE / THRESH) ){
            tWindow = t;

            break;
        }
            
        t++;
    }

    return tWindow;

}

/*
 * @brief Demodulation process
 * 
 * @param: sampleRate - size of buffer
 * @param: time window - samples per baud
 * @param: buffer
 * @param: binary string to be demodulated
 * @param: number of samples red from file
 * @param: where to start reading signal
 *          jumping over sync sequence
 *
 */

int demodulate(unsigned sampleRate, unsigned tWindow, int * buffer, string * data, unsigned samples, unsigned tOffset = 0 )
{

    unsigned diff[PHASES];

    
    const double FREQ = 1000.0 / sampleRate;

    for(unsigned i = 0; i < PHASES; i++)
        diff[i] = 0;

    //singal starting position


    //initialize signal
    static unsigned t;

    t += tOffset;

    int sample[PHASES];
    int diffMin = 2*AMPLITUDE;
    int index;
    int max = 0;
    unsigned ti;
    
    
    do{

        for(unsigned tw = 0; tw < tWindow;tw++){
            
            ti = t % sampleRate;

            //phase shifted samplet to compare in certain time
            
            sample[0] = AMPLITUDE * sin(2 * M_PI * FREQ * t + ((BAUD_01 * 2) - 1) * (M_PI / 4) );
            sample[1] = AMPLITUDE * sin(2 * M_PI * FREQ * t + ((BAUD_00 * 2) - 1) * (M_PI / 4) );
            sample[2] = AMPLITUDE * sin(2 * M_PI * FREQ * t + ((BAUD_10 * 2) - 1) * (M_PI / 4) );
            sample[3] = AMPLITUDE * sin(2 * M_PI * FREQ * t + ((BAUD_11 * 2) - 1) * (M_PI / 4) );

            for(unsigned i = 0; i < PHASES; i++){
                int d = abs( buffer[ti] - sample[i] );

                if( d < diffMin){
                    diffMin = d;
                    index = i;
                }
            }
                
            diff[index]++;
            max = 0;
            diffMin = 2* AMPLITUDE;
            t++;


            if( (t % sampleRate) == 0)
                break;

        }

        //find max

        unsigned max = 0, maxI;

        for(unsigned i = 0; i < PHASES; i++){

            if(diff[i] > max){
                max = diff[i];
                maxI = i;
            }
        }

        switch(maxI){
            
            case 0:
                data->append("01"); break;
            case 1:
                data->append("00"); break;
            case 2:
                data->append("10"); break;
            case 3:
                data->append("11"); break;
            default:
                cerr << "Index error" << endl;

        }
    
        for(unsigned i = 0; i < PHASES; i++)
            diff[i] = 0;

        ti = t % sampleRate;

    }while( ti != 0 && ti < samples);
   
    return 0; 
}

int main(int argc, char** argv) {
    
    SndfileHandle inputFile;
    unsigned sampleRate;

    
    ofstream output;
    int *buffer;
    unsigned tWindow;
   
    if(argc < 2){
        cerr << "Error: Give me output filename\n./bms1B [vstupni_soubor.wav]" << endl;
    }else{ 
        //open file descriptors

        inputFile = SndfileHandle(argv[1]);
        string outname(argv[1]);
       
        outname.resize( outname.rfind(".") );
        outname.append(".txt");
        output.open(outname.c_str(), ofstream::out);
    }
   
    sampleRate = inputFile.samplerate();
    
    //unsigned baudRate = sampleRate / FCARRIER;
    

    buffer = new int[sampleRate];

    int once = 1;

    string data;
    data.reserve(200);
    unsigned len;
    int res;

    while( (len = inputFile.read(buffer, sampleRate)) ){


        if(once){

            tWindow = syncSignal(buffer, sampleRate);
            once = 0;
            demodulate(sampleRate, tWindow, buffer, &data, len, 4*tWindow);
        }else{
        
            res = demodulate(sampleRate, tWindow, buffer, &data, len);
        }
       
        
        output << data;
        data.clear();
        if(res == -1)
            break;

    }

    output.close();
    


    delete [] buffer;
    return EXIT_SUCCESS;
}

