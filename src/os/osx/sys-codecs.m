/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**	Copyright 2021 Oldes
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Title: osx native codecs functionality
**  Author: Oldes
**  Purpose: To load and save images using system API
**	Related: core/n-image.c
**  Note: So far saves just one frame. For multi-image formats it would be
**        better to write a device (image:// port or something like that)
**
************************************************************************
**  Useful links:
**  https://chromium.googlesource.com/webm/webp-wic-codec
***********************************************************************/

#include "sys-codecs.h"
#include "sys-utils.h"


void listImageCodecs(void){
	//puts("Suppported codecs:");
	CFArrayRef mySourceTypes = CGImageSourceCopyTypeIdentifiers();
	CFShow(mySourceTypes);
	CFArrayRef myDestinationTypes = CGImageDestinationCopyTypeIdentifiers();
	CFShow(myDestinationTypes);
}

CFURLRef urlFromCString(const char* cString){
	NSString *myNSString = [NSString stringWithUTF8String:cString];
	NSString *path = [myNSString stringByExpandingTildeInPath];
	return CFURLCreateWithFileSystemPath(NULL, (CFStringRef)path, 0, false);
}

CODECS_API int DecodeImageFromFile(const char *uri, unsigned int frame, REBCDI *codi)
{
	int error = 0;
	CFURLRef url = NULL;
	CGImageRef img = NULL;
	CGImageSourceRef imgSrc = NULL;
	UInt32 *pixels;
	CGColorSpaceRef space;
	CGContextRef ctx;
	CFDataRef binSrc;
	
	NSUInteger w, h;
	NSUInteger bytesPerPixel = 4;
	NSUInteger bitsPerComponent = 8;
	do {
		if(uri) {
			// decoding from file
			url = urlFromCString(uri);
			imgSrc = CGImageSourceCreateWithURL(url, 0);
			ASSERT_NOT_NULL(imgSrc, 2, "read source file");
			// so far we are decoding only the first image of the source
		} else {
			// decoding from raw data
			binSrc = CFDataCreateWithBytesNoCopy(NULL, codi->data, codi->len, NULL);
			imgSrc = CGImageSourceCreateWithData(binSrc, 0);
		}
		img = CGImageSourceCreateImageAtIndex(imgSrc, 0, 0);
		ASSERT_NOT_NULL(img, 3, "create an image");
		w = CGImageGetWidth(img);
		h = CGImageGetHeight(img);
		pixels = (UInt32*)malloc(w * h * 4); // Rebol's library side must free it!
		ASSERT_NOT_NULL(pixels, 4, "allocate pixels buffer");
		space = CGColorSpaceCreateDeviceRGB();
		ctx = CGBitmapContextCreate(pixels, w, h, bitsPerComponent, bytesPerPixel * w, space, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
		ASSERT_NOT_NULL(ctx, 5, "create a bitmap context");
		CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), img);
		CGColorSpaceRelease(space);
		CGContextRelease(ctx);
		
		//listImageCodecs();
		codi->w = (UInt32)w;
		codi->h = (UInt32)h;
		codi->len = w * h * 4;
		codi->data = (unsigned char*)pixels;
	} while(FALSE);
	SAFE_CF_RELEASE(url);
	SAFE_CF_RELEASE(imgSrc);
	SAFE_CF_RELEASE(img);
	return error;
}

CODECS_API int EncodeImageToFile(const char *uri, REBCDI *codi)
{
	int result = 0;
	CFStringRef type;
	CFMutableDictionaryRef prop;
	
	switch (codi->type) {
	case CODI_IMG_PNG:  type = kUTTypePNG  ; break;     // Portable Network Graphics
	case CODI_IMG_JPEG: type = kUTTypeJPEG ; break;     // Joint Photographic Experts Group
	case CODI_IMG_JP2:  type = kUTTypeJPEG2000 ; break; // JPEG 2000
	case CODI_IMG_GIF:  type = kUTTypeGIF  ; break;     // Graphics Interchange Format
	case CODI_IMG_BMP:  type = kUTTypeBMP  ; break;     // Device independent bitmap
	case CODI_IMG_TIFF: type = kUTTypeTIFF ; break;     // Tagged Image File Format
	default:
			codi->error = 1;
			return codi->error;
	}
	
	CGDataProviderRef data = CGDataProviderCreateWithData(NULL, codi->bits, codi->w * codi->h * 4, NULL);
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGImageRef img = CGImageCreate(codi->w, codi->h, 8, 32, codi->w * 4, colorSpace, (kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big), data, NULL, TRUE, 0);
	CGDataProviderRelease(data);
	CGColorSpaceRelease(colorSpace);
	
	CFURLRef url;
	CGImageDestinationRef imgDst;
	CFDataRef dataDst;
	if(uri == NULL) {
		// writing into preallocated buffer (fixed size!)
		TRACE_PTR("dataTarget:", &codi->data);
		dataDst = CFDataCreateWithBytesNoCopy(NULL, codi->data, codi->len, NULL);
		imgDst = CGImageDestinationCreateWithData((CFMutableDataRef)dataDst, type, 1, 0);
	} else {
		// writing directly into file
		url = urlFromCString(uri);
		imgDst = CGImageDestinationCreateWithURL(url, type, 1, 0);
	}
	prop = CFDictionaryCreateMutable(NULL,0,NULL,NULL);
	CFDictionaryAddValue(prop, kCGImageDestinationLossyCompressionQuality, "0.2");
	CGFloat quality = 0.6;
	CGImageDestinationAddImage(imgDst, img, (__bridge CFDictionaryRef)@{(__bridge NSString *)kCGImageDestinationLossyCompressionQuality: @(quality)});
	CGImageDestinationFinalize(imgDst);
	
	if(uri == NULL) {
		TRACE_PTR("dataDst:", &dataDst);
		codi->len = CFDataGetLength(dataDst);
		//CFRelease(dataDst);
	}
	CFRelease(img);
	CFRelease(imgDst);
	codi->error = result;
	return result;
}
