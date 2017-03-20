#include "trkfileio.h"
TrkFileReader::TrkFileReader()
{
    xInit();
}


TrkFileReader::TrkFileReader(string &strFilepath)
{
    xInit();
    m_cFilepath = strFilepath;
}

TrkFileReader::~TrkFileReader()
{
    close();
}

void TrkFileReader::xInit()
{
    m_cFilepath.clear();
    m_iPntPos = -1;
    m_iTrkPos = -1;
    m_iPntPosMax = -1;
}


bool TrkFileReader::open()
{
    /// open trk file
    m_cFile.open(m_cFilepath.c_str(), ios::in|ios::binary);
    if( !m_cFile.is_open() )
    {
        cerr << "fail to open file" << endl;
        return false;
    }
    m_cFile.seekg(0);
    assert(sizeof(m_cHeader) == TRK_HEADER_SIZE);         ///< make sure header is 1000 bytes
    m_cFile.read((char*)(&m_cHeader), TRK_HEADER_SIZE);   ///< read header
    assert(m_cFile.gcount() == TRK_HEADER_SIZE);          ///< read check

    if( m_cHeader.n_properties != 0 )
        cerr << "n_properties is not 0:" << m_cHeader.n_properties << endl;
    if( m_cHeader.n_scalars != 0 )
        cerr << "n_scalars is not 0:" << m_cHeader.n_scalars << endl;

    /// build the random access table (track number to file offset map)
    static TrkInfo cTrkInfo;
    m_iTrkPos = 0;
    m_cRandomAccessMap.clear();
    int32_t iTrkSizeInByte = 0;
    while(!m_cFile.eof())
    {
        /// read track info
        m_cFile.read((char*)(&m_iPntPosMax), sizeof(int32_t));
        iTrkSizeInByte = ((3+m_cHeader.n_scalars)*m_iPntPosMax + m_cHeader.n_properties)*sizeof(float);
        /// save track position and length in byte
        cTrkInfo.numPntsInTrk = m_iPntPosMax;
        cTrkInfo.trackOffset = m_cFile.tellg();
        cTrkInfo.lengthInByte = iTrkSizeInByte;
        m_cRandomAccessMap[m_iTrkPos] = cTrkInfo;
        /// move to next track
        m_cFile.seekg( iTrkSizeInByte, ios::cur);
        m_cFile.peek();
        ++m_iTrkPos;
    }
    xResetPos();
    return true;

}

void TrkFileReader::close()
{
    xInit();
    m_cFile.close();
}

bool TrkFileReader::readTrack(size_t iTrkIdx, vector<float> &points)
{
    map<int32_t,TrkInfo>::iterator it = m_cRandomAccessMap.find(static_cast<int32_t>(iTrkIdx));
    if( it ==  m_cRandomAccessMap.end() )
        return false;
    TrkInfo& cTrkInfo = it->second;
    int iTotalPoints = cTrkInfo.numPntsInTrk;
    static float afPoint[3];
    m_cFile.seekg(cTrkInfo.trackOffset);    /// seek to track
    points.clear();
    for(int i = 0; i < iTotalPoints; i++)
    {
        m_cFile.read((char*)(afPoint), sizeof(float)*3);    /// read three component
        m_cFile.seekg(sizeof(float)*m_cHeader.n_scalars, ios::cur); /// ignore scalars
        for(int j = 0; j < 3; ++j)
            points.push_back(afPoint[j]);
    }
    return true;

}

bool TrkFileReader::readPoint(int iTrkIdx, int iPntIdx, vector<float> &point)
{
    map<int32_t,TrkInfo>::iterator it = m_cRandomAccessMap.find(iTrkIdx);
    if( it ==  m_cRandomAccessMap.end() )
        return false;
    TrkInfo& rcTrk = it->second;
    streamoff iOffset = rcTrk.trackOffset+(3+m_cHeader.n_scalars)*iPntIdx*sizeof(float);
    m_cFile.seekg(iOffset);

    float fCoord;
    point.clear();
    for(int i = 0; i < 3; i++)
    {
        m_cFile.read((char*)(&fCoord), sizeof(float));
        point.push_back(fCoord);
    }
    return true;
}

void TrkFileReader::xResetPos()
{
    m_iPntPos = -1;
    m_iTrkPos = -1;
    m_iPntPosMax = -1;
}


size_t TrkFileReader::getTotalTrkNum()
{
    return m_cRandomAccessMap.size();
}

