#include <stdio.h>
#include <strings.h>
#include <gtk/gtk.h>
#include <gtk/gtkdrawingarea.h>
#include <stdint.h>
#include "fast_hsv2rgb.h"
#include "maps.h"
char * titlestr;

#define NR_EFFECTPTR 16
typedef void (*effect_func_t)();
uint8_t justswitchedeffect = 1;

uint32_t mrand = 0xcafebabe;

static inline uint32_t xorshift32()
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  uint32_t x = mrand;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  mrand = x;
  return x;
}

// Callback function to handle the "destroy" signal
void on_destroy(GtkWidget *widget, gpointer data) {
  free(titlestr);
  gtk_main_quit();
}

#define LEDNUMX 12
#define LEDNUMY 12
#define LEDPIXSZ 48

GdkRGBA  rectcols[LEDNUMX*LEDNUMY];
uint16_t recthues[LEDNUMX*LEDNUMY];
uint16_t rectsat[LEDNUMX*LEDNUMY];
uint16_t rectval[LEDNUMX*LEDNUMY];
GdkRectangle * rects[LEDNUMX*LEDNUMY];

volatile uint32_t t = 0;

uint8_t or,og,ob;

void flipAltLines(){
   float tr,tg,tb;
   uint16_t th;
   for(int i=0;i<(LEDNUMX/2);i++){
     for(int j=0;j<LEDNUMY;j++){
       if((j&1)){
		 th                           = recthues[i+LEDNUMX*j];
		 recthues[i+LEDNUMX*j]        = recthues[LEDNUMX*(j+1)-(i+1)];
		 recthues[LEDNUMX*(j+1)-(i+1)]= th;
		 th                           = rectval[i+LEDNUMX*j];
		 rectval[i+LEDNUMX*j]         = rectval[LEDNUMX*(j+1)-(i+1)];
		 rectval[LEDNUMX*(j+1)-(i+1)] = th;
		 th                           = rectsat[i+LEDNUMX*j];
		 rectsat[i+LEDNUMX*j]         = rectsat[LEDNUMX*(j+1)-(i+1)];
		 rectsat[LEDNUMX*(j+1)-(i+1)] = th;
       }
     }
   }
}
/***********************************************************************/
uint8_t globalv = 255;
uint8_t globalsat = 255;

static inline void cpypix(unsigned int a,unsigned int b){
  recthues[a] = recthues[b];
  /*
   rectcols[a].red =  rectcols[b].red;
   rectcols[a].green =  rectcols[b].green;
   rectcols[a].blue =  rectcols[b].blue;
  */
}
static inline void applyglobalsatval(){
   for(int i=0;i<LEDNUMX;i++){
     for(int j=0;j<LEDNUMY;j++){
       rectsat[i+LEDNUMX*j]   = globalsat;
       rectval[i+LEDNUMX*j]   = globalv;
     }
   }
}

static inline void applyhue(uint8_t s,uint8_t v){
  uint8_t r,g,b;
  for(int i=0;i<LEDNUMX;i++){
    for(int j=0;j<LEDNUMY;j++){
      fast_hsv2rgb_32bit( recthues[i+LEDNUMX*j],
			  rectsat[i+LEDNUMX*j],
			  rectval[i+LEDNUMX*j],
			  &r,&g,&b);
      rectcols[i+LEDNUMX*j].red   =r/255.0;
      rectcols[i+LEDNUMX*j].green =g/255.0;
      rectcols[i+LEDNUMX*j].blue  =b/255.0;     
     }
   }
}

static inline void deltahueall(unsigned int d){
  for(int i=0;i<LEDNUMX;i++){
    for(int j=0;j<LEDNUMY;j++){
      recthues[i+LEDNUMX*j] = (recthues[i+LEDNUMX*j]+d);       
    }
  }
  for(int i=0;i<LEDNUMX;i++){
    for(int j=0;j<LEDNUMY;j++){
      if(recthues[i+LEDNUMX*j] > HSV_HUE_MAX){
		recthues[i+LEDNUMX*j]-=HSV_HUE_MAX;
	  }
    }
  }  
}

