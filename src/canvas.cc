/*
  The graphics canvas is a very simple thing. It maintains a 'document' and
  updates the screen when necessary. Later on, I'll add some double buffering
  to avoid flicker. 

             Table of Contents
   
  Constructor                     uu-cons
  Destructor                      uu-des
  Detachment                      uu-deta
  Initialization (fonts, colors)  uu-obinit
  Text Handling                   uu-text
  Text Decorations                uu-decor
  Verbatim Text                   uu-ver
  Lists                           uu-list
  Tables                          uu-table
  Images                          uu-img
  Links                           uu-link
  Painting                        uu-paint
  Resizing                        uu-resize
*/

namespace FX
{

/* This struct holds the coordinates of a text element. We have an
   array of these, one for each element. */
typedef struct { int x,y, h,a; } coord_t;

/* An imagedef is a read-only struct describing the source for an image.
   'data' is PNG encoded. */
typedef struct { int width, height; const unsigned char *data; } imagedef_t;

/* An himage is the run-time generated struct corresponding to an image. */
typedef struct { int x,y1,y2; FXImage* image; } himage_t;


class FXAPI FYGfxCanvas : public FXScrollArea
{
  FXDECLARE(FYGfxCanvas)
protected:
  FYGfxCanvas();
private:
  FYGfxCanvas(const FYGfxCanvas&);
  FYGfxCanvas &operator=(const FYGfxCanvas&);

public:
  FYGfxCanvas(FXComposite* p, FXuint opts=FRAME_NORMAL,
              FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual ~FYGfxCanvas();

  virtual bool canFocus() const;
  virtual void detach();

  virtual int getContentWidth();
  virtual int getContentHeight();

/* Since we have a scrolling area, we need to keep track of the maximum
   x and y coordinates for the page. This lets the scroll area to properly
   display scroll bars. */

private: int max_x,max_y, min_x,min_y;

/***********************************************************************
 *                                                                     *
 *                           Initialization                            *
 *                                                                     *
 ***********************************************************************/
/* We can handle 256 fonts. Each font has some metrics:
   - A: ascent
   - H: height
   - S: width of space */
private:
   FXFont *fonts[256];
   int fontA[256], fontH[256],fontS[256];

/* We initialize the font structures using this thing. All used fonts
   must be defined using this function. 
     FIXME: maybe we can copy a font from another (the default?)
            if a used font isn't defined.  */
public:
   void define_font(int num,const char *family,int size,const char* style);

/* In order to keep track of whether we have created fonts, we use the
   following flag. The FOX system of creation and detachment aren't very
   clear to me, so I simply create the fonts when I need them. */

private: int fonts_created;
         void create_fonts();

/* We also support 256 colors for text. These are initialized with the
   below function. Since colors don't need to be created, we don't have a
   create function or a flag for it. */

private: FXColor colors[256];
public:  void define_color(int num, unsigned int C);

/***********************************************************************
 *                                                                     *
 *                            Text Handling                            *
 *                                                                     *
 ***********************************************************************/
// uu-text
/* The canvas will primarily hold text. The text is split up into three
   logical sections:

   - words       : int -> string mapping
   - elements    : a sequence of <style,word_index> 
   - coordinates : coordinates for each element

   Words and elements are read-only and are set when the controlling widget
   sets the document. Coordinates are created by this widget and is filled
   in when the controlling function tells the canvas to paint the elements
   in a certain way. 

   The word array is set for the whole help document. Pages do share the
   word array. The word_width array is used for caching the width of 
   each word. Note that a word is the combination of a string with a
   font. i.e. the word 'the' is represented twice in the words array
   if it occurs with the default font and the top header font. */

private: const char **words;
         unsigned short *word_width;
public:  void set_words(const char **wrd,int nw);

/* In order to lay out the text properly, we need some metrics: width, height
   and ascent. We always return the font height and ascent, in order to provide
   consistent line heights. This function is used to compute the contents
   of the word_width array. */
public:  int text_width(int f, const char *str);

/* Elements are set per page. The element array consists of <style,word>
   pairs. 

   Each style is an integer of the form: 
       U-B(8)-F(8)-T(8)
   U: underline bit
   B: background color index to colors[]
   F: foreground color index to colors[]
   T: font index to fonts[]

   The set_elts function also initializes the coords array. */

private: const unsigned int *elts;
         unsigned int nelts;
         coord_t *coords;
public:  void set_elts(const unsigned int *e, unsigned int ne);

/* When printing text, we need to keep track of the 'cursor' position.
   The canvas works something like a terminal. You 'print' stuff to it.
   The stuff you can print is already in the elts[] array but you need
   to tell it where a paragraph starts and ends. The following members
   keep where you left off last time.
      FIXME: cursor_x is in fact never accessed across method invocations.
             It could be a local variable for println() and stuff. */

private: int cursor_x, cursor_y;

/* Our main method for printing text will be println. This function prints
the elements elts[start,end] as a single paragraph, and then advances
cursor_y so that later printings can continue from below. */

public:  void println(int start, int end);
         void pbreak();

/***********************************************************************
 *                                                                     *
 *                          Text Decorations                           *
 *                                                                     *
 ***********************************************************************/
// uu-decor
/* We will also have some inline decorations. These are background
   rectangles, verbatim boxes and underlines. These decorations are
   represented as integer pairs, indicating the start and end element indices
   for the decoration. i.e. rect(red,3,5) means that we should draw a red
   rectangle covering the background of elements 3,4,5.

   The following members will hold the corresponding element indices. [0] is
   start, [1] is end and [2] is color for each triplet. */

private: const unsigned int *bgrects, *underlines,*vboxes;
         unsigned int n_bgrects, n_underlines,n_vboxes;

public:  void set_vboxes(const unsigned int *d, int nd);
         void set_underlines(const unsigned int *d, int nd);
         void set_bgrects(const unsigned int *d, int nd);

private: void draw_bgrects(FXDCWindow *dc,int se,int ee);
         void draw_underlines(FXDCWindow *dc,int se,int ee);
         void draw_vboxes(FXDCWindow *dc,int se,int ee);


private: int find_decor(int se, int ee, int *sD, int *eD, 
                        const unsigned int *decor,unsigned int ndecor);

/***********************************************************************
 *                                                                     *
 *                            Verbatim Text                            *
 *                                                                     *
 ***********************************************************************/
// uu-ver
/* Vertical and left external padding for verbatim text boxes. These
   are the paddding for the text itself, not the box. */
private: int vpad_verbatim, lpad_verbatim;

/* Just like println, but:
   - elements are complete strings, with spaces etc. inside them
   - we don't wrap around. */
public:  void verbatim_line(int start, int end);

/* Advances cursor_y to make space. */
public:  void verbatim_vspace();

// uu-list
/***********************************************************************
 *                                                                     *
 *                                Lists                                *
 *                                                                     *
 ***********************************************************************/
/* finds out which character is present in the normal font and uses
   that as the list bullet. */
private: void figure_out_bullet();

         int lpad_paragraph;  // left padding for a normal paragraph
                              // list code modifies this to fool println.
public:  void bullet_println(int level,int start, int end);

//uu-table
/***********************************************************************
 *                                                                     *
 *                               Tables                                *
 *                                                                     *
 ***********************************************************************/
/* For tables, we do the whole layout from outside. So, we need to access
   some metrics: */

public: void get_seq_metrics(int start,int end,int *Rw,int *Rh,int *Ra);

/* Print elts[s..e] justified. x1 and x2 are absolute, y is relative
   to cursor_y. */

        void cell_left(int x1,int x2,int y,int h,int a,int s, int e);
        void cell_center(int x1,int x2, int y,int h,int a,int s,int e);
        void cell_right(int x1,int x2,int y,int h,int a,int s, int e);

/* We also need to support table borders and internal lines. These are
   a little different from the other decorations in the sense that they
   are not bound to any element but have their own absolute coordinates.

   However, their number is a fixed value and the array can be allocated
   once.  I will implement the horizontal and vertical lines separately.

   Let's start with the horizontal ones. We just need to allocate the
   associated array once when we initially switch to a page and then
   do the layout on the same array over and over without reallocating
   it. We only reallocate it when the page changes.

   The horizontal lines will be composed of three integers each: y,
   x1 and x2. Vertical lines will be very similar with the
   triplet <y1,y2,x>. */

private: int *row_lines; int n_row_lines;
         int *col_lines; int n_col_lines;

/* The following functions allocate enough space for the given number
   of lines. They don't initialize anything. They just make sure enough
   space is allocated. */
public:  int *get_row_lines(int nr);
         int *get_col_lines(int nr);

/* Some access functions needed by the table stuff. */
public:  int get_width() { return viewport_w; }
         void advance_y(int u) { cursor_y += u; max_y= cursor_y; }
         int get_cursor_y() { return cursor_y; }
         void xcoord(int x) { if (x>max_x) max_x= x; } 

private: void draw_col_lines(int y1, int y2, FXDCWindow *dc);
         void draw_row_lines(int y1, int y2, FXDCWindow *dc);

/***********************************************************************
 *                                                                     *
 *                               Images                                *
 *                                                                     *
 ***********************************************************************/
// uu-img
/* Image definition array is set by the page content function using
   set_images method. The image(n) method then later puts the given
   image at the cursor_y position with horizontal centering. */

private: int n_images;
         imagedef_t *imagedefs;
         himage_t   *images;
public:  void set_images(imagedef_t *id, int n);
         void image(int n);
private: void draw_images(FXDCWindow *dc, int y1, int y2);

/* We need to destroy images when we change pages or destroy 
   the help widget.  */

private: void destroy_images();

/***********************************************************************
 *                                                                     *
 *                                Links                                *
 *                                                                     *
 ***********************************************************************/
// uu-link
/* Links are very similar to decorations. Their representation in this
   class is actually identical: an array of integer triplets 
   <start,end,target> */
private: const unsigned int *links; int n_links;
public:  void set_links(const unsigned int*, int);
         void (*link_func)(int);
         long onLeftBtnPress(FXObject*,FXSelector,void*);
         int find_element_by_coord(int x,int y);

/***********************************************************************
 *                                                                     *
 *                              Painting                               *
 *                                                                     *
 ***********************************************************************/
// uu-paint
public:  long onPaint(FXObject* ob,FXSelector sel,void* ptr);
         int find_lower(int scr);
         int find_higher(int scr);
         void draw_everything (FXDCWindow *dc, int evX,int evY,
                                               int evW,int evH);

/***********************************************************************
 *                                                                     *
 *                              Resizing                               *
 *                                                                     *
 ***********************************************************************/
//uu-resize
private: void (*resize_func)(FYGfxCanvas *);

/* We mark the beginning and end of the page with the following functions
   so that the widget knows when to update its scrollbars. */

public:  void reset_doc();
         void end_doc(void (*f)(FYGfxCanvas*));

         virtual void layout();
         long on_layout_timeout(FXObject* ob,FXSelector sel,void* ptr);

// uz-end
};
}

/***********************************************************************
 *                                                                     *
 *                            Boiler-plate                             *
 *                                                                     *
 ***********************************************************************/
using namespace FX;

static inline int min_i(int a,int b) { return a<b? a:b; }
static inline int max_i(int a,int b) { return a>b? a:b; }

FXDEFMAP(FYGfxCanvas) FYGfxCanvasMap[]=
{
  FXMAPFUNC(SEL_PAINT,0,FYGfxCanvas::onPaint),
  // uu-resize
  FXMAPFUNC(SEL_TIMEOUT, 0, FYGfxCanvas::on_layout_timeout),
  // uu-link
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0, FYGfxCanvas::onLeftBtnPress),
};

