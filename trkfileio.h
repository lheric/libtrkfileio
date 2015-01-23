#ifndef TRKFILEIO_H
#define TRKFILEIO_H
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <stdint.h>
#include <assert.h>
#include "defs.h"
using namespace std;

static const int TRK_HEADER_SIZE = 1000;    ///< header size is 1000
#pragma pack(push)                          ///< push current alignment to stack
#pragma pack(1)                             ///< set alignment to 1 byte boundary
/**
 * @brief The TrkFileHeader class file header defined at http://www.trackvis.org/docs/?subsect=fileformat
 */
class TrkFileHeader
{
public:
    TrkFileHeader()
    {
        memset(this, 0, TRK_HEADER_SIZE);
        strcpy(id_string,"TRACK");
        strcpy(voxel_order,"LAS");
        version = 2;
        hdr_size = 1000;
    }
    char id_string[6];                          /// char	6	ID string for track file. The first 5 characters must be "TRACK".
    int16_t dim[3];                             /// short int	6	Dimension of the image volume.
    float voxel_size[3] ;                       /// float	12	Voxel size of the image volume.
    float origin[3];                            ///	float	12	Origin of the image volume. This field is not yet being used by TrackVis. That means the origin is always (0, 0, 0).
    int16_t n_scalars;                          ///	short int	2	Number of scalars saved at each track point (besides x, y and z coordinates).
    char scalar_name[10][20];                   ///	char	200	Name of each scalar. Can not be longer than 20 characters each. Can only store up to 10 names.
    int16_t n_properties;                       ///	short int	2	Number of properties saved at each track.
    char property_name[10][20];                 ///	char	200	Name of each property. Can not be longer than 20 characters each. Can only store up to 10 names.
    float vox_to_ras[4][4];                     ///	float	64	4x4 matrix for voxel to RAS (crs to xyz) transformation. If vox_to_ras[3][3] is 0, it means the matrix is not recorded. This field is added from version 2.
    char reserved[444];                         ///	char	444	Reserved space for future version.
    char voxel_order[4];                        ///	char	4	Storing order of the original image data. Explained here.
    char pad2[4];                               ///	char	4	Paddings.
    float image_orientation_patient[6];         ///	float	24	Image orientation of the original image. As defined in the DICOM header.
    char pad1[2];                               ///	char	2	Paddings.
    unsigned char invert_x;                     ///	unsigned char	1	Inversion/rotation flags used to generate this track file. For internal use only.
    unsigned char invert_y;                     ///	unsigned char	1	As above.
    unsigned char invert_z;                     ///	unsigned char	1	As above.
    unsigned char swap_xy;                      ///	unsigned char	1	As above.
    unsigned char swap_yz;                      ///	unsigned char	1	As above.
    unsigned char swap_zx;                      ///	unsigned char	1	As above.
    int32_t n_count;                            ///	int	4	Number of tracks stored in this track file. 0 means the number was NOT stored.
    int32_t version;                            ///	int	4	Version number. Current version is 2.
    int32_t hdr_size;                           ///	int	4	Size of the header. Used to determine byte swap. Should be 1000.
};
#pragma pack(pop)                               /// restore original alignment from stack


/**
 * @brief The TrkInfo struct Track offset, bytes and point numbers
 */
struct TrkInfo
{
    streamoff trackOffset;  ///< offset in file
    int16_t numPntsInTrk;   ///< points in this track
    int16_t lengthInByte;   ///< length in bytes for all points in this track
};

/**
 * @brief The TrkFileReader class track (*.trk) file writer
 */
class TrkFileReader
{
public:
    TrkFileReader();
    TrkFileReader(string& strFilepath);
    ~TrkFileReader();
    /**
     * @brief open Open the track file
     * @return
     */
    bool open();

    /**
     * @brief close Close the track file
     */
    void close();

    /**
     * @brief readTrack Read one track
     * @param iTrkIdx the track index (start from 0)
     * @param points
     * @return
     */
    bool readTrack(int iTrkIdx, vector<float>& points);

    /**
     * @brief readPoint Read one point in the track
     * @param iTrkIdx the track index (start from 0)
     * @param iPntIdx the point index (start from 0)
     * @param point
     * @return
     */
    bool readPoint(int iTrkIdx, int iPntIdx, vector<float>& point);

    /**
     * @brief getTotalTrkNum Get total number of tracks in this file
     * @return total number of tracks in this file
     */
    size_t getTotalTrkNum();

    /**
     * @brief getPointNumInTrk Get total number of points in the track
     * @param iIdx track index (start from 0)
     * @return total number of the points in this track
     */
    size_t getPointNumInTrk(int iIdx);

    /**
     * @brief checkFile For internal use, check is the file is valid
     */
    void checkFile();
protected:
    void xInit();
    void xResetPos();
    ADD_CLASS_FIELD_PRIVATE(fstream , cFile)                        ///< track file stream
    ADD_CLASS_FIELD(string, cFilepath, getFilepath, setFilepath)    ///< track file path
    ADD_CLASS_FIELD_NOSETTER(TrkFileHeader, cHeader, getHeader)     ///< track file header
    ADD_CLASS_FIELD(int32_t, iTrkPos, getTrkPos, setTrkPos)         ///< current track index
    ADD_CLASS_FIELD(int32_t, iPntPos, getPntPos, setPntPos)         ///< current point index
    ADD_CLASS_FIELD_NOSETTER(int32_t, iPntPosMax, getPntPosMax)     ///< total point number in current track
    ADD_CLASS_FIELD_NOSETTER(CONCATE(map<int32_t,TrkInfo>), cRandomAccessMap, getRandomAccessMap)    /// track (index to offset) map for random access support
};


/**
 * @brief The TrkFileWriter class track (*.trk) file reader
 */
class TrkFileWriter
{
public:
    TrkFileWriter();
    TrkFileWriter(string &strFilepath);
    ~TrkFileWriter();

    /**
     * @brief create Create a new empty trk file for write. **MUST BE EMPTY**
     * @return
     */
    bool create();

    /**
     * @brief copyHeader Copy a header form the other file
     * @param other
     */
    void copyHeader(const TrkFileHeader& other);

    /**
     * @brief appendTrack Append a track to the end of the file
     * @param points
     * @return
     */
    bool appendTrack(vector<float>& points);

    /**
     * @brief save Save the file immediately
     */
    void save();

    /**
     * @brief close Close the file
     */
    void close();

protected:
    void xWriteHeader();

    ADD_CLASS_FIELD_PRIVATE(fstream , cFile)    ///< track file stream
    ADD_CLASS_FIELD_NOSETTER(TrkFileHeader, cHeader, getHeader)     ///< track file header
    ADD_CLASS_FIELD(string, cFilepath, getFilepath, setFilepath)    ///< track file path
};

#endif // TRKFILEIO_H