static inline void randallhues(){
   for(int i=0;i<LEDNUMX;i++){
     for(int j=0;j<LEDNUMY;j++){
       recthues[i+LEDNUMX*j] = xorshift32() & HSV_HUE_MAX;     
     }
   }
}

static inline void randallvals(){
   for(int i=0;i<LEDNUMX;i++){
     for(int j=0;j<LEDNUMY;j++){
       rectval[i+LEDNUMX*j] = xorshift32() & 0xff;     
     }
   }
}

static inline void randallsats(){
   for(int i=0;i<LEDNUMX;i++){
     for(int j=0;j<LEDNUMY;j++){
       rectsat[i+LEDNUMX*j] = xorshift32() & 0xff;     
     }
   }
}

static inline void randall(){
  for(int i=0;i<LEDNUMX;i++){
     for(int j=0;j<LEDNUMY;j++){
       uint32_t v = xorshift32();
       rectval[i+LEDNUMX*j] = v & 0xff;     
       rectsat[i+LEDNUMX*j] = (v>>4) & 0xff;    
       recthues[i+LEDNUMX*j] = (v>>8) & HSV_HUE_MAX;     
     }
   }
}

static inline void randblackPox(uint8_t prob){
  for(int i=1;i<(LEDNUMX-1);i++){
    for(int j=1;j<(LEDNUMY-1);j++){
       if((xorshift32()& 0xff) < prob){
		 rectval[i+LEDNUMX*j]=0;
	 	 rectval[i+LEDNUMX*j]=0;
		 rectval[(i-1)+LEDNUMX*j] >>=1;
		 rectval[(i+1)+LEDNUMX*j] >>=1;
		 rectval[i+LEDNUMX*(j+1)] >>=1;
		 rectval[i+LEDNUMX*(j-1)] >>=1;
		 rectval[(i-1)+LEDNUMX*(j-1)]=  (rectval[(i-1)+LEDNUMX*(j-1)]>>1) + (rectval[(i-1)+LEDNUMX*(j-1)]>>2) ;
		 rectval[(i+1)+LEDNUMX*(j+1)]=  (rectval[(i+1)+LEDNUMX*(j+1)]>>1) + (rectval[(i+1)+LEDNUMX*(j+1)]>>2) ;
		 rectval[(i-1)+LEDNUMX*(j+1)]=  (rectval[(i-1)+LEDNUMX*(j+1)]>>1) + (rectval[(i-1)+LEDNUMX*(j+1)]>>2) ;
		 rectval[(i+1)+LEDNUMX*(j-1)]=  (rectval[(i+1)+LEDNUMX*(j-1)]>>1) + (rectval[(i+1)+LEDNUMX*(j-1)]>>2) ;		 
       }
     }
   }
}

static inline void randwhitePox(uint8_t prob){
  for(int i=1;i<(LEDNUMX-1);i++){
    for(int j=1;j<(LEDNUMY-1);j++){
       if((xorshift32()& 0xff) < prob){
	     rectval[i+LEDNUMX*j]=0;
	 	 rectval[i+LEDNUMX*j]=255;
		 rectval[(i-1)+LEDNUMX*j] =196;
		 rectval[(i+1)+LEDNUMX*j] =196;
		 rectval[i+LEDNUMX*(j+1)] =196;
		 rectval[i+LEDNUMX*(j-1)] =196;
		 rectval[(i-1)+LEDNUMX*(j-1)]= 128;
		 rectval[(i+1)+LEDNUMX*(j+1)]= 128;
		 rectval[(i-1)+LEDNUMX*(j+1)]= 128;
		 rectval[(i+1)+LEDNUMX*(j-1)]= 128;
       }
     }
   }
}

/**********************************************************************/

uint8_t muli=20,mulj=60;
int8_t dir = 1;
uint32_t eff_rnk = 0;
uint32_t deltat = 4;