FXIMPLEMENT(FYGfxCanvas,FXScrollArea,FYGfxCanvasMap,
            ARRAYNUMBER(FYGfxCanvasMap))

FYGfxCanvas::FYGfxCanvas(){ flags|=FLAG_ENABLED|FLAG_SHOWN; }
bool FYGfxCanvas::canFocus() const { return true; }
int FYGfxCanvas::getContentHeight() { return max_y; }
int FYGfxCanvas::getContentWidth() { return max_x; }


/***********************************************************************
 *                                                                     *
 *                      Constructor / Destructor                       *
 *                                                                     *
 ***********************************************************************/

FYGfxCanvas::FYGfxCanvas
     (FXComposite* p, FXuint opts, FXint x,FXint y,FXint w,FXint h):
		FXScrollArea(p,opts | VSCROLLER_ALWAYS ,x,y,w,h)
{
  flags|=FLAG_ENABLED;

  fonts_created= 0;
  { int i; for(i=0;i<256;i++) fonts[i]= NULL; }

  coords= NULL; words= NULL; elts= NULL; nelts= 0;
  word_width= NULL;

  cursor_x= cursor_y=0;

  max_x= max_y= min_x= min_y= 0;

  images= NULL; imagedefs= NULL; n_images= 0;

  bgrects= underlines= vboxes= NULL;
  n_bgrects= n_underlines= n_vboxes= 0;

  vpad_verbatim= 5; lpad_verbatim= 15; 
  lpad_paragraph= 10;

  // uu-resize
  resize_func= NULL;

  // uu-table
  row_lines= NULL; n_row_lines=0;
  col_lines= NULL; n_col_lines=0;

// uu-cons
}


