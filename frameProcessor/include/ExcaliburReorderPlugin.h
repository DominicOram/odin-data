/*
 * ExcaliburReorderPlugin.h
 *
 *  Created on: 6 Jun 2016
 *      Author: gnx91527
 */

#ifndef TOOLS_FILEWRITER_EXCALIBURREORDERPLUGIN_H_
#define TOOLS_FILEWRITER_EXCALIBURREORDERPLUGIN_H_

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;


#include "FileWriterPlugin.h"
#include "ExcaliburDefinitions.h"
#include "ClassLoader.h"

#define FEM_PIXELS_PER_CHIP_X 256
#define FEM_PIXELS_PER_CHIP_Y 256
#define FEM_CHIPS_PER_BLOCK_X 4
#define FEM_BLOCKS_PER_STRIPE_X 2
#define FEM_CHIPS_PER_STRIPE_X 8
#define FEM_CHIPS_PER_STRIPE_Y 1
#define FEM_STRIPES_PER_MODULE 2
#define FEM_STRIPES_PER_IMAGE 6
#define FEM_CHIP_GAP_PIXELS_X 3
#define FEM_CHIP_GAP_PIXELS_Y_LARGE 125
#define FEM_CHIP_GAP_PIXELS_Y_SMALL 3
#define FEM_PIXELS_PER_STRIPE_X ((FEM_PIXELS_PER_CHIP_X+FEM_CHIP_GAP_PIXELS_X)*FEM_CHIPS_PER_STRIPE_X-FEM_CHIP_GAP_PIXELS_X)
#define FEM_TOTAL_PIXELS_Y (FEM_PIXELS_PER_CHIP_Y*FEM_CHIPS_PER_STRIPE_Y*FEM_STRIPES_PER_IMAGE +\
                (FEM_STRIPES_PER_IMAGE/2-1)*FEM_CHIP_GAP_PIXELS_Y_LARGE +\
                (FEM_STRIPES_PER_IMAGE/2)*FEM_CHIP_GAP_PIXELS_Y_SMALL)
// #define FEM_TOTAL_PIXELS_X FEM_PIXELS_PER_STRIPE_X
#define FEM_TOTAL_PIXELS_X (FEM_PIXELS_PER_CHIP_X*FEM_CHIPS_PER_STRIPE_X)
#define FEM_TOTAL_PIXELS (FEM_TOTAL_PIXELS_X * FEM_PIXELS_PER_CHIP_Y)

#define FEM_PIXELS_IN_GROUP_6BIT 4
#define FEM_PIXELS_IN_GROUP_12BIT 4
#define FEM_PIXELS_PER_WORD_PAIR_1BIT 12
#define FEM_SUPERCOLUMNS_PER_CHIP 8
#define FEM_PIXELS_PER_SUPERCOLUMN_X (FEM_PIXELS_PER_CHIP_X / FEM_SUPERCOLUMNS_PER_CHIP)
#define FEM_SUPERCOLUMNS_PER_BLOCK_X (FEM_SUPERCOLUMNS_PER_CHIP * FEM_CHIPS_PER_BLOCK_X)

namespace filewriter
{

  /** Processing of Excalibur Frame objects.
   *
   * The ExcaliburReorderPlugin class is currently responsible for receiving a raw data
   * Frame object and reordering the data into valid Excalibur frames according to the selected
   * bit depth.
   */
  class ExcaliburReorderPlugin : public FileWriterPlugin
  {
  public:
    ExcaliburReorderPlugin();
    virtual ~ExcaliburReorderPlugin();
    void configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply);
    void status(OdinData::IpcMessage& status);

  private:
    /** Configuration constant for asic counter depth **/
    static const std::string CONFIG_ASIC_COUNTER_DEPTH;
    /** Configuration constant for image width **/
    static const std::string CONFIG_IMAGE_WIDTH;
    /** Configuration constant for image height **/
    static const std::string CONFIG_IMAGE_HEIGHT;
    /** Configuration constant for reset of 24bit image counter **/
    static const std::string CONFIG_RESET_24_BIT;

    /** Configuration constant for 1 bit asic counter depth */
    static const int DEPTH_1_BIT  = 0;
    /** Configuration constant for 6 bit asic counter depth */
    static const int DEPTH_6_BIT  = 1;
    /** Configuration constant for 12 bit asic counter depth */
    static const int DEPTH_12_BIT = 2;
    /** Configuration constant for 24 bit asic counter depth */
    static const int DEPTH_24_BIT = 3;
    /** Configuration string representations for the bit depths */
    static const std::string BIT_DEPTH[4];

    void processFrame(boost::shared_ptr<Frame> frame);
    void reorder1BitImage(unsigned int* in, unsigned char* out);
    void reorder6BitImage(unsigned char* in, unsigned char* out);
    void reorder12BitImage(unsigned short* in, unsigned short* out);
    void build24BitImage(unsigned short* inC0, unsigned short* inC1, unsigned int* out);

    /** Pointer to logger **/
    LoggerPtr logger_;
    /** Bit depth of the incoming frames **/
    int gAsicCounterDepth_;
    /** Image width **/
    int imageWidth_;
    /** Image height **/
    int imageHeight_;
    /** Counter used for construction of 24 bit frames **/
    int framesReceived_;
  };

  /**
   * Registration of this plugin through the ClassLoader.  This macro
   * registers the class without needing to worry about name mangling
   */
  REGISTER(FileWriterPlugin, ExcaliburReorderPlugin, "ExcaliburReorderPlugin");

} /* namespace filewriter */

#endif /* TOOLS_FILEWRITER_EXCALIBURREORDERPLUGIN_H_ */