void effect1(){
  uint8_t r,g,b;

  uint16_t h;
  muli+=dir;
  mulj+=dir;

  if(muli==255){
    dir=-dir;
  }
  
  for(int i=0;i<LEDNUMX;i++){
    for(int j=0;j<LEDNUMY;j++){
      recthues[i+LEDNUMX*j] = (t+(i*-muli)+(j*-mulj)) % HSV_HUE_MAX;
      /*   fast_hsv2rgb_32bit((t+(i*-muli)+(j*-mulj))%1534,255,255,&r,&g,&b);    
      rectcols[i+LEDNUMX*j].red =r/255.0;
      rectcols[i+LEDNUMX*j].green =g/255.0;
      rectcols[i+LEDNUMX*j].blue =b/255.0;
      */
    }
  }
  justswitchedeffect = 0;
}

void effect2(){
   uint8_t r,g,b;
   recthues[(LEDNUMX*LEDNUMY)/2+LEDNUMX/2] = (t*2)% HSV_HUE_MAX;
   cpypix((LEDNUMX*LEDNUMY)/2+LEDNUMX/2-1          ,(LEDNUMX*LEDNUMY)/2+LEDNUMX/2 );
   cpypix((LEDNUMX*LEDNUMY)/2+LEDNUMX/2-LEDNUMX-1  ,(LEDNUMX*LEDNUMY)/2+LEDNUMX/2 );
   cpypix((LEDNUMX*LEDNUMY)/2+LEDNUMX/2-LEDNUMX    ,(LEDNUMX*LEDNUMY)/2+LEDNUMX/2 );
   for(int i=1;i<LEDNUMX/2;i++){
     for(int j=1;j<LEDNUMY/2;j++){
       cpypix( (i-1)+LEDNUMX*(j-1) , (i)+LEDNUMX*(j));
       cpypix( LEDNUMX*(j)-i , LEDNUMX*(j+1)-(i+1));
       cpypix( (LEDNUMX*LEDNUMY-1)-((i-1)+LEDNUMX*(j-1)) , (LEDNUMX*LEDNUMY-1)-((i)+LEDNUMX*(j)));
       cpypix( (LEDNUMX*LEDNUMY-1)-(LEDNUMX*(j)-i) , (LEDNUMX*LEDNUMY-1)-(LEDNUMX*(j+1)-(i+1)));
     }
   }
}

void effect3(){
  deltahueall(2);
}

void effect4(){
  if(justswitchedeffect){
    effect1();
  }
  deltahueall(2);
}

void effect5(){
  if(justswitchedeffect){
    randallhues();
  }
  deltahueall(2);
}

void effect6(){
  if(justswitchedeffect){
    randallhues();
    randallsats();
  }
  if(xorshift32() & 1){
    deltahueall(xorshift32() & 0b111);
  }  
}

void effect7(){
  if(justswitchedeffect){
    t=200;
    effect1();
    randblackPox(16);
  }
  
}
void effect8(){
   if(justswitchedeffect)
     randwhitePox(16);
   effect5();
}

#define MAX_TRACERS (1<<3)
uint8_t TracerNr =0;
// XY coords of tracers
uint8_t TracerPos[MAX_TRACERS][3];
// XY velocity vector
int8_t TracerVel2DVec[MAX_TRACERS][2];