void FYGfxCanvas::detach()
{ 
  int i;

  FXScrollArea::detach();

  if (fonts_created)
     for(i=0;i<256;i++) 
        if (fonts[i]) fonts[i]->detach(); 
// uu-deta
}

FYGfxCanvas::~FYGfxCanvas() { 
  int i;

  for(i=0;i<256;i++) 
     if (fonts[i]) delete fonts[i];

  if (coords) { free(coords); coords= NULL; }

  destroy_images();

  // uu-table
  if (row_lines) free(row_lines);
  if (col_lines) free(col_lines);
// uu-des
}


/***********************************************************************
 *                                                                     *
 *                           Initialization                            *
 *                                                                     *
 ***********************************************************************/
// uu-obinit
void FYGfxCanvas::define_font
   (int num,const char *family,int size,const char* style)
{
   int weight;
   int slant;
   if (num>=256) return ;
   
   if (strchr(style,'B'))
      weight= FXFont::Bold;
   else
      weight= FXFont::Normal;
   if (strchr(style,'I'))
      slant= FXFont::Italic;
   else
      slant= FXFont::Straight;

   fonts[num]= new FXFont(FXApp::instance(),family, size, weight,slant);
}

void FYGfxCanvas::create_fonts()
{
  if (!fonts_created)
  {
    int i;
    fonts_created= 1;
    for(i=0;i<256;i++) 
      if (fonts[i])
      {
         fonts[i]->create();
         fontA[i]= fonts[i]->getFontAscent();
         fontH[i]= fonts[i]->getFontHeight();
         fontS[i]= fonts[i]->getTextWidth(" ", 1);
      }
    figure_out_bullet();
  }
}

void FYGfxCanvas::define_color(int num, unsigned int C)
{
   unsigned R,G,B;
   if (num>=256) return ;
   R= (C>>16u)&0xffu;
   G= (C>>8u)&0xffu;
   B= C&0xffu;
   colors[num]= FXRGB(R,G,B);
}

/***********************************************************************
 *                                                                     *
 *                            Text Handling                            *
 *                                                                     *
 ***********************************************************************/

// uu-text
void FYGfxCanvas::set_words(const char **wrd,int nw)
{
  words= wrd;
  if (word_width) free(word_width);
  word_width= (typeof(word_width)) calloc(nw, sizeof(word_width[0]));
}

