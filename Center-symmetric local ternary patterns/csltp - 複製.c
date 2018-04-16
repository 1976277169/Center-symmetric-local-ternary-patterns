#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 

typedef  struct HEADER {
    unsigned char B;               // File size in bytes
	unsigned char M;
    unsigned short int Type;        // Magic indentifier
    unsigned int Size;              // File size in bytes
    unsigned short int Reserved1, Reserved2;
    unsigned int Offset;            // Offset to data (in B)
} Header;                           // -- 14 Bytes

typedef struct INFOHEADER {
    unsigned int Size;              // Header size in bytes
    int Width, Height;              // Width / height of image
    unsigned short int Planes;      // Number of colour planes
    unsigned short int Bits;        // Bits per pixel
    unsigned int Compression;       // Compression type
    unsigned int ImageSize;         // Image size in bytes
    int xResolution, yResolution;   // Pixels per meter
    unsigned int Colors;            // Number of colors
    unsigned int ImportantColors;   // Important colors
} InfoHeader;                       // -- 40 Bytes

typedef struct PIXEL {
    unsigned char Red;
	unsigned char Green;
	unsigned char Blue; // Intensity of Red, Green, and Blue	
}  __attribute__((packed,aligned(1))) pixel; ;                                  // -- 3 Bytes
 
 
 typedef struct  LBHEADER {
    unsigned short identifier;      // 0x0000
	// unsigned char B;               // File size in bytes
	// unsigned char M;
    unsigned int filesize;          // 0x0002
    unsigned int reserved;          // 0x0006
    unsigned int bitmap_dataoffset; // 0x000A
    unsigned int bitmap_headersize; // 0x000E
    unsigned int width;             // 0x0012
    unsigned int height;            // 0x0016
    unsigned short planes;          // 0x001A
    unsigned short bits_perpixel;   // 0x001C
    unsigned int compression;       // 0x001E
    unsigned int bitmap_datasize;   // 0x0022
    unsigned int hresolution;       // 0x0026
    unsigned int vresolution;       // 0x002A
    unsigned int usedcolors;        // 0x002E
    unsigned int importantcolors;   // 0x0032
    unsigned int palette;           // 0x0036
} __attribute__((packed,aligned(1))) lbheader;

// void RGBtoXYZ(float r, float g , float b , float *x , float *y,float *z); 
// void RGBtoYIQ(float r , float g , float b , float *y , float *i, float *q);
// void RGBtoHSV(float r , float g , float b , float *h , float *s , float *v);
// void XYZtoCIElab(float x , float y , float z , float *CIEL_l , float *CIEL_a ,float *CIEL_b );
 

float MIN( float a,float b,float c);
float MAX(float a, float b, float c);
pixel reaPixel(unsigned char *image_s , int width , int height , int dx , int x, int y , int bc  );