void plotline(int8_t x0,int8_t y0,int8_t x1,int8_t y1,uint16_t h,uint8_t dofade){ // bresenham quad skibidi
  //printf("plot %d %d, %d %d\n",x0,y0,x1,y1);
  uint8_t it=0;
  uint8_t coords[10][2];
  int8_t dirX = (x0<x1)?1:-1;
  int8_t dirY = (y0<y1)?1:-1;  
  int8_t dx = ((x1-x0) < 0 ?  -(x1-x0) : (x1-x0)) ;
  int8_t dy = ((y1-y0) < 0 ?  (y1-y0) : -(y1-y0)) ;
  int16_t e  = dy+dx;
  int8_t e2 = 0;
  while(1){
    coords[it][0] = x0;
    coords[it][1] = y0;
    it++;
    e2 = 2*e;
    if(e2>=dy){
      if(x0==x1){
        break;
	  }
      e = e+dy;
      x0+=dirX;
      //  printf("%cx %d %d %d\n",dirX>0?'+':'-',e,dx,dy);
    }
    if(e2<=dx){
      if(y0==y1){
         break;
	  }
      e = e+dx;
      y0+= dirY;
      //printf("%cy %d %d %d\n",dirY>0?'+':'-',e,dx,dy);
    }
  }
  // separate recthues and rectval manipulation to be able to overlay effects independantly
  uint8_t v = 255/(it+1);
  for(int i = 0;i<it;i++){
    recthues[ coords[i][0] +  coords[i][1]*LEDNUMX] = h;
    if(dofade){
      rectval [ coords[i][0] +  coords[i][1]*LEDNUMX] = v*(i+1);
    }else{
      rectval [ coords[i][0] +  coords[i][1]*LEDNUMX] = 255;
    }
  }
}

uint16_t tracerhue;

static inline void init_tracer(uint32_t r){
  uint16_t v = (HSV_HUE_MAX >> 3) * ((r>>1) & 15);
  tracerhue=v;
  for(int i=0;i<LEDNUMX;i++){
    for(int j=0;j<LEDNUMY;j++){
      recthues[i+j*LEDNUMX] = v;
      rectsat[i+j*LEDNUMX] = 255;
      rectval[i+j*LEDNUMX] = 0;
    }
  }      
  TracerNr = (r>>3 & (MAX_TRACERS-1)) | 1;
  for(int i=0;i<TracerNr;i++){
    r = xorshift32();
    TracerPos[i][0] = (r>>i)   & 0b111;
    TracerPos[i][1] = (r>>i+1) & 0b111;
    TracerPos[i][2] = (r>>i+2) & 0xffff;
    rectval[TracerPos[i][0]+TracerPos[i][1]*LEDNUMX] = 255;
    TracerVel2DVec[i][0] = ((r>>i+2) & 0b0000011)|1;
    TracerVel2DVec[i][1] = ((r>>i+3) & 0b0000011)|1;
    if(r & 0b1000000){
      TracerVel2DVec[i][0] = -TracerVel2DVec[i][0];
      TracerVel2DVec[i][1] = -TracerVel2DVec[i][1];	
	}
  }
}

