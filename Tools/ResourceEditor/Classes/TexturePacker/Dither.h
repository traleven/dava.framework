//
//  Dither.h
//  ResourcePacker
//
//  Created by Yury Benesh on 12/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef ResourcePacker_Dither_h
#define ResourcePacker_Dither_h

class PixelReplacer 
{
public:
    class PixelReplace 
	{
	public:
        int rgba[4];
        int x;
        int y;
        
        //////////////////////////
        // order
        // (x-1,y+1) = 3/16 , (x,y+1) = 5/16, (x+1,y+1) = 1/16, (x+1, y)=7/16
        int index;
        
        void calcNew(int rgbaDiff[]) 
		{            
			static int coeffs[4] = {3, 5, 1, 7};
            for (int i = 0; i < 4; ++i) {
                rgba[i] = rgba[i] + rgbaDiff[i] * coeffs[index] / 16;
                if (rgba[i] > 255)
                    rgba[i] = 255;
                else if (rgba[i] < 0)
                    rgba[i] = 0;
            }            
        }
    };
    
    // (x-1,y+1), (x,y+1), (x+1,y+1), (x+1, y)
	PixelReplace pixels[4];
   	int pixelsToReplace;
    
    void reset()
	{
        pixelsToReplace = 0;
    }
    
    PixelReplace & getNext() 
	{
        PixelReplace & pr = pixels[pixelsToReplace];
        pr.index = pixelsToReplace;
        pixelsToReplace++;
        return pr;
    }
};


class PixelWriter 
{
public:
	enum PixelType
	{
		PIXEL_TYPE_4444 = 1,
		PIXEL_TYPE_565,
		PIXEL_TYPE_5551,
	};
    
	unsigned char * img;
    PixelType	targetPf;
	int imgWidth;
	
    PixelWriter(unsigned char * pixel_cache, PixelType targetPf, int width) 
	{
        this->img = pixel_cache;
        this->targetPf = targetPf;
		this->imgWidth = width;
    }
    
    int downGrade(int a, int targetBitCount) 
	{
        int maxv = ((1 << targetBitCount)-1);
        // a / 255 * maxv / maxv * 255
        return a * maxv / 255 * 255 / maxv ;
    }
    
    int downGradeComponent(int a, int cNum) 
	{
        switch (targetPf) {
            case PIXEL_TYPE_4444:
                return downGrade(a, 4);
                
            case PIXEL_TYPE_565:
                if (cNum == 1)
                    return downGrade(a, 6);
                if ((cNum == 0) || (cNum == 2))
                    return downGrade(a, 5);
                
                break;
                
            case PIXEL_TYPE_5551:
                if ((cNum >= 0) && (cNum <= 2))
                    return downGrade(a, 5);
                
                if (cNum == 3)
                    return downGrade(a, 1);
                
                break;
                
        }
        /// default for 4444
        //return a * 15 / 255 * 255 / 15;
        return 255;
    }
    
    void findClosest(int rgba[], int destRGBA[]) 
	{
        for (int i = 0; i < 4; ++i) 
		{
            destRGBA[i] = downGradeComponent(rgba[i], i);
        }
    }
    
    unsigned char elements[4];
    int pixel[4];
    
    void putPixel(int x, int y, int rgba[]) 
	{
		int pixelCoord = y * imgWidth *4 + x;
		img[pixelCoord] = rgba[0]; 
		img[pixelCoord+1] = rgba[1]; 
		img[pixelCoord+2] = rgba[2]; 
		img[pixelCoord+3] = rgba[3]; 
    }
    
    void getPixel(int x, int y, int rgba[4]) 
	{
		int pixelCoord = y * imgWidth *4 + x;
		rgba[0] = img[pixelCoord]; 
		rgba[1] = img[pixelCoord+1]; 
		rgba[2] = img[pixelCoord+2]; 
		rgba[3] = img[pixelCoord+3]; 
    }
};



#endif