void FYGfxCanvas::set_elts(const unsigned int *e, unsigned int ne)
{
  elts= e;
  nelts= ne;
  if (coords) free(coords);
  coords= (typeof(coords)) malloc(sizeof(*coords)*ne);
}

int FYGfxCanvas::text_width(int f, const char *str)
{
  create_fonts();
  if (f<0 || f>=256 || !fonts[f]) return 1;
  return fonts[f]->getTextWidth(str, strlen(str));
}

void FYGfxCanvas::println(int start, int end)
{
  int i,j,f,ww,w;
  int row_a, row_h;
  int canvas_w;

  int os= start;

  create_fonts();
  canvas_w= viewport_w-lpad_paragraph-10; //FIXME: we need rpad_paragraph also
again:
  cursor_x= lpad_paragraph;
  row_a= fontA[0];
  row_h= fontH[0];
  for(i=start;i<=end;i++)
  {
    f= elts[2*i]&0xff;
    w= elts[2*i+1];
    ww= word_width[w];
    if (ww==0) 
       word_width[w]= ww= text_width(f, words[w]);
    if (cursor_x+ww > canvas_w && i!=start) break;
    coords[i].x= cursor_x;
    cursor_x+= ww ;
    max_x= max_i(max_x, cursor_x);
    row_a= max_i(row_a, fontA[f]);
    row_h= max_i(row_h, fontH[f]);
    cursor_x+= fontS[f];
  }

  for(j=start;j<i;j++)
  {
    coords[j].y= cursor_y;
    coords[j].h= row_h;
    coords[j].a= row_a;
  }
  cursor_y += row_h + 2;
  max_y= cursor_y;

  if (i<=end) { start= i; goto again; }

  if (0)
  { printf("-----------------\n");
     for(i=os;i<=end;i++)
        printf("(x=%d,y=%d,a=%d,h=%d,w=%d)[%d:%s]\n", coords[i].x, 
                        coords[i].y, coords[i].a, coords[i].h,
                        word_width[elts[2*i+1]],
                        elts[2*i+1],
                        words[elts[2*i+1]]); }
}

void FYGfxCanvas::pbreak()
{
  create_fonts();
  cursor_y+= fontH[0];
  cursor_x= 0;
}


/***********************************************************************
 *                                                                     *
 *                          Text Decorations                           *
 *                                                                     *
 ***********************************************************************/
// uu-decor
void FYGfxCanvas::set_bgrects(const unsigned int *d, int nd)
{
  bgrects= d;
  n_bgrects= nd;
}

void FYGfxCanvas::set_underlines(const unsigned int *d, int nd)
{
  underlines= d;
  n_underlines= nd;
}

void FYGfxCanvas::set_vboxes(const unsigned int *d, int nd)
{
  vboxes= d;
  n_vboxes= nd;
}


/* Just like we find the elements visible within a window, we need to find
   decorations visible within a given element-window. */

int FYGfxCanvas::find_decor(int se, int ee, int *sD, int *eD, 
                const unsigned int *decor,unsigned int ndecor)
{
#if 1
  int i,j,k,S,E;
  i=0; j= ndecor-1;
  while(i<=j)
  {
    k= (i+j)/2;
    S= decor[k*3]; E= decor[k*3+1];
    if (E<se) { i= k+1; continue; }
    if (S>ee) { j= k-1; continue; }
    goto found;
  }
  return 1;
found:
  i=j= k;
  while(i>=0 && decor[i*3+1] >= se)    { *sD= i; i--; }
  while(j<=ndecor-1 && decor[j*3]<=ee) { *eD= j; j++;  }
  //printf("decor found for (%d %d) : (%d %d)\n", se, ee, *sD, *eD); 
  return 0;
#else

  int i,j,k;
  for(i=0;i<ndecor;i++)
    if ((decor[i*3+1]>=se && decor[i*3+1]<=ee)
        || (decor[i*3]>=se && decor[i*3]<=ee) )
        { *sD= *eD= i; break; }
  if (i==ndecor) return 1;

  for(i=ndecor-1;i>=0;i--)
    if ((decor[i*3+1]>=se && decor[i*3+1]<=ee)
        || (decor[i*3]>=se && decor[i*3]<=ee) )
        { *eD= i; break; }

  return 0;
#endif
}

void FYGfxCanvas::draw_bgrects(FXDCWindow *dc,int se,int ee)
{
  int sb, eb;
  if (!n_bgrects || !bgrects) return ;
  if (find_decor(se, ee, &sb, &eb, bgrects, n_bgrects)) return ;
  
  while(sb<=eb)
  {
    int S,E,C;
    S= bgrects[sb*3];
    E= bgrects[sb*3+1];
    C= bgrects[sb*3+2];

    int sx, ex;
    int sy,h;

    dc->setForeground(colors[C]);
    while(S<=E)
    {
      h= coords[S].h;
      sx= coords[S].x;
      sy= coords[S].y;
      while(S<=E && coords[S].y==sy) 
         { ex= coords[S].x+word_width[elts[2*S+1]]; S++; }
      dc->fillRectangle(sx+pos_x, sy+pos_y, ex-sx, h);
      //printf("RECT(%d,%d,%d,%d)\n", sx+pos_x, sy+pos_y, ex-sx, h);
    }
    sb++;
  }
}