void effectTracer(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){
    init_tracer(r);
    for(int i=0;i<TracerNr;i++){
      TracerPos[i][2] = ((r) & 0xffff)%HSV_HUE_MAX;
    }     
  } 
  if((t%20)==0){
    for(int i=0;i<(LEDNUMX*LEDNUMY);i++){
      recthues[i] = rectval[i] = 0;
	}
    for(int i=0;i<TracerNr;i++){
      dx = TracerPos[i][0]+TracerVel2DVec[i][0];
      dy = TracerPos[i][1]+TracerVel2DVec[i][1];
      if(dx<0){
		dx=0;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dx>=LEDNUMX-1){
		dx=LEDNUMX-1;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dy<0){
		dy=0;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      if(dy>=LEDNUMY-1){
		dy=LEDNUMY-1;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      plotline(TracerPos[i][0],TracerPos[i][1],dx,dy,TracerPos[i][2],1);
      TracerPos[i][0]=dx;
      TracerPos[i][1]=dy;
    }
    tracerhue+=5;
    tracerhue %= HSV_HUE_MAX;
  }  
}

void effectTracer2(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){
    init_tracer(r);
    for(int i=0;i<TracerNr;i++){
      TracerPos[i][2] = ((r) & 0xffff)%HSV_HUE_MAX;
    }     
  } 
  if((t%20)==0){
    for(int i=0;i<(LEDNUMX*LEDNUMY);i++){
      recthues[i] = rectval[i] = 0;
	}
    for(int i=0;i<TracerNr;i++){
      dx = TracerPos[i][0]+TracerVel2DVec[i][0];
      dy = TracerPos[i][1]+TracerVel2DVec[i][1];
      if(dx<0){
		dx=0;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dx>=LEDNUMX-1){
		dx=LEDNUMX-1;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dy<0){
		dy=0;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      if(dy>=LEDNUMY-1){
		dy=LEDNUMY-1;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      plotline(TracerPos[i][0],TracerPos[i][1],dx,dy,tracerhue,1);
      TracerPos[i][0]=dx;
      TracerPos[i][1]=dy;
    }
    tracerhue+=5;
    tracerhue %= HSV_HUE_MAX;
  }  
}

void effectTracer3(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){
    init_tracer(r);
    for(int i=0;i<TracerNr;i++){
      TracerPos[i][2] = ((r) & 0xffff)%HSV_HUE_MAX;
      r = xorshift32();
    }     
  } 
  if((t%20)==0){
    for(int i=0;i<(LEDNUMX*LEDNUMY);i++){
      recthues[i] = rectval[i] = 0;
	}
    for(int i=0;i<TracerNr;i++){
      dx = TracerPos[i][0]+TracerVel2DVec[i][0];
      dy = TracerPos[i][1]+TracerVel2DVec[i][1];
      if(dx<0){
		dx=0;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dx>=LEDNUMX-1){
		dx=LEDNUMX-1;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dy<0){
		dy=0;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      if(dy>=LEDNUMY-1){
		dy=LEDNUMY-1;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      plotline(TracerPos[i][0],TracerPos[i][1],dx,dy,TracerPos[i][2] ,1);
      TracerPos[i][0]=dx;
      TracerPos[i][1]=dy;
    }
    tracerhue+=5;
    tracerhue %= HSV_HUE_MAX;
  }  
}

void effectTracer4(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){
    init_tracer(r);
    for(int i=0;i<TracerNr;i++){
      TracerPos[i][2] = ((r) & 0xffff)%HSV_HUE_MAX;
    }     
  } 
  if((t%20)==0){
    //for(int i=0;i<(LEDNUMX*LEDNUMY);i++)
    //  recthues[i] = rectval[i] = 0;
    for(int i=0;i<TracerNr;i++){
      dx = TracerPos[i][0]+TracerVel2DVec[i][0];
      dy = TracerPos[i][1]+TracerVel2DVec[i][1];
      if(dx<0){
		dx=0;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dx>=LEDNUMX-1){
		dx=LEDNUMX-1;
		TracerVel2DVec[i][0]=-TracerVel2DVec[i][0];
      }
      if(dy<0){
		dy=0;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      if(dy>=LEDNUMY-1){
		dy=LEDNUMY-1;
		TracerVel2DVec[i][1]=-TracerVel2DVec[i][1];
      }
      plotline(TracerPos[i][0],TracerPos[i][1],dx,dy,tracerhue,1);
      TracerPos[i][0]=dx;
      TracerPos[i][1]=dy;
    }
    tracerhue+=5;
    tracerhue %= HSV_HUE_MAX;
  }  
}
int8_t tracer5Dir = 1;
void effectTracer5(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){    
    init_tracer(r);
    tracer5Dir = 1;
    TracerNr = 4;
    TracerPos[0][0] = 5;
    TracerPos[0][1] = 5;
    TracerVel2DVec[0][0] =1;
    TracerVel2DVec[0][1] =0;
    TracerPos[1][0] = 6;
    TracerPos[1][1] = 5;
    TracerVel2DVec[1][0] =0;
    TracerVel2DVec[1][1] =1;
    TracerPos[2][0] = 6;
    TracerPos[2][1] = 6;
    TracerVel2DVec[2][0] =-1;
    TracerVel2DVec[2][1] =0;
    TracerPos[3][0] = 5;
    TracerPos[3][1] = 6;
    TracerVel2DVec[3][0] =0;
    TracerVel2DVec[3][1] =-1;      
  } 
  if((t%40)==0){
    for(int i=0;i<(LEDNUMX*LEDNUMY);i++){
      recthues[i] = rectval[i] = 0;
	}
    for(int i=0;i<TracerNr;i++){
      plotline(TracerPos[i][0],TracerPos[i][1],TracerPos[(i+1)%TracerNr][0],TracerPos[(i+1)%TracerNr][1],tracerhue,0);    
    }
    TracerPos[0][0]-=tracer5Dir;
    TracerPos[0][1]-=tracer5Dir;
    TracerPos[1][0]+=tracer5Dir;
    TracerPos[1][1]-=tracer5Dir;
    TracerPos[2][0]+=tracer5Dir;
    TracerPos[2][1]+=tracer5Dir;
    TracerPos[3][0]-=tracer5Dir;
    TracerPos[3][1]+=tracer5Dir;
    if((TracerPos[0][0] == 0) || (TracerPos[0][0] == ((LEDNUMX/2)-1))){
      tracer5Dir = -tracer5Dir;    
	}
    tracerhue+=5;
    tracerhue %= HSV_HUE_MAX;
  }  
}

int8_t tracer6Dir = 1;
void effectTracer6(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){
    init_tracer(r);
    tracer5Dir = 1;
    TracerNr = 4;
    TracerPos[0][0] = 5;
    TracerPos[0][1] = 5;
    TracerVel2DVec[0][0] =1;
    TracerVel2DVec[0][1] =0;
    TracerPos[1][0] = 6;
    TracerPos[1][1] = 5;
    TracerVel2DVec[1][0] =0;
    TracerVel2DVec[1][1] =1;
    TracerPos[3][0] = 6;
    TracerPos[3][1] = 6;
    TracerVel2DVec[2][0] =-1;
    TracerVel2DVec[2][1] =0;
    TracerPos[2][0] = 5;
    TracerPos[2][1] = 6;
    TracerVel2DVec[3][0] =0;
    TracerVel2DVec[3][1] =-1;      
  } 
  if((t%40)==0){
    for(int i=0;i<(LEDNUMX*LEDNUMY);i++){
      recthues[i] = rectval[i] = 0;
	}
    for(int i=0;i<TracerNr;i++){
      plotline(TracerPos[i][0],TracerPos[i][1],TracerPos[(i+1)%TracerNr][0],TracerPos[(i+1)%TracerNr][1],tracerhue,0);    
    }
    if(((TracerPos[0][0]) <= 0) || ((TracerPos[0][0]) > ((LEDNUMX/2)-1))){
      tracer5Dir = -tracer5Dir;    
	}
   
    TracerPos[0][0]-=tracer5Dir;
    TracerPos[0][1]-=tracer5Dir;
    TracerPos[1][0]+=tracer5Dir;
    TracerPos[1][1]-=tracer5Dir;
    TracerPos[3][0]+=tracer5Dir;
    TracerPos[3][1]+=tracer5Dir;
    TracerPos[2][0]-=tracer5Dir;
    TracerPos[2][1]+=tracer5Dir;

    tracerhue+=5;
    tracerhue %= HSV_HUE_MAX;
  }  
}

void effectTracer7(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){
    init_tracer(r);
    tracer5Dir = 1;
    TracerNr = 4;
    TracerPos[0][0] = 5;
    TracerPos[0][1] = 5;
    TracerVel2DVec[0][0] =1;
    TracerVel2DVec[0][1] =0;
    TracerPos[1][0] = 6;
    TracerPos[1][1] = 5;
    TracerVel2DVec[1][0] =0;
    TracerVel2DVec[1][1] =1;
    TracerPos[2][0] = 6;
    TracerPos[2][1] = 6;
    TracerVel2DVec[2][0] =-1;
    TracerVel2DVec[2][1] =0;
    TracerPos[3][0] = 5;
    TracerPos[3][1] = 6;
    TracerVel2DVec[3][0] =0;
    TracerVel2DVec[3][1] =-1;      
  } 
  if((t%40)==0){
    //printf("7 %d\n",TracerPos[0][0]);
    for(int i=0;i<(LEDNUMX*LEDNUMY);i++){
      recthues[i] = rectval[i] = 0;
	}
    for(int i=0;i<TracerNr;i++){
      plotline(TracerPos[i][0],TracerPos[i][1],TracerPos[(i+1)%TracerNr][0],TracerPos[(i+1)%TracerNr][1],tracerhue,0);    
    }
    plotline(TracerPos[0][0],TracerPos[0][1],TracerPos[2][0],TracerPos[2][1],tracerhue,0);
    plotline(TracerPos[1][0],TracerPos[1][1],TracerPos[3][0],TracerPos[3][1],tracerhue,0);
    if((TracerPos[0][0] <= 0) || (TracerPos[0][0] >= ((LEDNUMX/2))) ){
      tracer5Dir = -tracer5Dir;
	}
    TracerPos[0][0]-=tracer5Dir;
    TracerPos[0][1]-=tracer5Dir;
    TracerPos[1][0]+=tracer5Dir;
    TracerPos[1][1]-=tracer5Dir;
    TracerPos[2][0]+=tracer5Dir;
    TracerPos[2][1]+=tracer5Dir;
    TracerPos[3][0]-=tracer5Dir;
    TracerPos[3][1]+=tracer5Dir;
    
    tracerhue+=5;
    tracerhue %= HSV_HUE_MAX;
  }  
}

