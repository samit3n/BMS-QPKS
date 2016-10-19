/* 
 * File:   bms1A.cpp
 */

#include <cstdlib>
#include <math.h>
#include <iostream>
#include <fstream>

#include "sndfile.hh"

#define SAMPLE_RATE 42000
#define CHANELS 1
#define FORMAT (SF_FORMAT_WAV | SF_FORMAT_PCM_24)
#define AMPLITUDE (1.0 * 0x7F000000)

#define FREQ (1000.0 / SAMPLE_RATE)
#define BAUD_RATE 1000
#define BAUD_SIZE (SAMPLE_RATE / BAUD_RATE)


using namespace std;

/*
 *
 * @brief Map input symbols to shift
 *
 * 01 -> 2 (PI / $)
 * 00 -> 1 (3PI /4)
 * 10 -> 3 (5PI / 4)
 * 11 -> 4 (7PI / 4)
 * 
 */


int inp2shift(const char * inp)
{

    if(inp[0] == '0'){
        
        if(inp[1] == '0')
            return 2;
        else
            return 1;

    }else{

        if( inp[1] == '1')
            return 4;
        else
            return 3;
    }
}

/*
 * inp2shift testing
 */

void test()
{
 
    cout << inp2shift("00") << endl; //2
    cout << inp2shift("01") << endl; //1
    cout << inp2shift("10") << endl; //3
    cout << inp2shift("11") << endl; //4

}

/*
 * 
 * QPSK samplet testing
 *
 */

void sine(int baud = 1)
{
    
    int * buffer = new int[SAMPLE_RATE];

    cout << "sin(PI) = "  << sin(M_PI) << endl;
    cout << "sin(PI/4) = " << sin(M_PI / 4) << endl;
    cout << "sin(3PI/4) = " << sin(3* M_PI / 4) << endl;
    cout << "sin(5PI/4) = " << sin(5 * M_PI / 4) << endl;
    cout << "sin(7PI/4) = " << sin(7 * M_PI / 4) << endl;
            
   unsigned cnt = 0; 

    for(unsigned t = 0; t < SAMPLE_RATE; t++){
        if(cnt == SAMPLE_RATE / 4){
           baud++;
           cnt = 0;
        }
    
         buffer [t] = AMPLITUDE * sin(2 * M_PI * FREQ * t + ((baud * 2) - 1) * (M_PI / 4) );
         cnt++;
    }


    SndfileHandle outputFile;
    outputFile = SndfileHandle("test_sine.waw", SFM_WRITE, FORMAT, CHANELS, SAMPLE_RATE);

    outputFile.write(buffer, SAMPLE_RATE);

}

/*
 * Main modulation function
 *
 */

int modulate(ifstream * file, SndfileHandle * ofile)
{
    
    char inbuf[3] ;

    int *buffer = new int[SAMPLE_RATE];

    int baud;
    unsigned t = 0;

    for(unsigned init = 0; init < 4; init++ ){ 

        baud = init % 2 == 0 ? 2 : 4;

        for(int b = 0; b < (unsigned) BAUD_SIZE; b++){ 
        
                buffer [t] = AMPLITUDE * sin(2 * M_PI * FREQ * t + ((baud * 2) - 1) * (M_PI / 4) );
                t++;
        }
    }

    //modulating input sequence

    while( file->read(inbuf,2)) {
    
        ios_base::iostate st = file->rdstate();

        if( ( st & std::ifstream::failbit ) != 0  && (st & std::ifstream::eofbit) != 0){
            cerr << "Input sequence len probably odd\n";
            delete[] buffer;
            return 1;
        }

        //shift determining
        baud = inp2shift(inbuf);
        cout << baud  << " ,";
           
       //signal generation 
        for(int b = 0; b < (unsigned) BAUD_SIZE; b++){ 
        
                buffer [t % SAMPLE_RATE] = AMPLITUDE * sin(2 * M_PI * FREQ * t + ((baud * 2) - 1) * (M_PI / 4) );
                t++;

                //signal output at the end of buffer capacity
                if( t > 0 && (t % SAMPLE_RATE) == 0 ){
                    ofile->write(buffer, SAMPLE_RATE);
                }
        }
    }
    
    //signal output otherwise
    if( (t % SAMPLE_RATE) > 0) 
        ofile->write(buffer, (t % SAMPLE_RATE) );
    

    delete [] buffer;
    
    return 0; 

}
int main(int argc, char** argv) {

    ifstream input;
    SndfileHandle outputFile;


    //parameters processing
    
    if(argc <  2){

        cerr << "Give me input file\n ./bms1A [INPUT_FILE] [OUTPUT_FILE]" ;
        return 1;

    }else{ 

        //output file open
        input.open(argv[1], ios::in);

        string outname(argv[1]);
       
        outname.resize( outname.rfind(".") );
        outname.append(".wav");

        outputFile = SndfileHandle(outname.c_str(), SFM_WRITE, FORMAT, CHANELS, SAMPLE_RATE);


    }

    // modulation function
    if(modulate(&input, &outputFile))
        return EXIT_FAILURE;
     
    
    return EXIT_SUCCESS;
}