int Ostu(unsigned char *pixel_1, int width, int height);
void CSLTP(unsigned char *pixel_1, int width, int height);

 
int main(int argc, char const *argv[]) 
{   
	printf("argc=%d\n",argc);
	if  (argc!=4) 
	    { 
         printf("=========================================================\n");
	     printf(" exec.exe  a.bmp b.csv a.htm\n");
	     printf("=========================================================\n");
	     return  -1; 	
        } 	
		/*
		執行時 -> ./a.out CT52.bmp xx.csv  	 		
	    int i=0;		
	    for( i = 0; i < argc; i++ )
	       {
		    printf( "arg %d: %s <br>\n", i, argv[i] );		 
	     }
		 */   			
      //Out CSV File (2) ================================= 		
	  const char *outHTML = argv[3];	
	  printf("outHTML=%s\n",outHTML);   	
	  FILE *html;
	  html = fopen(outHTML, "w");	 
      if (!html)	{
		printf("The outHTML File Create have some Error , %s\n",outHTML);
	    return -1;	}
	    printf("outHTML Opening .... \n"); 		 
	//Out CSV File (2) ================================= 		
	  const char *outfilename = argv[2];	
	  printf("outfilename=%s\n",outfilename);   	
	  FILE *csv;
	  csv = fopen(outfilename, "w");	 
      if (!csv)	{
		printf("The CSV File Create have some Error , %s\n",outfilename);
	    return -1;	}
	    printf("outfilename Opening .... \n"); 
	  fprintf(csv,"CSLTP\n");	
    //		
	// BMP File (1) ===========================	  	
	   const char *filename = argv[1];	
	   printf("Image-filename=%s\n",filename); 
	   FILE *img;	
       img = fopen(filename, "rb");
	   if (!img) {
         printf("The BMP File not Found !! , %s\n",filename);
         return -1; }
       printf("BMP,Opening .... , %s\n",filename);   
	//=====Read BMP ============================================
	 unsigned int Dx;
	 unsigned int BMP_DataSize;
	 unsigned int BitCount;
	lbheader BMP;
	fread(&BMP, sizeof(unsigned char) ,sizeof(BMP), img);  
    //	=======================================================
	if (BMP.identifier != 0x4d42){
		printf("the bmp file not correct format = &s \n",filename);
		return -1;	}
	//	
	 printf("identifier=%x\n", BMP.identifier);
     printf("filesize=%d\n", BMP.filesize);
	 printf("reserved=%d\n", BMP.reserved);	 
	 printf("bitmap_dataoffset=%d\n", BMP.bitmap_dataoffset);
	 printf("bitmap_headersize=%d\n", BMP.bitmap_headersize);
	 printf("width=%d\n", BMP.width);
	 printf("height=%d\n", BMP.height );
	 printf("planes=%d\n", BMP.planes);	
	 printf("bits_perpixel=%d\n", BMP.bits_perpixel);//bits_perpixel
	 printf("compression=%d\n", BMP.compression);//compression
	 printf("bitmap_datasize=%d\n", BMP.bitmap_datasize);//bitmap_datasize
	 printf("hresolution=%d\n", BMP.hresolution);//hresolution
	 printf("vresolution=%d\n", BMP.vresolution);//vresolution
	 printf("usedcolors=%d\n", BMP.usedcolors);//usedcolors
	 printf("importantcolors=%d\n", BMP.importantcolors);//importantcolors
	 printf("palette=%d\n", BMP.palette);//palette	
     BMP_DataSize=BMP.bitmap_datasize ;
	 if (BMP.filesize- BMP.bitmap_datasize != BMP.bitmap_dataoffset){
		 printf("bitmap_dataoffset , Have some Error \n");
		 BMP_DataSize = BMP.filesize - BMP.bitmap_dataoffset ;
		 printf("New BMP_DataSize=%d \n",BMP_DataSize);
	 }
	 //------------------------------------------------------------------------
	 BitCount = BMP.bits_perpixel/8 ;
     Dx = (BMP_DataSize-(BMP.width*BMP.height*BitCount))/BMP.height; 
	 //-------------------------------------------------------------------------
	 printf("Dx=%d\n",Dx);	 //
         fprintf(html,"identifier=%x<br/>\n", BMP.identifier);
	 fprintf(html,"filesize=%d<br/>\n", BMP.filesize);
	 fprintf(html,"reserved=%d<br/>\n", BMP.reserved);	 
	 fprintf(html,"bitmap_dataoffset=%d<br/>\n", BMP.bitmap_dataoffset);
	 fprintf(html,"bitmap_headersize=%d<br/>\n", BMP.bitmap_headersize);
	 fprintf(html,"width=%d<br/>\n", BMP.width);
	 fprintf(html,"height=%d<br/>\n", BMP.height );
	 fprintf(html,"planes=%d<br/>\n", BMP.planes);	
	 fprintf(html,"bits_perpixel=%d<br/>\n", BMP.bits_perpixel);//bits_perpixel
	 fprintf(html,"compression=%d<br/>\n", BMP.compression);//compression
	 fprintf(html,"bitmap_datasize=%d<br/>\n", BMP_DataSize);//bitmap_datasize
	 fprintf(html,"hresolution=%d<br/>\n", BMP.hresolution);//hresolution
	 fprintf(html,"vresolution=%d<br/>\n", BMP.vresolution);//vresolution
	 fprintf(html,"usedcolors=%d<br/>\n", BMP.usedcolors);//usedcolors
	 fprintf(html,"importantcolors=%d<br/>\n", BMP.importantcolors);//importantcolors
	 fprintf(html,"palette=%d<br/>\n", BMP.palette);//palette	   
	 fprintf(html,"<br><br><a href=../../index.html>Home</a><br>\n");   // <a href="../../Welcome.html">Home</a><br>//	
	 fclose(html);
	//==============================================================================================
	// int myoffset ; 
	// myoffset = BMP.filesize - (BMP.width *  BMP.height * 3 ) ; 
    // fseek(img,myoffset,SEEK_SET); 	// fseek(檔案指標,位移量,起始量)		
	// 起始量 : SEEK_SET -> 從頭開始(0) , SEEK_CUR -> 目前位置(1) , SEEK_END ->檔案結尾 (2)	 
	//PIXEL	     	
	unsigned char *image_s = NULL;
	unsigned int  width, height;   // image width, image height
	fseek(img,BMP.bitmap_dataoffset,SEEK_SET); 	// fseek(檔案指標,位移量,起始量)   
	image_s = (unsigned char *)malloc((size_t)BMP_DataSize);	   //image_s = (unsigned char *)malloc((size_t)width  * height * 4);	
    if (image_s == NULL) {
        printf("malloc images_s error\n");
        return -1;}	
	//=====================實際運算(開始)================================	
	int x;
	int y;
	int offset;
	width = BMP.width ;
	height = BMP.height;
	
	unsigned char *AllPixel = (unsigned char*)malloc(height * width);
	
    fread(image_s, sizeof(unsigned char), (size_t)(long)BMP_DataSize, img);
	fclose(img);	   	  		   
	pixel aPIXEL;	  
	for (y = 0 ; y < height; y++ )		
	{
		for (x = 0; x < width; x++)			
		{   				 	
            aPIXEL = reaPixel(image_s,width,height,Dx,x,y,BitCount);	
			offset = y + width * x;
			AllPixel[offset] = (aPIXEL.Red*299+ 
								aPIXEL.Green*587+
								aPIXEL.Blue*114+500)/1000;
		}
	}
		
	CSLTP(AllPixel, width, height); //CSLTP
		
	for (y = 0 ; y < height; y++ )		
	{
		for (x = 0; x < width; x++)							 	
			fprintf(csv,"%f\n", (float)AllPixel[y + width * x]);
	} 
	// ====================實際運算(結束)===========================================	
    free(AllPixel);	
	fclose(csv);
    return 1;
}