void effectTracer8(){
  uint32_t r = xorshift32();
  int8_t dx,dy;
  if(justswitchedeffect){
    init_tracer(r);
    tracer5Dir = 1;
    TracerNr = 4;
    TracerPos[0][0] = 5;
    TracerPos[0][1] = 5;
    TracerVel2DVec[0][0] =1;
    TracerVel2DVec[0][1] =0;
    TracerPos[1][0] = 6;
    TracerPos[1][1] = 5;
    TracerVel2DVec[1][0] =0;
    TracerVel2DVec[1][1] =1;
    TracerPos[2][0] = 6;
    TracerPos[2][1] = 6;
    TracerVel2DVec[2][0] =-1;
    TracerVel2DVec[2][1] =0;
    TracerPos[3][0] = 5;
    TracerPos[3][1] = 6;
    TracerVel2DVec[3][0] =0;
    TracerVel2DVec[3][1] =-1;      
  } 
  if((t%20)==0){
    for(int i=0;i<TracerNr;i++){
      plotline(TracerPos[i][0],TracerPos[i][1],TracerPos[(i+1)%TracerNr][0],TracerPos[(i+1)%TracerNr][1],tracerhue,0);    
    }
    plotline(TracerPos[0][0],TracerPos[0][1],TracerPos[2][0],TracerPos[2][1],tracerhue,0);
    plotline(TracerPos[1][0],TracerPos[1][1],TracerPos[3][0],TracerPos[3][1],tracerhue,0);
    if((TracerPos[0][0] <= 0) || (TracerPos[0][0] >= ((LEDNUMX/2))) ){
      tracer5Dir = -tracer5Dir;
	}
    TracerPos[0][0]-=tracer5Dir;
    TracerPos[0][1]-=tracer5Dir;
    TracerPos[1][0]+=tracer5Dir;
    TracerPos[1][1]-=tracer5Dir;
    TracerPos[2][0]+=tracer5Dir;
    TracerPos[2][1]+=tracer5Dir;
    TracerPos[3][0]-=tracer5Dir;
    TracerPos[3][1]+=tracer5Dir;    
    tracerhue+=55;
    tracerhue %= HSV_HUE_MAX;
  }  
}

