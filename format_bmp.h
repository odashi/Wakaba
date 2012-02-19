// format_bmp.h
#ifndef WAKABA_FORMAT_BMP_H
#define WAKABA_FORMAT_BMP_H

#include "basetype.h"

/* Windows/OS2 Bitmap File Format */

namespace Wakaba {
	namespace BMP {
		enum Compression {
			COMPRESSION_RGB = 0,
			COMPRESSION_RLE8 = 1,
			COMPRESSION_RLE4 = 2,
			COMPRESSION_BITFIELDS = 3,
			COMPRESSION_JPEG = 4,
			COMPRESSION_PNG = 5
		};

	#pragma pack(push, 2)
		struct BitmapFileHeader {
			UInt8 signature[2];
			UInt32 file_size;
			UInt8 reserved[4];
			UInt32 offset;
		};
	#pragma pack(pop)

		struct BitmapCoreHeader {
			UInt32 struct_size;
			UInt16 width;
			UInt16 height;
			UInt16 planes;
			UInt16 bit_count;
		};

		struct BitmapInfoHeader {
			UInt32 struct_size;
			Int32 width;
			Int32 height;
			UInt16 planes;
			UInt16 bit_count;
			UInt32 compression;
			UInt32 image_size;
			Int32 x_pixels_per_meter;
			Int32 y_pixels_per_meter;
			UInt32 used_color;
			UInt32 important_color;
		};

	#pragma pack(push, 1)
		struct RGBTriple {
			UInt8 b;
			UInt8 g;
			UInt8 r;
		};
	#pragma pack(pop)

		struct RGBQuad {
			UInt8 b;
			UInt8 g;
			UInt8 r;
			UInt8 reserved;
		};
	}
}

#endif