pixel reaPixel(unsigned char *image_s , int width , int height , int dx , int x, int y  , int bc)
{
	 pixel a;
	 int xx;
	 int yy; 				
         yy= height - y-1;				
	     xx=x;		         
		 a.Red = *(image_s + ((bc * (width * yy + xx)) + 2)+ ( yy*dx));
         a.Green = *(image_s + ((bc * (width * yy + xx)) + 1)+ ( yy*dx));
         a.Blue = *(image_s + ((bc* (width * yy + xx)) + 0)+ ( yy*dx));		  
	 return a;
}

void CSLTP(unsigned char *pixel_1, int width, int height)
{
  int x, y;
  int mask_csltp[4];
  int mask_temp[4];
  int counter1 = 0, counter2 = -1, counter3 = 0;
  int thres = Ostu(pixel_1, width, height);
  int threshold = (thres/4);

  /* Create a temp 2-D array (column-Pointer) to calculate CSLTP */
  unsigned char **temp_image = (unsigned char**)calloc((height + 2) , sizeof(unsigned char*));
  /* Create a temp 2-D array (each-row-data) to calculate CSLTP */
  for(x = 0; x < height+2; x++)
    temp_image[x] = (unsigned char*)calloc((width + 2) , sizeof(unsigned char));


  /* Create a 2-D array to store each CSLTP 3x3 mask result */
  unsigned char **csltp_data= (unsigned char**)calloc(height , sizeof(unsigned char*));
  for(x = 0; x < height; x++)
    csltp_data[x] = (unsigned char*)calloc(width , sizeof(unsigned char));

  /* Import image to temp array */
  for(x = 1; x < height+1; x++)
  {
    for(y = 1; y < width+1; y++)
    {
      counter1 += 1;
      temp_image[x][y] = pixel_1[54+(counter1-1)*3];
    }
  }

/* CSLTP calculating area, still working. */
  for( x = 1 ; x < height+1; x++)
  {
    for( y = 1; y < width+1; y++ )
    {
      // 3x3 mask do convolution in here
      int temp = 0;
      mask_csltp[0] = temp_image[x][y+1] - temp_image[x][y-1];
      mask_csltp[1] = temp_image[x+1][y+1] - temp_image[x-1][y-1];
      mask_csltp[2] = temp_image[x+1][y] - temp_image[x-1][y];
      mask_csltp[3] = temp_image[x+1][y-1] - temp_image[x-1][y+1];
      for(counter2 = 0; counter2 < 4; counter2++)
      {
        if(mask_csltp[counter2] >= threshold)
          mask_temp[counter2] = 2;
        else if( abs(mask_csltp[counter2]) < threshold)
          mask_temp[counter2] = 1;
        else if(mask_csltp[counter2] <= -threshold)
          mask_temp[counter2] = 0;
      }
      for(counter2= 0; counter2 < 4; counter2++)
      {
        temp += mask_temp[counter2] * pow(3,counter2);
      }
      csltp_data[x-1][y-1] = temp;
    }
  }

  //After CSLTP calculate then write to original input data
  for(x = 0; x < height; x++)
  {
    for(y = 0; y < width; y++)
    {
      counter3 += 1;

      pixel_1[54+(counter3-1)*3] = csltp_data[x][y];
      pixel_1[55+(counter3-1)*3] = csltp_data[x][y];
      pixel_1[56+(counter3-1)*3] = csltp_data[x][y];
    }
  }

  for(x = 0; x < height+2; x++)
    free(temp_image[x]);
  for(x = 0; x < height; x++)
    free(csltp_data[x]);

  free(temp_image);
  free(csltp_data);
}