void FYGfxCanvas::draw_underlines(FXDCWindow *dc,int se,int ee)
{
  int sb, eb;
  if (!n_underlines || !underlines) return ;
  if (find_decor(se, ee, &sb, &eb, underlines, n_underlines)) return ;
  
  while(sb<=eb)
  {
    int S,E,C;
    S= underlines[sb*3];
    E= underlines[sb*3+1];
    C= underlines[sb*3+2];

    int sx, ex;
    int sy,a;

    dc->setForeground(colors[C]);
    while(S<=E)
    {
      a= coords[S].a;
      sx= coords[S].x;
      sy= coords[S].y;
      while(S<=E && coords[S].y==sy) 
         { ex= coords[S].x+word_width[elts[2*S+1]]; S++; }
      dc->drawLine(sx+pos_x, sy+a+pos_y+2, ex+pos_x, sy+a+pos_y+2);
    }
    sb++;
  }
}

void FYGfxCanvas::draw_vboxes(FXDCWindow *dc,int se,int ee)
{
  int sb, eb;
  if (!n_vboxes || !vboxes) return ;
  if (find_decor(se, ee, &sb, &eb, vboxes, n_vboxes)) return ;

  // printf("found boxes: %d -> %d\n", sb, eb);
  
  while(sb<=eb)
  {
    int S,E,C;
    int i;
    int mx;
    S= vboxes[sb*3];
    E= vboxes[sb*3+1];
    C= vboxes[sb*3+2];

    dc->setForeground(colors[C]);

    mx= word_width[elts[2*S+1]] + coords[S].x;
    for(i=S+1;i<=E;i++)
       mx= max_i(mx,word_width[elts[2*i+1]] + coords[i].x); 

    int x1,x2,y1,y2;
    x1= lpad_paragraph;
    y1= coords[S].y-vpad_verbatim;
    x2= mx+ lpad_verbatim-lpad_paragraph;
    y2= coords[E].y+coords[E].h + vpad_verbatim;
    dc->fillRectangle(x1+pos_x, y1+pos_y, x2-x1, y2-y1);
    dc->setForeground(colors[0]);
    dc->drawRectangle(x1+pos_x, y1+pos_y, x2-x1, y2-y1);
    sb++;
  }
}

/***********************************************************************
 *                                                                     *
 *                            Verbatim Text                            *
 *                                                                     *
 ***********************************************************************/
// uu-ver
void FYGfxCanvas::verbatim_vspace()
{
  cursor_y+= vpad_verbatim;
  max_y= cursor_y;
}

/* we just keep on going, never wrapping around. */
void FYGfxCanvas::verbatim_line(int start, int end)
{
  int i,j,f,ww,w;
  int row_a, row_h;

  int os= start;

  create_fonts();

  cursor_x= lpad_verbatim;
  row_a= fontA[0];
  row_h= fontH[0];
  for(i=start;i<=end;i++)
  {
    f= elts[2*i]&0xff;
    w= elts[2*i+1];
    ww= word_width[w];
    if (ww==0) 
       word_width[w]= ww= text_width(f, words[w]);
    coords[i].x= cursor_x;
    cursor_x+= ww;
    max_x= max_i(max_x, cursor_x);
    row_a= max_i(row_a, fontA[f]);
    row_h= max_i(row_h, fontH[f]);
  }

  for(j=start;j<=end;j++)
  {
    coords[j].y= cursor_y;
    coords[j].h= row_h;
    coords[j].a= row_a;
  }
  cursor_y += row_h + 2;
  max_y= cursor_y;
}

/***********************************************************************
 *                                                                     *
 *                                Lists                                *
 *                                                                     *
 ***********************************************************************/
// uu-list
/* In order to display lists, we use 'bullet paragraphs'. What we do is:
    - make some space on the left for indentation
    - print a bullet character
    - adjust the paragraph start to point after the bullet character
    - call println()

   The bullet character is in fact a string, just like other text. This
   string is somewhat special, its value depends on the font. If the
   normal font doesn't contain the default bullet character, other
   characters should be tried.

   So, when we set up the fonts, I should try to get the width of the
   bullet string. Normally, I'd have the 'words' array as:

      const char **const words

   but this wouldn't let me modify words[0], which will be the bullet
   string.  So, I will keep it as:

      const char **words

   and then modify words[0] accordingly. */

void FYGfxCanvas::figure_out_bullet()
{
  int k,i;
  const char *blt[]= { "⚫","▪","★" ,"♦" ,"◆" ,"✸" ,"*", NULL};
  for(i=0;blt[i];i++)
  {
    FXString str= blt[i];
    if (fonts[0]->hasChar(str.wc(0)))
    {
       k= text_width(0, blt[i]);
       break;
    }
  }
  if (!blt[i]) { i--; k= 16; }
  words[0]= blt[i];
  word_width[0]= k;
}


/* The following function does the actual bullet paragraph printing.
   elts[2*start] should contain <normal,0>, which points to the bullet
   string. The rest is the actual content for the paragraph. */