effect_func_t effectTable[NR_EFFECTPTR] = {
  effectTracer8,
  effect8,
  effectTracer7,
  effect7,
  effectTracer6,
  effect6,
  effectTracer5,
  effect3,
  effectTracer4,
  effect4,
  effectTracer3,
  effect5,
  effectTracer2,
  effect2,
  effectTracer, 
  effect1,
};

void tableupdate(){
  t+=deltat;
  flipAltLines();
  if((t%(deltat*400))==0){
    justswitchedeffect = 1;
    applyglobalsatval();
    eff_rnk = (eff_rnk+1) % NR_EFFECTPTR;
    printf("Eff:%d\n",eff_rnk);
  }
  effectTable[eff_rnk]();
  justswitchedeffect = 0;
  flipAltLines();
  applyhue(globalsat,globalv);
}

gboolean
draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data){
  guint width, height;
  GdkRGBA color;
  GtkStyleContext *context;
  tableupdate();
  context = gtk_widget_get_style_context(widget);
  width = gtk_widget_get_allocated_width(widget);
  height = gtk_widget_get_allocated_height(widget);

  gtk_render_background(context, cr, 0, 0, width, height);

  for(int i=0;i<LEDNUMX;i++){
    for(int j=0;j<LEDNUMY;j++){
       gdk_cairo_set_source_rgba(cr, &rectcols[i+LEDNUMX*j]);
       gdk_cairo_rectangle(cr,rects[i+LEDNUMX*j]);
       cairo_fill(cr);
	}
  }

  return FALSE;
}

