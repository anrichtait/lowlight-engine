/**
  \file G3D-base.lib/include/G3D-base/ImageConvert.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_ImageConvert_H
#define G3D_ImageConvert_H

#include "G3D-base/PixelTransferBuffer.h"

namespace G3D {


/**
    Image conversion utility methods
*/
class ImageConvert {
private:
    ImageConvert();
    
    typedef shared_ptr<PixelTransferBuffer> (*ConvertFunc)(const shared_ptr<PixelTransferBuffer>& src, const ImageFormat* dstFormat);
    static ConvertFunc findConverter(const shared_ptr<PixelTransferBuffer>& src, const ImageFormat* dstFormat);
    
    // Converters
    static shared_ptr<PixelTransferBuffer> convertRGBAddAlpha(const shared_ptr<PixelTransferBuffer>& src, const ImageFormat* dstFormat);
    static shared_ptr<PixelTransferBuffer> convertRGBA8toBGRA8(const shared_ptr<PixelTransferBuffer>& src, const ImageFormat* dstFormat);
    static shared_ptr<PixelTransferBuffer> convertFloatToUnorm8(const shared_ptr<PixelTransferBuffer>& src, const ImageFormat* dstFormat);
    static shared_ptr<PixelTransferBuffer> convertUnorm8ToFloat(const shared_ptr<PixelTransferBuffer>& src, const ImageFormat* dstFormat);

public:
    /** Converts image buffer to another format if supported, otherwise returns null ref.
        If no conversion is necessary then \a src reference is returned and a new buffer is NOT created. */
    static shared_ptr<PixelTransferBuffer> convertBuffer(const shared_ptr<PixelTransferBuffer>& src, const ImageFormat* dstFormat);
};

} // namespace G3D

#endif // G3D_app_ImageConvert_H