void FYGfxCanvas::bullet_println(int level,int start, int end)
{
  int lp;
  unsigned int bw, bs;
  lp= lpad_paragraph;

  lpad_paragraph= level*30;  // FIXME: make it configurable
  bw= elts[2*start+1];
  bs= elts[2*start];

  if (word_width[bw]==0) 
     word_width[bw]= text_width(bs&0xff, words[bw]);

  coords[start].x= lpad_paragraph-word_width[bw] - 1;
  coords[start].y= cursor_y;
  coords[start].h= fontH[0]; 
  coords[start].a= fontA[0]; 

  println(start+1, end);
  if (coords[start+1].y==coords[start].y)
  {
   coords[start].h= max_i(coords[start].h , coords[start+1].h);
   coords[start].a= max_i(coords[start].a , coords[start+1].a);
  }

  lpad_paragraph= lp;
}

/* Note that using words[0] isn't mandatory at all. You can use whatever
   you want. Just put it in the elt[] array and this function will use
   the first word as the bullet. */

/***********************************************************************
 *                                                                     *
 *                               Tables                                *
 *                                                                     *
 ***********************************************************************/
// uu-table
/* When laying out table cells, we do the width / height computation
   of cells outside of canvas. We also compute the positions of the
   cells at the same place. Positioning the cell text inside the cell
   is done by the following functions.  _left means left justified,
   _right means right justified etc.

   The given y value is relative to cursor_y. It's not updated at this
   time.  However, the x values are absolute. The centering is also done
   outside since the canvas itself doesn't know anything about the size
   of the table. */

void FYGfxCanvas::cell_left(int x1,int x2,int y,int h,int a,int s, int e)
{
   int i;
   for(i=s;i<=e;i++)
   {
     if (i!=s)
       x1+= fontS[ elts[2*i] & 0xff ];
     coords[i].x= x1; 
     coords[i].y= cursor_y + y;
     coords[i].h= h;
     coords[i].a= a;
     x1+= word_width[ elts[2*i+1] ];
     max_x= max_i(max_x,x1);
   }
}

void FYGfxCanvas::cell_center(int x1,int x2, int y,int h,int a,int s,int e)
{
   int x,d,i;
   x= x1;
   for(i=s;i<=e;i++)
   {
     if (i!=s)
       x+= fontS[ elts[2*i] & 0xff ];
     coords[i].x= x; 
     coords[i].y= cursor_y + y;
     coords[i].h= h;
     coords[i].a= a;
     x+= word_width[ elts[2*i+1] ];
     max_x= max_i(max_x,x);
   }
   d= (x2-x)/2;
   for(i=s;i<=e;i++)
     coords[i].x+= d;
}

void FYGfxCanvas::cell_right(int x1,int x2,int y,int h,int a,int s, int e)
{
   int x,d,i;
   x= x1;
   for(i=s;i<=e;i++)
   {
     if (i!=s)
       x+= fontS[ elts[2*i] & 0xff ];
     coords[i].x= x; 
     coords[i].y= cursor_y + y;
     coords[i].h= h;
     coords[i].a= a;
     x+= word_width[ elts[2*i+1] ];
     max_x= max_i(max_x,x);
   }
   d= (x2-x);
   for(i=s;i<=e;i++) coords[i].x+= d;
}

/* This function is used for computing the width/height of a cell.
   It returns the dimensions of the given text elts[start,end] 
   as if it was displayed on a single row. */

void FYGfxCanvas::get_seq_metrics(int start,int end,int *Rw,int *Rh,int *Ra)
{
  int i;
  int W,S,F;
  int w,h,a;
  w= h= a= 0;

  create_fonts();
  for(i=start;i<=end;i++)
  {
     W= elts[2*i+1];
     S= elts[2*i];
     F= S & 0xff;
     if (i!=start) w+= fontS[F];
     if (word_width[W]==0) word_width[W]= text_width(F, words[W]);
     w+= word_width[W];
     h= max_i(h, fontH[F]);
     a= max_i(a, fontA[F]);
  }
  *Rw= w; *Ra= a; *Rh= h;
}

int *FYGfxCanvas::get_row_lines(int nr)
{
  if (n_row_lines==nr) return row_lines;
  if (row_lines) free(row_lines);
  row_lines= NULL;
  n_row_lines= nr;
  if (n_row_lines) 
    row_lines= (typeof(row_lines)) malloc(3*sizeof(row_lines[0])*nr);
  return row_lines;
}

int *FYGfxCanvas::get_col_lines(int nr)
{
  if (n_col_lines==nr) return col_lines;
  if (col_lines) free(col_lines);
  col_lines= NULL;
  n_col_lines= nr;
  if (n_col_lines) 
    col_lines= (typeof(col_lines)) malloc(3*sizeof(col_lines[0])*nr);
  return col_lines;
}


void FYGfxCanvas::draw_row_lines(int y1, int y2, FXDCWindow *dc)
{
  int i,j,k;
  if (!n_row_lines || !row_lines) return ;

  i= 0; j= n_row_lines-1;
  while(i<=j)
  {
    k= (i+j)/2;
    if (row_lines[3*k] < y1) i= k+1; 
    else if (row_lines[3*k] > y2) j= k-1; 
    else break;
  }
  if (i>j) return ;
  dc->setForeground(colors[0]);

  for(i=k;i>=0 && row_lines[3*i]>=y1;i--)
    dc->drawLine(row_lines[3*i+1]+pos_x, row_lines[3*i]+pos_y,
                 row_lines[3*i+2]+pos_x, row_lines[3*i]+pos_y);

  for(j=k+1;j<n_row_lines && row_lines[3*j]<=y2;j++)
    dc->drawLine(row_lines[3*j+1]+pos_x, row_lines[3*j]+pos_y,
                 row_lines[3*j+2]+pos_x, row_lines[3*j]+pos_y);
}


