#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "trkfileio.h"
using namespace std;

int main()
{
    /// input & output filename
    string strInputFilePath  = "/path/to/input.trk";
    string strOutputFilePath = "/path/to/output.trk";

    /// create reader and open file
    TrkFileReader cReader(strInputFilePath);
    if( !cReader.open() )
        return EXIT_FAILURE;

    /// create writer and create an empty new file
    TrkFileWriter cWriter(strOutputFilePath);
    cWriter.copyHeader(cReader.getHeader());    /// Copy header from the input file, beacuse the coordinates system (LPS/RAS/..) is defined in header.
    if( !cWriter.create() )
        return EXIT_FAILURE;

    /// random sampling (0.1%)
    vector<float> cTrk;
    int iTotalTrackNum = cReader.getTotalTrkNum();  /// total number of tracks in input file
    srand(time(nullptr));                           /// random seed
    for(int i = 0; i < iTotalTrackNum; i++)
    {
        if( rand() % 1000 < 998 )   /// 0.1% random sampling
            continue;
        cTrk.clear();
        cReader.readTrack(i, cTrk); /// read one track from input
        cWriter.appendTrack(cTrk);  /// write it back to output
    }

    /// close input and output file
    cWriter.close();
    cReader.close();

    cout << "Finished" << endl;
    return EXIT_SUCCESS;
}