static gboolean on_tick(GtkWidget *widget){
    // Force a redraw of the widget
    gtk_widget_queue_draw(widget);
    return TRUE; // Return TRUE to keep the timeout active
}

int main(int argc, char *argv[]){
    // Initialize GTK
    gtk_init(&argc, &argv);
    //srand(time(NULL));
    //mrand=rand();
    xorshift32();
    for(int i=0;i<LEDNUMX;i++){
      for(int j=0;j<LEDNUMY;j++){
		rects[i+LEDNUMX*j] = malloc(sizeof(GdkRectangle));
		if((j&1) == 0){
			rects[i+LEDNUMX*j]->x = i*LEDPIXSZ;
		}
		else{
			rects[i+LEDNUMX*j]->x = (LEDNUMX*LEDPIXSZ)-(i+1)*LEDPIXSZ;
		}
		rects[i+LEDNUMX*j]->y = j*LEDPIXSZ;
		rects[i+LEDNUMX*j]->width = LEDPIXSZ;
		rects[i+LEDNUMX*j]->height = LEDPIXSZ;
		/*
		rectcols[i+LEDNUMX*j].red = ((i/(LEDNUMX*1.0)) + (j/(LEDNUMY*1.0))) /2.0 ;;
		rectcols[i+LEDNUMX*j].green = ((i/(LEDNUMX*1.0)) + (j/(LEDNUMY*1.0))) /2.0  ;;
		rectcols[i+LEDNUMX*j].blue = ((i/(LEDNUMX*1.0)) + (j/(LEDNUMY*1.0))) /2.0 ;;
		*/
		rectcols[i+LEDNUMX*j].alpha = 1.0;
      }
    }

    applyglobalsatval();

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Text Example");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    //gtk_widget_set_size_request(window, LEDNUMX*LEDPIXSZ ,LEDNUMY*LEDPIXSZ );

    // Create a label with some text
    titlestr = (char*) malloc (512);
    sprintf(titlestr,"%d x %d - %d",LEDNUMX*LEDPIXSZ ,LEDNUMY*LEDPIXSZ, LEDPIXSZ  );
    // GtkWidget *label = gtk_label_new("Hello, GTK!");

    GtkWidget *area = gtk_drawing_area_new ();
    gtk_widget_set_size_request(area,LEDNUMX*LEDPIXSZ ,LEDNUMY*LEDPIXSZ );
    g_signal_connect (G_OBJECT (area), "draw",
		      G_CALLBACK (draw_callback), NULL);

    // Add the label to the window
    //gtk_container_add(GTK_CONTAINER(window), label);
    
    gtk_container_add(GTK_CONTAINER(window), area);

    g_timeout_add(16, (GSourceFunc)on_tick, area);
    
    // Connect the "destroy" signal to the callback function
    g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);
 
    // Show all the widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}