void FYGfxCanvas::draw_col_lines(int y1, int y2, FXDCWindow *dc)
{
  int i,j,k;
  if (!n_col_lines || !col_lines) return ;

  i= 0; j= n_col_lines-1;
  while(i<=j)
  {
    k= (i+j)/2;
    if (col_lines[3*k+1] < y1) i= k+1; 
    else if (col_lines[3*k] > y2) j= k-1; 
    else break;
  }
  if (i>j) return ;
  dc->setForeground(colors[0]);

  for(i=k;i>=0 && col_lines[3*i+1]>=y1;i--)
    dc->drawLine(col_lines[3*i+2]+pos_x, col_lines[3*i]+pos_y,
                 col_lines[3*i+2]+pos_x, col_lines[3*i+1]+pos_y);

  for(j=k+1;j<n_col_lines && col_lines[3*j]<=y2;j++)
    dc->drawLine(col_lines[3*j+2]+pos_x, col_lines[3*j]+pos_y,
                 col_lines[3*j+2]+pos_x, col_lines[3*j+1]+pos_y);
}


/***********************************************************************
 *                                                                     *
 *                               Images                                *
 *                                                                     *
 ***********************************************************************/
// uu-img 
void FYGfxCanvas::set_images(imagedef_t *id, int n)
{
  if (id==imagedefs) return ;
  destroy_images();
  n_images= n;
  if (!n_images) return ;
  imagedefs= id;
  images= (typeof(images)) calloc(n, sizeof(images[0]));
}


void FYGfxCanvas::image(int n)
{
  images[n].y1= cursor_y;
  cursor_y+= imagedefs[n].height;
  images[n].y2= cursor_y-1;
  images[n].x= max_i(0,(viewport_w - imagedefs[n].width)/2);
  max_x= max_i(max_x, images[n].x + imagedefs[n].width);
  max_y= cursor_y;
}

void FYGfxCanvas::draw_images(FXDCWindow *dc, int y1, int y2)
{
  int i,j,k;
  if (!n_images) return ;
  i= 0; j= n_images-1;
  while(i<=j)
  {
    k= (i+j)/2;
    if (images[k].y1>y2) i= k+1; 
    else if (images[k].y2<y1) j= k-1;
    else break;
  }
  if (i>j) return ;
  for(i=k;i>0 && images[i-1].y2>=y1;i--) ;
  for(j=k;j<n_images-1 && images[j+1].y1<=y2;j++) ;

  for(k=i;k<=j;k++)
  {
    if (!images[k].image)
    {
      images[k].image= new FXPNGImage(FXApp::instance(), imagedefs[k].data);
      images[k].image->create();
    }
    dc->drawImage(images[k].image, images[k].x+pos_x, images[k].y1+pos_y);
  }
}

void FYGfxCanvas::destroy_images()
{
  int i;
  for(i=0;i<n_images;i++)
     if (images[i].image)
     {
         images[i].image->detach(); 
         delete images[i].image;
     }
  n_images= 0;
  if (images) free(images);
  imagedefs= NULL;
  images= NULL;
}

/***********************************************************************
 *                                                                     *
 *                                Links                                *
 *                                                                     *
 ***********************************************************************/
// uu-link
void FYGfxCanvas::set_links(const unsigned int* lk, int n)
{
  links= lk;
  n_links= n;
}

int FYGfxCanvas::find_element_by_coord(int evX,int evY)
{
  int i,j,k;

  i= 0; j= nelts-1;
  while(i<=j)
  {
    k= (i+j)/2;
    if (coords[k].y>evY) j= k-1;
    else if (coords[k].y+coords[k].h<=evY) i= k+1;
    else break;
  }
  if (i>j) return -1;

  for(i=k;i>=0;i--)
  {
    if (coords[i].y!=coords[k].y) break;
    if (coords[i].x+word_width[elts[2*i+1]]<evX) break;
    if (coords[i].x<=evX) return i;
  }
  for(i=k+1;i<nelts;i++)
  {
    if (coords[i].y!=coords[k].y) break;
    if (coords[i].x>evX) break;
    if (coords[i].x+word_width[elts[2*i+1]]>evX) return i;
  }
  return -1;
}

long FYGfxCanvas::onLeftBtnPress(FXObject* obj,FXSelector sel,void* ptr)
{
  FXEvent *event;
  int evX, evY;
  int k;
  int sl, el;

  event= (FXEvent*) ptr;
  if (!event || !nelts) return 1;
  evX= event->win_x-pos_x;
  evY= event->win_y-pos_y;

  k= find_element_by_coord(evX, evY);
  
  if (k==-1) return 1;

  if (find_decor(k, k, &sl, &el, links, n_links)) return 1;
  if (link_func) (*link_func)(links[3*sl+2]);
 
  return 1;
}


/***********************************************************************
 *                                                                     *
 *                              Painting                               *
 *                                                                     *
 ***********************************************************************/
// uu-paint

/* The disabled part below was for a delayed full-screen paint.
   It does work but it's ugly since the window borders moving around
   leave marks on the canvas. */