int Ostu(unsigned char *pixel_1, int width, int height)
{
	int pixel, graylevel, graylevel_a;
	int threshold;
	double p1, p2, u1, u2, max_ostu = 0, ostu_temp;
	//**************************************************************************
	int *histogram = (int*)malloc(256 * sizeof(int));
	for(graylevel = 0; graylevel < 256; graylevel++)
     histogram[graylevel] = 0;
	//**************************************************************************
	double *histogram_p = (double*)malloc(256 * sizeof(double));
	for(graylevel = 0; graylevel < 256; graylevel++)
     histogram_p[graylevel] = 0;
	//**************************************************************************
	
	for(pixel = 0; pixel < width*height; pixel += 3)
	{
		for(graylevel = 0; graylevel < 256; graylevel++)
		{
			if(pixel_1[pixel] == graylevel)
				histogram[graylevel] ++;
		}
	}
	
	for(graylevel = 0; graylevel < 256; graylevel++)
	{
		histogram_p[graylevel] = (double)histogram[graylevel] / (width*height);
	}
	
	for(graylevel = 0; graylevel < 256; graylevel++)
	{
		//****************************************************************************p1 & p2 
		p1 = p2 = 0;
		for(graylevel_a = 0; graylevel_a < graylevel+1; graylevel_a++)
		{
			p1 += histogram_p[graylevel_a];
		}
		p2 = 1 - p1;
		//****************************************************************************u1 
		u1 = u2 = 0;
		for(graylevel_a = 0; graylevel_a < graylevel+1; graylevel_a++)
		{
			u1 += (graylevel_a * histogram_p[graylevel_a]);
		}
		u1 = u1 / p1;
		//****************************************************************************u2
		for(graylevel_a = graylevel+1; graylevel_a < 256; graylevel_a++)
		{
			u2 += (graylevel_a * histogram_p[graylevel_a]);
		}  
		u2 = u2 / p2;
		//****************************************************************************
		ostu_temp = p1 * p2 * pow(u1 - u2, 2);
		
		if(ostu_temp >= max_ostu)
		{
			max_ostu = ostu_temp;
			threshold = graylevel;
		}
	}
	
	free(histogram);
	free(histogram_p);
	
	return threshold;
}
