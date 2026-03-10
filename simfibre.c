#include <stdio.h>
#include <strings.h>
#include <gtk/gtk.h>
#include <gtk/gtkdrawingarea.h>
#include <stdint.h>
#include "fast_hsv2rgb.h"

#include "effects.h"

char * titlestr;




// Callback function to handle the "destroy" signal
void on_destroy(GtkWidget *widget, gpointer data){
  free(titlestr);
  gtk_main_quit();
}



GdkRGBA  rectcols[LEDNUMX*LEDNUMY];
GdkRectangle * rects[LEDNUMX*LEDNUMY];

extern volatile uint32_t t;

static inline void applyhue(){
  RGBcolor_t res;
  for(int i=0;i<LEDNUMX;i++){
    for(int j=0;j<LEDNUMY;j++){
      getRGB(i,j,&res);
      rectcols[i+LEDNUMX*j].red   =res.r/255.0;
      rectcols[i+LEDNUMX*j].green =res.g/255.0;
      rectcols[i+LEDNUMX*j].blue  =res.b/255.0;     
     }
   }
}


/***********************************************************************/






/**********************************************************************/







gboolean
draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data){
  guint width, height;
  GdkRGBA color;
  GtkStyleContext *context;
  tableupdate();
  applyhue();
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
    //xorshift32();
    for(int i=0;i<LEDNUMX;i++){
      for(int j=0;j<LEDNUMY;j++){
	rects[i+LEDNUMX*j] = (GdkRectangle*) malloc(sizeof(GdkRectangle));
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