long FYGfxCanvas::onPaint(FXObject* ob,FXSelector sel,void* ptr)
{
  FXEvent* event=(FXEvent*)ptr;
#if 0
  if (0 && event->rect.x==0 && event->rect.y==0 &&
      event->rect.w>= width && event->rect.h>=height)
  {
    FXApp::instance()->addTimeout(this, 1, 250, NULL);
  } 
  else 
#endif
  {
    FXDCWindow dc(this,event);
    draw_everything( &dc, event->rect.x, event->rect.y,
                        event->rect.w, event->rect.h);
  }
  return 1;
}

/* I'm not very sure about the following functions. They find the lowest and
   heighest displayed row. This is done by a binary search.

   The first function finds the first row which intersects with the screen.
   i.e. y<scr y+h>=scr. */
int FYGfxCanvas::find_lower(int scr)
{
  int i,j,k,y1,y2;
  i=0; j= nelts-1;
  while(i<=j && coords[i].y!=coords[j].y)
  {
    k= (i+j)/2;
    y1= coords[k].y; y2= coords[k].y+coords[k].h;
    if (y1<=scr)
    { 
       if (y2>=scr) goto done;
       i= k+1;
       continue;
    }
    j= k-1;
  }
  if (i>j) return 0;
  k= i;
done:
  while(k>0 && coords[k-1].y==coords[k].y) k--;
  return k;
}


int FYGfxCanvas::find_higher(int scr)
{
  int i,j,k,y1,y2;
  i=0; j= nelts-1;
  while(i<=j && coords[i].y!=coords[j].y)
  {
    k= (i+j)/2;
    y1= coords[k].y; y2= coords[k].y+coords[k].h;
    if (y1<=scr)
    { 
       if (y2>=scr) goto done;
       i= k+1;
       continue;
    }
    j= k-1;
  }
  if (i>j) return nelts-1;
  k= j;
done:
  while(k<nelts-1 && coords[k+1].y==coords[k].y) k++;
  return k;
}

void FYGfxCanvas::draw_everything
  (FXDCWindow *dc, int evX,int evY, int evW,int evH)
{
  FXColor white;
  int se, ee,e;
  unsigned int style, word;

  white= FXRGB(0xff,0xff,0xff);
#if 0
  static unsigned int K;
  static int C[4];
  C[0]= FXRGB(0x00,0xff,0x00);
  C[1]= FXRGB(0x00,0x00,0xff);
  C[2]= FXRGB(0xff,0x00,0xff);
  C[3]= FXRGB(0x33,0x33,0x33);
  K= (K+1)%4;
  white= C[K];
#endif

  dc->setForeground(white);
  dc->fillRectangle(evX, evY, evW, evH);
  if (!coords || !elts || !words) return ;

  create_fonts();

  evY-= pos_y; evX-= pos_x;

  se= find_lower( evY );
  ee= find_higher(evY+evH-1);

  // printf("y1= %d y2= %d se= %d ee= %d\n", evY, evY+evH-1, se, ee);

  //se= 0; ee= nelts-1;
  draw_vboxes(dc, se,ee);
  draw_bgrects(dc, se,ee);

  draw_row_lines(evY, evY+evH-1, dc);
  draw_col_lines(evY, evY+evH-1, dc);

  if (0) { int i; printf("[PAINT]"); 
           for(i=se;i<=ee;i++) printf("%s",words[elts[2*i+1]]); 
           printf("\n"); }

  for(e=se;e<=ee;e++)
  {
     word= elts[e*2+1];
     style= elts[e*2];
     dc->setForeground(colors[(style>>8)&0xff]);
     dc->setFont(fonts[style&0xff]);
     dc->drawText(coords[e].x + pos_x,
                 coords[e].y+ coords[e].a + pos_y, 
                 words[word], strlen(words[word])); 
  }

  draw_underlines(dc, se,ee);
  draw_images(dc, evY, evY+evH-1);
}

/***********************************************************************
 *                                                                     *
 *                              Resizing                               *
 *                                                                     *
 ***********************************************************************/
// uu-resize

/* When we resize the widget, we will need to lay the text out again. This
   will be done by the function which did the initial layout. We simply
   start from scratch. So, we store the function somewhere and then call
   it when a resize occurs. However, we shouldn't do re-layouts all the
   time while the window is being resized. They discussed this in some
   email list and Jeroen suggested a timer. I'll try that. 

   It's not necessary to catch the resize event. It's OK if we simply
   override the layout method. If we don't have a resize function yet,
   we simply don't do anything since the widget is empty.

   Otherwise, we avoid laying out by starting a timer. We only do the
   re-layout when the timer expires. */

long FYGfxCanvas::on_layout_timeout(FXObject* ob,FXSelector sel,void* ptr)
{
  if (!resize_func) return 1;
  (*resize_func)(this);
  update();
  return 1;
}


void FYGfxCanvas::layout()
{
  if (!resize_func) goto done;
  // 250 msecs..
  FXApp::instance()->addTimeout(this, 0, 250, NULL);
done:
  FXScrollArea::layout();
}

void FYGfxCanvas::reset_doc()
{
  cursor_y= max_y= max_x= 0;
}

void FYGfxCanvas::end_doc(void (*f)(FYGfxCanvas*))
{
  resize_func= f;
  FXScrollArea::layout();
  update();
}