size_t TrkFileReader::getPointNumInTrk(int iIdx)
{
    map<int32_t,TrkInfo>::iterator it = m_cRandomAccessMap.find(iIdx);
    if( it ==  m_cRandomAccessMap.end() )
        return -1;
    return it->second.numPntsInTrk;
}


void TrkFileReader::checkFile()
{
    cerr << "Start Checking..." << endl;
    m_cFile.seekg(0);

    assert(sizeof(m_cHeader) == TRK_HEADER_SIZE);      ///< make sure header is 1000 bytes
    m_cFile.read((char*)(&m_cHeader), TRK_HEADER_SIZE);  ///< read header

    cerr << "n_count:" << m_cHeader.n_count << "\nn_scalars:" << m_cHeader.n_scalars << "\nn_properties:" << m_cHeader.n_properties << endl;

    m_iPntPos = 0;
    m_iTrkPos = 0;
    m_cFile.seekg(TRK_HEADER_SIZE);

    float points[3];

    m_iTrkPos = 0;
    while(!m_cFile.eof())
    {

        m_cFile.read((char*)(&m_iPntPosMax), sizeof(int32_t));
        cerr << "Track" << m_iTrkPos << ":" << m_iPntPosMax << "points" << endl;

        for(m_iPntPos = 0; m_iPntPos < m_iPntPosMax; ++m_iPntPos)
        {
            m_cFile.read((char*)points, 3*sizeof(float));
            streamsize lReadSize = m_cFile.gcount();
            if(lReadSize != 3*sizeof(float))
            {
                cerr << "Reading Fail for #:" << m_iPntPos << "Point" << endl;
                cerr << "Read Size is:" << lReadSize << endl;
                assert(lReadSize == 3*sizeof(float));
            }
            m_cFile.seekg(m_cHeader.n_scalars*sizeof(float), ios::cur);
        }
        m_cFile.seekg(m_cHeader.n_properties*sizeof(float), ios::cur);
        m_cFile.peek();
        ++m_iTrkPos;
    }
    cerr << "Total:" << m_iTrkPos << endl;
    cerr << "Checking Finished..." << endl;

    xResetPos();
}


TrkFileWriter::TrkFileWriter()
{

}


TrkFileWriter::TrkFileWriter(string &strFilepath)
{
    m_cFilepath = strFilepath;
}


TrkFileWriter::~TrkFileWriter()
{
    close();
}

bool TrkFileWriter::create()
{
    /// open trk file
    m_cFile.open(m_cFilepath.c_str(), ios::out|ios::binary);
    if( !m_cFile.is_open() )
    {
        cerr << "fail to open file" << endl;
        return false;
    }
    xWriteHeader();
    return true;
}

bool TrkFileWriter::appendTrack(vector<float> &points)
{
    /// check if 3d points
    if(points.size()%3 != 0)
    {
        cerr << "point number cannot divided by 3" << endl;
        return false;
    }
    /// append
    m_cFile.seekp(0, ios::end);

    /// write total point number in this track
    int32_t iTotalPoint = static_cast<int32_t>(points.size()/3);
    m_cFile.write((char*)&iTotalPoint, sizeof(int32_t));

    /// write the points

    m_cFile.write((char*)points.data(), points.size()*sizeof(float));

    /// fill extra scalars with 0
    float fZero = 0;
    for(int i = 0; i < m_cHeader.n_scalars; i++)
        m_cFile.write((char*)&fZero, sizeof(float));


    /// fill extra properties with 0
    for(int i = 0; i < m_cHeader.n_properties; i++)
        m_cFile.write((char*)&fZero, sizeof(float));

    return true;
}

void TrkFileWriter::save()
{
    xWriteHeader();
    return;
}

void TrkFileWriter::close()
{
    xWriteHeader();
    m_cFile.close();
}

void TrkFileWriter::copyHeader(const TrkFileHeader &other)
{
    memcpy(&m_cHeader, &other, TRK_HEADER_SIZE);
    m_cHeader.n_count = 0;
    m_cHeader.n_properties = 0; /// TODO for now it simply ignore n_properties & n_scalars
    m_cHeader.n_scalars = 0;
}


void TrkFileWriter::xWriteHeader()
{
    m_cFile.seekp(0);
    assert(sizeof(m_cHeader) == TRK_HEADER_SIZE);         ///< make sure header is 1000 bytes
    m_cFile.write((char*)(&m_cHeader), TRK_HEADER_SIZE);   ///< write header
}
