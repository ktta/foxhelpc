#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "ifile.h"
#include "buffer.h"
#include "critbit.h"
#include "pngdim.h"

static const char *libhdr=
"#include <fx.h>\n"
"#include <FXString.h>\n"
"#include <FXApp.h>\n"
"#include <FXScrollBar.h>\n"
"#include <FXScrollArea.h>\n"
"#include <FXImage.h>\n"
"#include <FXDCWindow.h>\n"
"#include <FXFont.h>\n"
"#include <FXPNGImage.h>\n"
"#include <FXMenuCommand.h>\n"
"#include <FXTopWindow.h>\n"
"\n"
"#define CELL_PAD 5\n"
"\n"

;

static const char *libcode=
"/* The menu commands in FOX are designed for static menus. i.e. you\n"
"   can\047t dynamically generate menu entries. Actually you can, but it\047s\n"
"   cumbersome.  A better way is to subclass the menu command class and\n"
"   let the menu command do the necessary actions by itself.\n"
"\n"
"   These actions may also cause the menu command to be deleted. This seems\n"
"   to be fine. The handle() functions in the widgets don\047t refer to the\n"
"   object, they simply return. This could change in future releases of\n"
"   FOX but for now it\047s safe. */\n"
"\n"
"namespace FX\n"
"{\n"
"\n"
"class FXAPI FYMenuCommand : public FXMenuCommand\n"
"{\n"
"  FXDECLARE(FYMenuCommand)\n"
"protected:\n"
"  FYMenuCommand();\n"
"private:\n"
"  FYMenuCommand(const FYMenuCommand&);\n"
"  FYMenuCommand &operator=(const FYMenuCommand&);\n"
"\n"
"  void (*a_func)(int);\n"
"  int a_arg;\n"
"public:\n"
"  FYMenuCommand(FXComposite* p,\n"
"                const FXString& text,FXIcon* ic=NULL,\n"
"                FXuint opts=0, \n"
"                void (*afun)(int)= NULL, int aarg= 0);\n"
"  long run_func(FXObject* sender,FXSelector sel,void* ptr);\n"
"\n"
"  virtual ~FYMenuCommand();\n"
"};\n"
"}\n"
"\n"
"using namespace FX;\n"
"\n"
"FXDEFMAP(FYMenuCommand) FYMenuCommandMap[]=\n"
"{\n"
"  FXMAPFUNC(SEL_COMMAND, 2216, FYMenuCommand::run_func),\n"
"};\n"
"\n"
"\n"
"FXIMPLEMENT(FYMenuCommand,FXMenuCommand,FYMenuCommandMap,\n"
"            ARRAYNUMBER(FYMenuCommandMap))\n"
"\n"
"FYMenuCommand::FYMenuCommand(){ }\n"
"FYMenuCommand::~FYMenuCommand() { }\n"
"\n"
"FYMenuCommand::FYMenuCommand\n"
"             (FXComposite* p,\n"
"              const FXString& text,FXIcon* ic,\n"
"              FXuint opts,\n"
"              void (*afun)(int), int aarg)\n"
"   : FXMenuCommand(p, text, ic, this, 2216, opts)\n"
"{\n"
"  a_func= afun;\n"
"  a_arg= aarg;\n"
"}\n"
"\n"
"long FYMenuCommand::run_func(FXObject* sender,FXSelector sel,void* ptr)\n"
"{\n"
"  if (a_func) (*a_func)(a_arg);\n"
"  return 1;\n"
"}\n"
"\n"
"/*\n"
"  The graphics canvas is a very simple thing. It maintains a \047document\047 and\n"
"  updates the screen when necessary. Later on, I\047ll add some double buffering\n"
"  to avoid flicker. \n"
"\n"
"             Table of Contents\n"
"   \n"
"  Constructor                     uu-cons\n"
"  Destructor                      uu-des\n"
"  Detachment                      uu-deta\n"
"  Initialization (fonts, colors)  uu-obinit\n"
"  Text Handling                   uu-text\n"
"  Text Decorations                uu-decor\n"
"  Verbatim Text                   uu-ver\n"
"  Lists                           uu-list\n"
"  Tables                          uu-table\n"
"  Images                          uu-img\n"
"  Links                           uu-link\n"
"  Painting                        uu-paint\n"
"  Resizing                        uu-resize\n"
"*/\n"
"\n"
"namespace FX\n"
"{\n"
"\n"
"/* This struct holds the coordinates of a text element. We have an\n"
"   array of these, one for each element. */\n"
"typedef struct { int x,y, h,a; } coord_t;\n"
"\n"
"/* An imagedef is a read-only struct describing the source for an image.\n"
"   \047data\047 is PNG encoded. */\n"
"typedef struct { int width, height; const unsigned char *data; } imagedef_t;\n"
"\n"
"/* An himage is the run-time generated struct corresponding to an image. */\n"
"typedef struct { int x,y1,y2; FXImage* image; } himage_t;\n"
"\n"
"\n"
"class FXAPI FYGfxCanvas : public FXScrollArea\n"
"{\n"
"  FXDECLARE(FYGfxCanvas)\n"
"protected:\n"
"  FYGfxCanvas();\n"
"private:\n"
"  FYGfxCanvas(const FYGfxCanvas&);\n"
"  FYGfxCanvas &operator=(const FYGfxCanvas&);\n"
"\n"
"public:\n"
"  FYGfxCanvas(FXComposite* p, FXuint opts=FRAME_NORMAL,\n"
"              FXint x=0,FXint y=0,FXint w=0,FXint h=0);\n"
"  virtual ~FYGfxCanvas();\n"
"\n"
"  virtual bool canFocus() const;\n"
"  virtual void detach();\n"
"\n"
"  virtual int getContentWidth();\n"
"  virtual int getContentHeight();\n"
"\n"
"/* Since we have a scrolling area, we need to keep track of the maximum\n"
"   x and y coordinates for the page. This lets the scroll area to properly\n"
"   display scroll bars. */\n"
"\n"
"private: int max_x,max_y, min_x,min_y;\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                           Initialization                            *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"/* We can handle 256 fonts. Each font has some metrics:\n"
"   - A: ascent\n"
"   - H: height\n"
"   - S: width of space */\n"
"private:\n"
"   FXFont *fonts[256];\n"
"   int fontA[256], fontH[256],fontS[256];\n"
"\n"
"/* We initialize the font structures using this thing. All used fonts\n"
"   must be defined using this function. \n"
"     FIXME: maybe we can copy a font from another (the default?)\n"
"            if a used font isn\047t defined.  */\n"
"public:\n"
"   void define_font(int num,const char *family,int size,const char* style);\n"
"\n"
"/* In order to keep track of whether we have created fonts, we use the\n"
"   following flag. The FOX system of creation and detachment aren\047t very\n"
"   clear to me, so I simply create the fonts when I need them. */\n"
"\n"
"private: int fonts_created;\n"
"         void create_fonts();\n"
"\n"
"/* We also support 256 colors for text. These are initialized with the\n"
"   below function. Since colors don\047t need to be created, we don\047t have a\n"
"   create function or a flag for it. */\n"
"\n"
"private: FXColor colors[256];\n"
"public:  void define_color(int num, unsigned int C);\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                            Text Handling                            *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-text\n"
"/* The canvas will primarily hold text. The text is split up into three\n"
"   logical sections:\n"
"\n"
"   - words       : int -> string mapping\n"
"   - elements    : a sequence of <style,word_index> \n"
"   - coordinates : coordinates for each element\n"
"\n"
"   Words and elements are read-only and are set when the controlling widget\n"
"   sets the document. Coordinates are created by this widget and is filled\n"
"   in when the controlling function tells the canvas to paint the elements\n"
"   in a certain way. \n"
"\n"
"   The word array is set for the whole help document. Pages do share the\n"
"   word array. The word_width array is used for caching the width of \n"
"   each word. Note that a word is the combination of a string with a\n"
"   font. i.e. the word \047the\047 is represented twice in the words array\n"
"   if it occurs with the default font and the top header font. */\n"
"\n"
"private: const char **words;\n"
"         unsigned short *word_width;\n"
"public:  void set_words(const char **wrd,int nw);\n"
"\n"
"/* In order to lay out the text properly, we need some metrics: width, height\n"
"   and ascent. We always return the font height and ascent, in order to provide\n"
"   consistent line heights. This function is used to compute the contents\n"
"   of the word_width array. */\n"
"public:  int text_width(int f, const char *str);\n"
"\n"
"/* Elements are set per page. The element array consists of <style,word>\n"
"   pairs. \n"
"\n"
"   Each style is an integer of the form: \n"
"       U-B(8)-F(8)-T(8)\n"
"   U: underline bit\n"
"   B: background color index to colors[]\n"
"   F: foreground color index to colors[]\n"
"   T: font index to fonts[]\n"
"\n"
"   The set_elts function also initializes the coords array. */\n"
"\n"
"private: const unsigned int *elts;\n"
"         unsigned int nelts;\n"
"         coord_t *coords;\n"
"public:  void set_elts(const unsigned int *e, unsigned int ne);\n"
"\n"
"/* When printing text, we need to keep track of the \047cursor\047 position.\n"
"   The canvas works something like a terminal. You \047print\047 stuff to it.\n"
"   The stuff you can print is already in the elts[] array but you need\n"
"   to tell it where a paragraph starts and ends. The following members\n"
"   keep where you left off last time.\n"
"      FIXME: cursor_x is in fact never accessed across method invocations.\n"
"             It could be a local variable for println() and stuff. */\n"
"\n"
"private: int cursor_x, cursor_y;\n"
"\n"
"/* Our main method for printing text will be println. This function prints\n"
"the elements elts[start,end] as a single paragraph, and then advances\n"
"cursor_y so that later printings can continue from below. */\n"
"\n"
"public:  void println(int start, int end);\n"
"         void pbreak();\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                          Text Decorations                           *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-decor\n"
"/* We will also have some inline decorations. These are background\n"
"   rectangles, verbatim boxes and underlines. These decorations are\n"
"   represented as integer pairs, indicating the start and end element indices\n"
"   for the decoration. i.e. rect(red,3,5) means that we should draw a red\n"
"   rectangle covering the background of elements 3,4,5.\n"
"\n"
"   The following members will hold the corresponding element indices. [0] is\n"
"   start, [1] is end and [2] is color for each triplet. */\n"
"\n"
"private: const unsigned int *bgrects, *underlines,*vboxes;\n"
"         unsigned int n_bgrects, n_underlines,n_vboxes;\n"
"\n"
"public:  void set_vboxes(const unsigned int *d, int nd);\n"
"         void set_underlines(const unsigned int *d, int nd);\n"
"         void set_bgrects(const unsigned int *d, int nd);\n"
"\n"
"private: void draw_bgrects(FXDCWindow *dc,int se,int ee);\n"
"         void draw_underlines(FXDCWindow *dc,int se,int ee);\n"
"         void draw_vboxes(FXDCWindow *dc,int se,int ee);\n"
"\n"
"\n"
"private: int find_decor(int se, int ee, int *sD, int *eD, \n"
"                        const unsigned int *decor,unsigned int ndecor);\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                            Verbatim Text                            *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-ver\n"
"/* Vertical and left external padding for verbatim text boxes. These\n"
"   are the paddding for the text itself, not the box. */\n"
"private: int vpad_verbatim, lpad_verbatim;\n"
"\n"
"/* Just like println, but:\n"
"   - elements are complete strings, with spaces etc. inside them\n"
"   - we don\047t wrap around. */\n"
"public:  void verbatim_line(int start, int end);\n"
"\n"
"/* Advances cursor_y to make space. */\n"
"public:  void verbatim_vspace();\n"
"\n"
"// uu-list\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                                Lists                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"/* finds out which character is present in the normal font and uses\n"
"   that as the list bullet. */\n"
"private: void figure_out_bullet();\n"
"\n"
"         int lpad_paragraph;  // left padding for a normal paragraph\n"
"                              // list code modifies this to fool println.\n"
"public:  void bullet_println(int level,int start, int end);\n"
"\n"
"//uu-table\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                               Tables                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"/* For tables, we do the whole layout from outside. So, we need to access\n"
"   some metrics: */\n"
"\n"
"public: void get_seq_metrics(int start,int end,int *Rw,int *Rh,int *Ra);\n"
"\n"
"/* Print elts[s..e] justified. x1 and x2 are absolute, y is relative\n"
"   to cursor_y. */\n"
"\n"
"        void cell_left(int x1,int x2,int y,int h,int a,int s, int e);\n"
"        void cell_center(int x1,int x2, int y,int h,int a,int s,int e);\n"
"        void cell_right(int x1,int x2,int y,int h,int a,int s, int e);\n"
"\n"
"/* We also need to support table borders and internal lines. These are\n"
"   a little different from the other decorations in the sense that they\n"
"   are not bound to any element but have their own absolute coordinates.\n"
"\n"
"   However, their number is a fixed value and the array can be allocated\n"
"   once.  I will implement the horizontal and vertical lines separately.\n"
"\n"
"   Let\047s start with the horizontal ones. We just need to allocate the\n"
"   associated array once when we initially switch to a page and then\n"
"   do the layout on the same array over and over without reallocating\n"
"   it. We only reallocate it when the page changes.\n"
"\n"
"   The horizontal lines will be composed of three integers each: y,\n"
"   x1 and x2. Vertical lines will be very similar with the\n"
"   triplet <y1,y2,x>. */\n"
"\n"
"private: int *row_lines; int n_row_lines;\n"
"         int *col_lines; int n_col_lines;\n"
"\n"
"/* The following functions allocate enough space for the given number\n"
"   of lines. They don\047t initialize anything. They just make sure enough\n"
"   space is allocated. */\n"
"public:  int *get_row_lines(int nr);\n"
"         int *get_col_lines(int nr);\n"
"\n"
"/* Some access functions needed by the table stuff. */\n"
"public:  int get_width() { return viewport_w; }\n"
"         void advance_y(int u) { cursor_y += u; max_y= cursor_y; }\n"
"         int get_cursor_y() { return cursor_y; }\n"
"         void xcoord(int x) { if (x>max_x) max_x= x; } \n"
"\n"
"private: void draw_col_lines(int y1, int y2, FXDCWindow *dc);\n"
"         void draw_row_lines(int y1, int y2, FXDCWindow *dc);\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                               Images                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-img\n"
"/* Image definition array is set by the page content function using\n"
"   set_images method. The image(n) method then later puts the given\n"
"   image at the cursor_y position with horizontal centering. */\n"
"\n"
"private: int n_images;\n"
"         imagedef_t *imagedefs;\n"
"         himage_t   *images;\n"
"public:  void set_images(imagedef_t *id, int n);\n"
"         void image(int n);\n"
"private: void draw_images(FXDCWindow *dc, int y1, int y2);\n"
"\n"
"/* We need to destroy images when we change pages or destroy \n"
"   the help widget.  */\n"
"\n"
"private: void destroy_images();\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                                Links                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-link\n"
"/* Links are very similar to decorations. Their representation in this\n"
"   class is actually identical: an array of integer triplets \n"
"   <start,end,target> */\n"
"private: const unsigned int *links; int n_links;\n"
"public:  void set_links(const unsigned int*, int);\n"
"         void (*link_func)(int);\n"
"         long onLeftBtnPress(FXObject*,FXSelector,void*);\n"
"         int find_element_by_coord(int x,int y);\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                              Painting                               *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-paint\n"
"public:  long onPaint(FXObject* ob,FXSelector sel,void* ptr);\n"
"         int find_lower(int scr);\n"
"         int find_higher(int scr);\n"
"         void draw_everything (FXDCWindow *dc, int evX,int evY,\n"
"                                               int evW,int evH);\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                              Resizing                               *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"//uu-resize\n"
"private: void (*resize_func)(FYGfxCanvas *);\n"
"\n"
"/* We mark the beginning and end of the page with the following functions\n"
"   so that the widget knows when to update its scrollbars. */\n"
"\n"
"public:  void reset_doc();\n"
"         void end_doc(void (*f)(FYGfxCanvas*));\n"
"\n"
"         virtual void layout();\n"
"         long on_layout_timeout(FXObject* ob,FXSelector sel,void* ptr);\n"
"\n"
"// uz-end\n"
"};\n"
"}\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                            Boiler-plate                             *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"using namespace FX;\n"
"\n"
"static inline int min_i(int a,int b) { return a<b? a:b; }\n"
"static inline int max_i(int a,int b) { return a>b? a:b; }\n"
"\n"
"FXDEFMAP(FYGfxCanvas) FYGfxCanvasMap[]=\n"
"{\n"
"  FXMAPFUNC(SEL_PAINT,0,FYGfxCanvas::onPaint),\n"
"  // uu-resize\n"
"  FXMAPFUNC(SEL_TIMEOUT, 0, FYGfxCanvas::on_layout_timeout),\n"
"  // uu-link\n"
"  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0, FYGfxCanvas::onLeftBtnPress),\n"
"};\n"
"\n"
"FXIMPLEMENT(FYGfxCanvas,FXScrollArea,FYGfxCanvasMap,\n"
"            ARRAYNUMBER(FYGfxCanvasMap))\n"
"\n"
"FYGfxCanvas::FYGfxCanvas(){ flags|=FLAG_ENABLED|FLAG_SHOWN; }\n"
"bool FYGfxCanvas::canFocus() const { return true; }\n"
"int FYGfxCanvas::getContentHeight() { return max_y; }\n"
"int FYGfxCanvas::getContentWidth() { return max_x; }\n"
"\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                      Constructor / Destructor                       *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"\n"
"FYGfxCanvas::FYGfxCanvas\n"
"     (FXComposite* p, FXuint opts, FXint x,FXint y,FXint w,FXint h):\n"
"\011\011FXScrollArea(p,opts | VSCROLLER_ALWAYS ,x,y,w,h)\n"
"{\n"
"  flags|=FLAG_ENABLED;\n"
"\n"
"  fonts_created= 0;\n"
"  { int i; for(i=0;i<256;i++) fonts[i]= NULL; }\n"
"\n"
"  coords= NULL; words= NULL; elts= NULL; nelts= 0;\n"
"  word_width= NULL;\n"
"\n"
"  cursor_x= cursor_y=0;\n"
"\n"
"  max_x= max_y= min_x= min_y= 0;\n"
"\n"
"  images= NULL; imagedefs= NULL; n_images= 0;\n"
"\n"
"  bgrects= underlines= vboxes= NULL;\n"
"  n_bgrects= n_underlines= n_vboxes= 0;\n"
"\n"
"  vpad_verbatim= 5; lpad_verbatim= 15; \n"
"  lpad_paragraph= 10;\n"
"\n"
"  // uu-resize\n"
"  resize_func= NULL;\n"
"\n"
"  // uu-table\n"
"  row_lines= NULL; n_row_lines=0;\n"
"  col_lines= NULL; n_col_lines=0;\n"
"\n"
"// uu-cons\n"
"}\n"
"\n"
"\n"
"void FYGfxCanvas::detach()\n"
"{ \n"
"  int i;\n"
"\n"
"  FXScrollArea::detach();\n"
"\n"
"  if (fonts_created)\n"
"     for(i=0;i<256;i++) \n"
"        if (fonts[i]) fonts[i]->detach(); \n"
"// uu-deta\n"
"}\n"
"\n"
"FYGfxCanvas::~FYGfxCanvas() { \n"
"  int i;\n"
"\n"
"  for(i=0;i<256;i++) \n"
"     if (fonts[i]) delete fonts[i];\n"
"\n"
"  if (coords) { free(coords); coords= NULL; }\n"
"\n"
"  destroy_images();\n"
"\n"
"  // uu-table\n"
"  if (row_lines) free(row_lines);\n"
"  if (col_lines) free(col_lines);\n"
"// uu-des\n"
"}\n"
"\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                           Initialization                            *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-obinit\n"
"void FYGfxCanvas::define_font\n"
"   (int num,const char *family,int size,const char* style)\n"
"{\n"
"   int weight;\n"
"   int slant;\n"
"   if (num>=256) return ;\n"
"   \n"
"   if (strchr(style,\047B\047))\n"
"      weight= FXFont::Bold;\n"
"   else\n"
"      weight= FXFont::Normal;\n"
"   if (strchr(style,\047I\047))\n"
"      slant= FXFont::Italic;\n"
"   else\n"
"      slant= FXFont::Straight;\n"
"\n"
"   fonts[num]= new FXFont(FXApp::instance(),family, size, weight,slant);\n"
"}\n"
"\n"
"void FYGfxCanvas::create_fonts()\n"
"{\n"
"  if (!fonts_created)\n"
"  {\n"
"    int i;\n"
"    fonts_created= 1;\n"
"    for(i=0;i<256;i++) \n"
"      if (fonts[i])\n"
"      {\n"
"         fonts[i]->create();\n"
"         fontA[i]= fonts[i]->getFontAscent();\n"
"         fontH[i]= fonts[i]->getFontHeight();\n"
"         fontS[i]= fonts[i]->getTextWidth(\042 \042, 1);\n"
"      }\n"
"    figure_out_bullet();\n"
"  }\n"
"}\n"
"\n"
"void FYGfxCanvas::define_color(int num, unsigned int C)\n"
"{\n"
"   unsigned R,G,B;\n"
"   if (num>=256) return ;\n"
"   R= (C>>16u)&0xffu;\n"
"   G= (C>>8u)&0xffu;\n"
"   B= C&0xffu;\n"
"   colors[num]= FXRGB(R,G,B);\n"
"}\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                            Text Handling                            *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"\n"
"// uu-text\n"
"void FYGfxCanvas::set_words(const char **wrd,int nw)\n"
"{\n"
"  words= wrd;\n"
"  if (word_width) free(word_width);\n"
"  word_width= (typeof(word_width)) calloc(nw, sizeof(word_width[0]));\n"
"}\n"
"\n"
"void FYGfxCanvas::set_elts(const unsigned int *e, unsigned int ne)\n"
"{\n"
"  elts= e;\n"
"  nelts= ne;\n"
"  if (coords) free(coords);\n"
"  coords= (typeof(coords)) malloc(sizeof(*coords)*ne);\n"
"}\n"
"\n"
"int FYGfxCanvas::text_width(int f, const char *str)\n"
"{\n"
"  create_fonts();\n"
"  if (f<0 || f>=256 || !fonts[f]) return 1;\n"
"  return fonts[f]->getTextWidth(str, strlen(str));\n"
"}\n"
"\n"
"void FYGfxCanvas::println(int start, int end)\n"
"{\n"
"  int i,j,f,ww,w;\n"
"  int row_a, row_h;\n"
"  int canvas_w;\n"
"\n"
"  int os= start;\n"
"\n"
"  create_fonts();\n"
"  canvas_w= viewport_w-lpad_paragraph-10; //FIXME: we need rpad_paragraph also\n"
"again:\n"
"  cursor_x= lpad_paragraph;\n"
"  row_a= fontA[0];\n"
"  row_h= fontH[0];\n"
"  for(i=start;i<=end;i++)\n"
"  {\n"
"    f= elts[2*i]&0xff;\n"
"    w= elts[2*i+1];\n"
"    ww= word_width[w];\n"
"    if (ww==0) \n"
"       word_width[w]= ww= text_width(f, words[w]);\n"
"    if (cursor_x+ww > canvas_w && i!=start) break;\n"
"    coords[i].x= cursor_x;\n"
"    cursor_x+= ww ;\n"
"    max_x= max_i(max_x, cursor_x);\n"
"    row_a= max_i(row_a, fontA[f]);\n"
"    row_h= max_i(row_h, fontH[f]);\n"
"    cursor_x+= fontS[f];\n"
"  }\n"
"\n"
"  for(j=start;j<i;j++)\n"
"  {\n"
"    coords[j].y= cursor_y;\n"
"    coords[j].h= row_h;\n"
"    coords[j].a= row_a;\n"
"  }\n"
"  cursor_y += row_h + 2;\n"
"  max_y= cursor_y;\n"
"\n"
"  if (i<=end) { start= i; goto again; }\n"
"\n"
"  if (0)\n"
"  { printf(\042-----------------\134n\042);\n"
"     for(i=os;i<=end;i++)\n"
"        printf(\042(x=%d,y=%d,a=%d,h=%d,w=%d)[%d:%s]\134n\042, coords[i].x, \n"
"                        coords[i].y, coords[i].a, coords[i].h,\n"
"                        word_width[elts[2*i+1]],\n"
"                        elts[2*i+1],\n"
"                        words[elts[2*i+1]]); }\n"
"}\n"
"\n"
"void FYGfxCanvas::pbreak()\n"
"{\n"
"  create_fonts();\n"
"  cursor_y+= fontH[0];\n"
"  cursor_x= 0;\n"
"}\n"
"\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                          Text Decorations                           *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-decor\n"
"void FYGfxCanvas::set_bgrects(const unsigned int *d, int nd)\n"
"{\n"
"  bgrects= d;\n"
"  n_bgrects= nd;\n"
"}\n"
"\n"
"void FYGfxCanvas::set_underlines(const unsigned int *d, int nd)\n"
"{\n"
"  underlines= d;\n"
"  n_underlines= nd;\n"
"}\n"
"\n"
"void FYGfxCanvas::set_vboxes(const unsigned int *d, int nd)\n"
"{\n"
"  vboxes= d;\n"
"  n_vboxes= nd;\n"
"}\n"
"\n"
"\n"
"/* Just like we find the elements visible within a window, we need to find\n"
"   decorations visible within a given element-window. */\n"
"\n"
"int FYGfxCanvas::find_decor(int se, int ee, int *sD, int *eD, \n"
"                const unsigned int *decor,unsigned int ndecor)\n"
"{\n"
"#if 1\n"
"  int i,j,k,S,E;\n"
"  i=0; j= ndecor-1;\n"
"  while(i<=j)\n"
"  {\n"
"    k= (i+j)/2;\n"
"    S= decor[k*3]; E= decor[k*3+1];\n"
"    if (E<se) { i= k+1; continue; }\n"
"    if (S>ee) { j= k-1; continue; }\n"
"    goto found;\n"
"  }\n"
"  return 1;\n"
"found:\n"
"  i=j= k;\n"
"  while(i>=0 && decor[i*3+1] >= se)    { *sD= i; i--; }\n"
"  while(j<=ndecor-1 && decor[j*3]<=ee) { *eD= j; j++;  }\n"
"  //printf(\042decor found for (%d %d) : (%d %d)\134n\042, se, ee, *sD, *eD); \n"
"  return 0;\n"
"#else\n"
"\n"
"  int i,j,k;\n"
"  for(i=0;i<ndecor;i++)\n"
"    if ((decor[i*3+1]>=se && decor[i*3+1]<=ee)\n"
"        || (decor[i*3]>=se && decor[i*3]<=ee) )\n"
"        { *sD= *eD= i; break; }\n"
"  if (i==ndecor) return 1;\n"
"\n"
"  for(i=ndecor-1;i>=0;i--)\n"
"    if ((decor[i*3+1]>=se && decor[i*3+1]<=ee)\n"
"        || (decor[i*3]>=se && decor[i*3]<=ee) )\n"
"        { *eD= i; break; }\n"
"\n"
"  return 0;\n"
"#endif\n"
"}\n"
"\n"
"void FYGfxCanvas::draw_bgrects(FXDCWindow *dc,int se,int ee)\n"
"{\n"
"  int sb, eb;\n"
"  if (!n_bgrects || !bgrects) return ;\n"
"  if (find_decor(se, ee, &sb, &eb, bgrects, n_bgrects)) return ;\n"
"  \n"
"  while(sb<=eb)\n"
"  {\n"
"    int S,E,C;\n"
"    S= bgrects[sb*3];\n"
"    E= bgrects[sb*3+1];\n"
"    C= bgrects[sb*3+2];\n"
"\n"
"    int sx, ex;\n"
"    int sy,h;\n"
"\n"
"    dc->setForeground(colors[C]);\n"
"    while(S<=E)\n"
"    {\n"
"      h= coords[S].h;\n"
"      sx= coords[S].x;\n"
"      sy= coords[S].y;\n"
"      while(S<=E && coords[S].y==sy) \n"
"         { ex= coords[S].x+word_width[elts[2*S+1]]; S++; }\n"
"      dc->fillRectangle(sx+pos_x, sy+pos_y, ex-sx, h);\n"
"      //printf(\042RECT(%d,%d,%d,%d)\134n\042, sx+pos_x, sy+pos_y, ex-sx, h);\n"
"    }\n"
"    sb++;\n"
"  }\n"
"}\n"
"\n"
"void FYGfxCanvas::draw_underlines(FXDCWindow *dc,int se,int ee)\n"
"{\n"
"  int sb, eb;\n"
"  if (!n_underlines || !underlines) return ;\n"
"  if (find_decor(se, ee, &sb, &eb, underlines, n_underlines)) return ;\n"
"  \n"
"  while(sb<=eb)\n"
"  {\n"
"    int S,E,C;\n"
"    S= underlines[sb*3];\n"
"    E= underlines[sb*3+1];\n"
"    C= underlines[sb*3+2];\n"
"\n"
"    int sx, ex;\n"
"    int sy,a;\n"
"\n"
"    dc->setForeground(colors[C]);\n"
"    while(S<=E)\n"
"    {\n"
"      a= coords[S].a;\n"
"      sx= coords[S].x;\n"
"      sy= coords[S].y;\n"
"      while(S<=E && coords[S].y==sy) \n"
"         { ex= coords[S].x+word_width[elts[2*S+1]]; S++; }\n"
"      dc->drawLine(sx+pos_x, sy+a+pos_y+2, ex+pos_x, sy+a+pos_y+2);\n"
"    }\n"
"    sb++;\n"
"  }\n"
"}\n"
"\n"
"void FYGfxCanvas::draw_vboxes(FXDCWindow *dc,int se,int ee)\n"
"{\n"
"  int sb, eb;\n"
"  if (!n_vboxes || !vboxes) return ;\n"
"  if (find_decor(se, ee, &sb, &eb, vboxes, n_vboxes)) return ;\n"
"\n"
"  // printf(\042found boxes: %d -> %d\134n\042, sb, eb);\n"
"  \n"
"  while(sb<=eb)\n"
"  {\n"
"    int S,E,C;\n"
"    int i;\n"
"    int mx;\n"
"    S= vboxes[sb*3];\n"
"    E= vboxes[sb*3+1];\n"
"    C= vboxes[sb*3+2];\n"
"\n"
"    dc->setForeground(colors[C]);\n"
"\n"
"    mx= word_width[elts[2*S+1]] + coords[S].x;\n"
"    for(i=S+1;i<=E;i++)\n"
"       mx= max_i(mx,word_width[elts[2*i+1]] + coords[i].x); \n"
"\n"
"    int x1,x2,y1,y2;\n"
"    x1= lpad_paragraph;\n"
"    y1= coords[S].y-vpad_verbatim;\n"
"    x2= mx+ lpad_verbatim-lpad_paragraph;\n"
"    y2= coords[E].y+coords[E].h + vpad_verbatim;\n"
"    dc->fillRectangle(x1+pos_x, y1+pos_y, x2-x1, y2-y1);\n"
"    dc->setForeground(colors[0]);\n"
"    dc->drawRectangle(x1+pos_x, y1+pos_y, x2-x1, y2-y1);\n"
"    sb++;\n"
"  }\n"
"}\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                            Verbatim Text                            *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-ver\n"
"void FYGfxCanvas::verbatim_vspace()\n"
"{\n"
"  cursor_y+= vpad_verbatim;\n"
"  max_y= cursor_y;\n"
"}\n"
"\n"
"/* we just keep on going, never wrapping around. */\n"
"void FYGfxCanvas::verbatim_line(int start, int end)\n"
"{\n"
"  int i,j,f,ww,w;\n"
"  int row_a, row_h;\n"
"\n"
"  int os= start;\n"
"\n"
"  create_fonts();\n"
"\n"
"  cursor_x= lpad_verbatim;\n"
"  row_a= fontA[0];\n"
"  row_h= fontH[0];\n"
"  for(i=start;i<=end;i++)\n"
"  {\n"
"    f= elts[2*i]&0xff;\n"
"    w= elts[2*i+1];\n"
"    ww= word_width[w];\n"
"    if (ww==0) \n"
"       word_width[w]= ww= text_width(f, words[w]);\n"
"    coords[i].x= cursor_x;\n"
"    cursor_x+= ww;\n"
"    max_x= max_i(max_x, cursor_x);\n"
"    row_a= max_i(row_a, fontA[f]);\n"
"    row_h= max_i(row_h, fontH[f]);\n"
"  }\n"
"\n"
"  for(j=start;j<=end;j++)\n"
"  {\n"
"    coords[j].y= cursor_y;\n"
"    coords[j].h= row_h;\n"
"    coords[j].a= row_a;\n"
"  }\n"
"  cursor_y += row_h + 2;\n"
"  max_y= cursor_y;\n"
"}\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                                Lists                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-list\n"
"/* In order to display lists, we use \047bullet paragraphs\047. What we do is:\n"
"    - make some space on the left for indentation\n"
"    - print a bullet character\n"
"    - adjust the paragraph start to point after the bullet character\n"
"    - call println()\n"
"\n"
"   The bullet character is in fact a string, just like other text. This\n"
"   string is somewhat special, its value depends on the font. If the\n"
"   normal font doesn\047t contain the default bullet character, other\n"
"   characters should be tried.\n"
"\n"
"   So, when we set up the fonts, I should try to get the width of the\n"
"   bullet string. Normally, I\047d have the \047words\047 array as:\n"
"\n"
"      const char **const words\n"
"\n"
"   but this wouldn\047t let me modify words[0], which will be the bullet\n"
"   string.  So, I will keep it as:\n"
"\n"
"      const char **words\n"
"\n"
"   and then modify words[0] accordingly. */\n"
"\n"
"void FYGfxCanvas::figure_out_bullet()\n"
"{\n"
"  int k,i;\n"
"  const char *blt[]= { \042\342\232\253\042,\042\342\226\252\042,\042\342\230\205\042 ,\042\342\231\246\042 ,\042\342\227\206\042 ,\042\342\234\270\042 ,\042*\042, NULL};\n"
"  for(i=0;blt[i];i++)\n"
"  {\n"
"    FXString str= blt[i];\n"
"    if (fonts[0]->hasChar(str.wc(0)))\n"
"    {\n"
"       k= text_width(0, blt[i]);\n"
"       break;\n"
"    }\n"
"  }\n"
"  if (!blt[i]) { i--; k= 16; }\n"
"  words[0]= blt[i];\n"
"  word_width[0]= k;\n"
"}\n"
"\n"
"\n"
"/* The following function does the actual bullet paragraph printing.\n"
"   elts[2*start] should contain <normal,0>, which points to the bullet\n"
"   string. The rest is the actual content for the paragraph. */\n"
"\n"
"void FYGfxCanvas::bullet_println(int level,int start, int end)\n"
"{\n"
"  int lp;\n"
"  unsigned int bw, bs;\n"
"  lp= lpad_paragraph;\n"
"\n"
"  lpad_paragraph= level*30;  // FIXME: make it configurable\n"
"  bw= elts[2*start+1];\n"
"  bs= elts[2*start];\n"
"\n"
"  if (word_width[bw]==0) \n"
"     word_width[bw]= text_width(bs&0xff, words[bw]);\n"
"\n"
"  coords[start].x= lpad_paragraph-word_width[bw] - 1;\n"
"  coords[start].y= cursor_y;\n"
"  coords[start].h= fontH[0]; \n"
"  coords[start].a= fontA[0]; \n"
"\n"
"  println(start+1, end);\n"
"  if (coords[start+1].y==coords[start].y)\n"
"  {\n"
"   coords[start].h= max_i(coords[start].h , coords[start+1].h);\n"
"   coords[start].a= max_i(coords[start].a , coords[start+1].a);\n"
"  }\n"
"\n"
"  lpad_paragraph= lp;\n"
"}\n"
"\n"
"/* Note that using words[0] isn\047t mandatory at all. You can use whatever\n"
"   you want. Just put it in the elt[] array and this function will use\n"
"   the first word as the bullet. */\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                               Tables                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-table\n"
"/* When laying out table cells, we do the width / height computation\n"
"   of cells outside of canvas. We also compute the positions of the\n"
"   cells at the same place. Positioning the cell text inside the cell\n"
"   is done by the following functions.  _left means left justified,\n"
"   _right means right justified etc.\n"
"\n"
"   The given y value is relative to cursor_y. It\047s not updated at this\n"
"   time.  However, the x values are absolute. The centering is also done\n"
"   outside since the canvas itself doesn\047t know anything about the size\n"
"   of the table. */\n"
"\n"
"void FYGfxCanvas::cell_left(int x1,int x2,int y,int h,int a,int s, int e)\n"
"{\n"
"   int i;\n"
"   for(i=s;i<=e;i++)\n"
"   {\n"
"     if (i!=s)\n"
"       x1+= fontS[ elts[2*i] & 0xff ];\n"
"     coords[i].x= x1; \n"
"     coords[i].y= cursor_y + y;\n"
"     coords[i].h= h;\n"
"     coords[i].a= a;\n"
"     x1+= word_width[ elts[2*i+1] ];\n"
"     max_x= max_i(max_x,x1);\n"
"   }\n"
"}\n"
"\n"
"void FYGfxCanvas::cell_center(int x1,int x2, int y,int h,int a,int s,int e)\n"
"{\n"
"   int x,d,i;\n"
"   x= x1;\n"
"   for(i=s;i<=e;i++)\n"
"   {\n"
"     if (i!=s)\n"
"       x+= fontS[ elts[2*i] & 0xff ];\n"
"     coords[i].x= x; \n"
"     coords[i].y= cursor_y + y;\n"
"     coords[i].h= h;\n"
"     coords[i].a= a;\n"
"     x+= word_width[ elts[2*i+1] ];\n"
"     max_x= max_i(max_x,x);\n"
"   }\n"
"   d= (x2-x)/2;\n"
"   for(i=s;i<=e;i++)\n"
"     coords[i].x+= d;\n"
"}\n"
"\n"
"void FYGfxCanvas::cell_right(int x1,int x2,int y,int h,int a,int s, int e)\n"
"{\n"
"   int x,d,i;\n"
"   x= x1;\n"
"   for(i=s;i<=e;i++)\n"
"   {\n"
"     if (i!=s)\n"
"       x+= fontS[ elts[2*i] & 0xff ];\n"
"     coords[i].x= x; \n"
"     coords[i].y= cursor_y + y;\n"
"     coords[i].h= h;\n"
"     coords[i].a= a;\n"
"     x+= word_width[ elts[2*i+1] ];\n"
"     max_x= max_i(max_x,x);\n"
"   }\n"
"   d= (x2-x);\n"
"   for(i=s;i<=e;i++) coords[i].x+= d;\n"
"}\n"
"\n"
"/* This function is used for computing the width/height of a cell.\n"
"   It returns the dimensions of the given text elts[start,end] \n"
"   as if it was displayed on a single row. */\n"
"\n"
"void FYGfxCanvas::get_seq_metrics(int start,int end,int *Rw,int *Rh,int *Ra)\n"
"{\n"
"  int i;\n"
"  int W,S,F;\n"
"  int w,h,a;\n"
"  w= h= a= 0;\n"
"\n"
"  create_fonts();\n"
"  for(i=start;i<=end;i++)\n"
"  {\n"
"     W= elts[2*i+1];\n"
"     S= elts[2*i];\n"
"     F= S & 0xff;\n"
"     if (i!=start) w+= fontS[F];\n"
"     if (word_width[W]==0) word_width[W]= text_width(F, words[W]);\n"
"     w+= word_width[W];\n"
"     h= max_i(h, fontH[F]);\n"
"     a= max_i(a, fontA[F]);\n"
"  }\n"
"  *Rw= w; *Ra= a; *Rh= h;\n"
"}\n"
"\n"
"int *FYGfxCanvas::get_row_lines(int nr)\n"
"{\n"
"  if (n_row_lines==nr) return row_lines;\n"
"  if (row_lines) free(row_lines);\n"
"  row_lines= NULL;\n"
"  n_row_lines= nr;\n"
"  if (n_row_lines) \n"
"    row_lines= (typeof(row_lines)) malloc(3*sizeof(row_lines[0])*nr);\n"
"  return row_lines;\n"
"}\n"
"\n"
"int *FYGfxCanvas::get_col_lines(int nr)\n"
"{\n"
"  if (n_col_lines==nr) return col_lines;\n"
"  if (col_lines) free(col_lines);\n"
"  col_lines= NULL;\n"
"  n_col_lines= nr;\n"
"  if (n_col_lines) \n"
"    col_lines= (typeof(col_lines)) malloc(3*sizeof(col_lines[0])*nr);\n"
"  return col_lines;\n"
"}\n"
"\n"
"\n"
"void FYGfxCanvas::draw_row_lines(int y1, int y2, FXDCWindow *dc)\n"
"{\n"
"  int i,j,k;\n"
"  if (!n_row_lines || !row_lines) return ;\n"
"\n"
"  i= 0; j= n_row_lines-1;\n"
"  while(i<=j)\n"
"  {\n"
"    k= (i+j)/2;\n"
"    if (row_lines[3*k] < y1) i= k+1; \n"
"    else if (row_lines[3*k] > y2) j= k-1; \n"
"    else break;\n"
"  }\n"
"  if (i>j) return ;\n"
"  dc->setForeground(colors[0]);\n"
"\n"
"  for(i=k;i>=0 && row_lines[3*i]>=y1;i--)\n"
"    dc->drawLine(row_lines[3*i+1]+pos_x, row_lines[3*i]+pos_y,\n"
"                 row_lines[3*i+2]+pos_x, row_lines[3*i]+pos_y);\n"
"\n"
"  for(j=k+1;j<n_row_lines && row_lines[3*j]<=y2;j++)\n"
"    dc->drawLine(row_lines[3*j+1]+pos_x, row_lines[3*j]+pos_y,\n"
"                 row_lines[3*j+2]+pos_x, row_lines[3*j]+pos_y);\n"
"}\n"
"\n"
"\n"
"void FYGfxCanvas::draw_col_lines(int y1, int y2, FXDCWindow *dc)\n"
"{\n"
"  int i,j,k;\n"
"  if (!n_col_lines || !col_lines) return ;\n"
"\n"
"  i= 0; j= n_col_lines-1;\n"
"  while(i<=j)\n"
"  {\n"
"    k= (i+j)/2;\n"
"    if (col_lines[3*k+1] < y1) i= k+1; \n"
"    else if (col_lines[3*k] > y2) j= k-1; \n"
"    else break;\n"
"  }\n"
"  if (i>j) return ;\n"
"  dc->setForeground(colors[0]);\n"
"\n"
"  for(i=k;i>=0 && col_lines[3*i+1]>=y1;i--)\n"
"    dc->drawLine(col_lines[3*i+2]+pos_x, col_lines[3*i]+pos_y,\n"
"                 col_lines[3*i+2]+pos_x, col_lines[3*i+1]+pos_y);\n"
"\n"
"  for(j=k+1;j<n_col_lines && col_lines[3*j]<=y2;j++)\n"
"    dc->drawLine(col_lines[3*j+2]+pos_x, col_lines[3*j]+pos_y,\n"
"                 col_lines[3*j+2]+pos_x, col_lines[3*j+1]+pos_y);\n"
"}\n"
"\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                               Images                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-img \n"
"void FYGfxCanvas::set_images(imagedef_t *id, int n)\n"
"{\n"
"  if (id==imagedefs) return ;\n"
"  destroy_images();\n"
"  n_images= n;\n"
"  if (!n_images) return ;\n"
"  imagedefs= id;\n"
"  images= (typeof(images)) calloc(n, sizeof(images[0]));\n"
"}\n"
"\n"
"\n"
"void FYGfxCanvas::image(int n)\n"
"{\n"
"  images[n].y1= cursor_y;\n"
"  cursor_y+= imagedefs[n].height;\n"
"  images[n].y2= cursor_y-1;\n"
"  images[n].x= max_i(0,(viewport_w - imagedefs[n].width)/2);\n"
"  max_x= max_i(max_x, images[n].x + imagedefs[n].width);\n"
"  max_y= cursor_y;\n"
"}\n"
"\n"
"void FYGfxCanvas::draw_images(FXDCWindow *dc, int y1, int y2)\n"
"{\n"
"  int i,j,k;\n"
"  if (!n_images) return ;\n"
"  i= 0; j= n_images-1;\n"
"  while(i<=j)\n"
"  {\n"
"    k= (i+j)/2;\n"
"    if (images[k].y1>y2) i= k+1; \n"
"    else if (images[k].y2<y1) j= k-1;\n"
"    else break;\n"
"  }\n"
"  if (i>j) return ;\n"
"  for(i=k;i>0 && images[i-1].y2>=y1;i--) ;\n"
"  for(j=k;j<n_images-1 && images[j+1].y1<=y2;j++) ;\n"
"\n"
"  for(k=i;k<=j;k++)\n"
"  {\n"
"    if (!images[k].image)\n"
"    {\n"
"      images[k].image= new FXPNGImage(FXApp::instance(), imagedefs[k].data);\n"
"      images[k].image->create();\n"
"    }\n"
"    dc->drawImage(images[k].image, images[k].x+pos_x, images[k].y1+pos_y);\n"
"  }\n"
"}\n"
"\n"
"void FYGfxCanvas::destroy_images()\n"
"{\n"
"  int i;\n"
"  for(i=0;i<n_images;i++)\n"
"     if (images[i].image)\n"
"     {\n"
"         images[i].image->detach(); \n"
"         delete images[i].image;\n"
"     }\n"
"  n_images= 0;\n"
"  if (images) free(images);\n"
"  imagedefs= NULL;\n"
"  images= NULL;\n"
"}\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                                Links                                *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-link\n"
"void FYGfxCanvas::set_links(const unsigned int* lk, int n)\n"
"{\n"
"  links= lk;\n"
"  n_links= n;\n"
"}\n"
"\n"
"int FYGfxCanvas::find_element_by_coord(int evX,int evY)\n"
"{\n"
"  int i,j,k;\n"
"\n"
"  i= 0; j= nelts-1;\n"
"  while(i<=j)\n"
"  {\n"
"    k= (i+j)/2;\n"
"    if (coords[k].y>evY) j= k-1;\n"
"    else if (coords[k].y+coords[k].h<=evY) i= k+1;\n"
"    else break;\n"
"  }\n"
"  if (i>j) return -1;\n"
"\n"
"  for(i=k;i>=0;i--)\n"
"  {\n"
"    if (coords[i].y!=coords[k].y) break;\n"
"    if (coords[i].x+word_width[elts[2*i+1]]<evX) break;\n"
"    if (coords[i].x<=evX) return i;\n"
"  }\n"
"  for(i=k+1;i<nelts;i++)\n"
"  {\n"
"    if (coords[i].y!=coords[k].y) break;\n"
"    if (coords[i].x>evX) break;\n"
"    if (coords[i].x+word_width[elts[2*i+1]]>evX) return i;\n"
"  }\n"
"  return -1;\n"
"}\n"
"\n"
"long FYGfxCanvas::onLeftBtnPress(FXObject* obj,FXSelector sel,void* ptr)\n"
"{\n"
"  FXEvent *event;\n"
"  int evX, evY;\n"
"  int k;\n"
"  int sl, el;\n"
"\n"
"  event= (FXEvent*) ptr;\n"
"  if (!event || !nelts) return 1;\n"
"  evX= event->win_x-pos_x;\n"
"  evY= event->win_y-pos_y;\n"
"\n"
"  k= find_element_by_coord(evX, evY);\n"
"  \n"
"  if (k==-1) return 1;\n"
"\n"
"  if (find_decor(k, k, &sl, &el, links, n_links)) return 1;\n"
"  if (link_func) (*link_func)(links[3*sl+2]);\n"
" \n"
"  return 1;\n"
"}\n"
"\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                              Painting                               *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-paint\n"
"\n"
"/* The disabled part below was for a delayed full-screen paint.\n"
"   It does work but it\047s ugly since the window borders moving around\n"
"   leave marks on the canvas. */\n"
"long FYGfxCanvas::onPaint(FXObject* ob,FXSelector sel,void* ptr)\n"
"{\n"
"  FXEvent* event=(FXEvent*)ptr;\n"
"#if 0\n"
"  if (0 && event->rect.x==0 && event->rect.y==0 &&\n"
"      event->rect.w>= width && event->rect.h>=height)\n"
"  {\n"
"    FXApp::instance()->addTimeout(this, 1, 250, NULL);\n"
"  } \n"
"  else \n"
"#endif\n"
"  {\n"
"    FXDCWindow dc(this,event);\n"
"    draw_everything( &dc, event->rect.x, event->rect.y,\n"
"                        event->rect.w, event->rect.h);\n"
"  }\n"
"  return 1;\n"
"}\n"
"\n"
"/* I\047m not very sure about the following functions. They find the lowest and\n"
"   heighest displayed row. This is done by a binary search.\n"
"\n"
"   The first function finds the first row which intersects with the screen.\n"
"   i.e. y<scr y+h>=scr. */\n"
"int FYGfxCanvas::find_lower(int scr)\n"
"{\n"
"  int i,j,k,y1,y2;\n"
"  i=0; j= nelts-1;\n"
"  while(i<=j && coords[i].y!=coords[j].y)\n"
"  {\n"
"    k= (i+j)/2;\n"
"    y1= coords[k].y; y2= coords[k].y+coords[k].h;\n"
"    if (y1<=scr)\n"
"    { \n"
"       if (y2>=scr) goto done;\n"
"       i= k+1;\n"
"       continue;\n"
"    }\n"
"    j= k-1;\n"
"  }\n"
"  if (i>j) return 0;\n"
"  k= i;\n"
"done:\n"
"  while(k>0 && coords[k-1].y==coords[k].y) k--;\n"
"  return k;\n"
"}\n"
"\n"
"\n"
"int FYGfxCanvas::find_higher(int scr)\n"
"{\n"
"  int i,j,k,y1,y2;\n"
"  i=0; j= nelts-1;\n"
"  while(i<=j && coords[i].y!=coords[j].y)\n"
"  {\n"
"    k= (i+j)/2;\n"
"    y1= coords[k].y; y2= coords[k].y+coords[k].h;\n"
"    if (y1<=scr)\n"
"    { \n"
"       if (y2>=scr) goto done;\n"
"       i= k+1;\n"
"       continue;\n"
"    }\n"
"    j= k-1;\n"
"  }\n"
"  if (i>j) return nelts-1;\n"
"  k= j;\n"
"done:\n"
"  while(k<nelts-1 && coords[k+1].y==coords[k].y) k++;\n"
"  return k;\n"
"}\n"
"\n"
"void FYGfxCanvas::draw_everything\n"
"  (FXDCWindow *dc, int evX,int evY, int evW,int evH)\n"
"{\n"
"  FXColor white;\n"
"  int se, ee,e;\n"
"  unsigned int style, word;\n"
"\n"
"  white= FXRGB(0xff,0xff,0xff);\n"
"#if 0\n"
"  static unsigned int K;\n"
"  static int C[4];\n"
"  C[0]= FXRGB(0x00,0xff,0x00);\n"
"  C[1]= FXRGB(0x00,0x00,0xff);\n"
"  C[2]= FXRGB(0xff,0x00,0xff);\n"
"  C[3]= FXRGB(0x33,0x33,0x33);\n"
"  K= (K+1)%4;\n"
"  white= C[K];\n"
"#endif\n"
"\n"
"  dc->setForeground(white);\n"
"  dc->fillRectangle(evX, evY, evW, evH);\n"
"  if (!coords || !elts || !words) return ;\n"
"\n"
"  create_fonts();\n"
"\n"
"  evY-= pos_y; evX-= pos_x;\n"
"\n"
"  se= find_lower( evY );\n"
"  ee= find_higher(evY+evH-1);\n"
"\n"
"  // printf(\042y1= %d y2= %d se= %d ee= %d\134n\042, evY, evY+evH-1, se, ee);\n"
"\n"
"  //se= 0; ee= nelts-1;\n"
"  draw_vboxes(dc, se,ee);\n"
"  draw_bgrects(dc, se,ee);\n"
"\n"
"  draw_row_lines(evY, evY+evH-1, dc);\n"
"  draw_col_lines(evY, evY+evH-1, dc);\n"
"\n"
"  if (0) { int i; printf(\042[PAINT]\042); \n"
"           for(i=se;i<=ee;i++) printf(\042%s\042,words[elts[2*i+1]]); \n"
"           printf(\042\134n\042); }\n"
"\n"
"  for(e=se;e<=ee;e++)\n"
"  {\n"
"     word= elts[e*2+1];\n"
"     style= elts[e*2];\n"
"     dc->setForeground(colors[(style>>8)&0xff]);\n"
"     dc->setFont(fonts[style&0xff]);\n"
"     dc->drawText(coords[e].x + pos_x,\n"
"                 coords[e].y+ coords[e].a + pos_y, \n"
"                 words[word], strlen(words[word])); \n"
"  }\n"
"\n"
"  draw_underlines(dc, se,ee);\n"
"  draw_images(dc, evY, evY+evH-1);\n"
"}\n"
"\n"
"/***********************************************************************\n"
" *                                                                     *\n"
" *                              Resizing                               *\n"
" *                                                                     *\n"
" ***********************************************************************/\n"
"// uu-resize\n"
"\n"
"/* When we resize the widget, we will need to lay the text out again. This\n"
"   will be done by the function which did the initial layout. We simply\n"
"   start from scratch. So, we store the function somewhere and then call\n"
"   it when a resize occurs. However, we shouldn\047t do re-layouts all the\n"
"   time while the window is being resized. They discussed this in some\n"
"   email list and Jeroen suggested a timer. I\047ll try that. \n"
"\n"
"   It\047s not necessary to catch the resize event. It\047s OK if we simply\n"
"   override the layout method. If we don\047t have a resize function yet,\n"
"   we simply don\047t do anything since the widget is empty.\n"
"\n"
"   Otherwise, we avoid laying out by starting a timer. We only do the\n"
"   re-layout when the timer expires. */\n"
"\n"
"long FYGfxCanvas::on_layout_timeout(FXObject* ob,FXSelector sel,void* ptr)\n"
"{\n"
"  if (!resize_func) return 1;\n"
"  (*resize_func)(this);\n"
"  update();\n"
"  return 1;\n"
"}\n"
"\n"
"\n"
"void FYGfxCanvas::layout()\n"
"{\n"
"  if (!resize_func) goto done;\n"
"  // 250 msecs..\n"
"  FXApp::instance()->addTimeout(this, 0, 250, NULL);\n"
"done:\n"
"  FXScrollArea::layout();\n"
"}\n"
"\n"
"void FYGfxCanvas::reset_doc()\n"
"{\n"
"  cursor_y= max_y= max_x= 0;\n"
"}\n"
"\n"
"void FYGfxCanvas::end_doc(void (*f)(FYGfxCanvas*))\n"
"{\n"
"  resize_func= f;\n"
"  FXScrollArea::layout();\n"
"  update();\n"
"}\n"
"/*\n"
"  The help window will contain the following:\n"
"   - a menu bar for navigation\n"
"   - a toolbar for displaying buttons such as prev,next, search, hprev, etc.\n"
"   - a canvas to display the help text\n"
"*/\n"
"\n"
"\n"
"namespace FX\n"
"{\n"
"\n"
"typedef struct { const char *Vs; int Vi; } sipair_t;\n"
"\n"
"class FXAPI FYHelpWin : public FXTopWindow\n"
"{\n"
"  FXDECLARE(FYHelpWin)\n"
"protected:\n"
"  FYHelpWin();\n"
"private:\n"
"  FYHelpWin(const FYHelpWin&);\n"
"  FYHelpWin &operator=(const FYHelpWin&);\n"
"public:\n"
"  FYHelpWin (FXApp* ap,const FXString& name, FXIcon *ic,FXIcon *mi,\n"
"            FXuint opts, FXint w,FXint h,\n"
"            FXint x=0,FXint y=0,\n"
"            FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,\n"
"            FXint hs=0,FXint vs=0) ;\n"
"  virtual ~FYHelpWin();\n"
"\n"
"// uu-chapsec\n"
"/* Chapters and sections will be accessible thru menus. */\n"
"private:\n"
"  FXMenuPane *chapters_pane;\n"
"  FXMenuPane *sections_pane;\n"
"  FXMenuTitle *sections_mtitle;\n"
"\n"
"/* The chapter menu will never change. We don\047t need to keep track of\n"
"   that stuff. We simply provide a method to record them into the menu. */\n"
"\n"
"public:  void menu_add_chapter(const char *title, int number);\n"
"\n"
"/* The section menu will be set as a whole, rather than incremental \n"
"   method calls. */\n"
"private: const sipair_t *sections_def;\n"
"public:  void set_section_menu(const sipair_t *);\n"
"\n"
"public:\n"
"  FYGfxCanvas *canvas;\n"
"};\n"
"}\n"
"\n"
"using namespace FX;\n"
"\n"
"static void follow_link(int);\n"
"\n"
"FXDEFMAP(FYHelpWin) FYHelpWinMap[]=\n"
"{\n"
"};\n"
"\n"
"\n"
"FXIMPLEMENT(FYHelpWin,FXTopWindow,FYHelpWinMap, ARRAYNUMBER(FYHelpWinMap))\n"
"\n"
"FYHelpWin::FYHelpWin(){ flags|=FLAG_ENABLED|FLAG_SHOWN; }\n"
"\n"
"\n"
"FYHelpWin::FYHelpWin\n"
"           (FXApp* ap,const FXString& name, FXIcon *ic,FXIcon *mi,\n"
"            FXuint opts, FXint w,FXint h,\n"
"            FXint x,FXint y,\n"
"            FXint pl,FXint pr,FXint pt,FXint pb,\n"
"            FXint hs,FXint vs) :\n"
" FXTopWindow(ap,name,ic,mi, opts, x,y,w,h, pl,pr,pt,pb, hs,vs)\n"
"{\n"
"  FXVerticalFrame *vbox;\n"
"  flags|=FLAG_ENABLED|FLAG_SHOWN; \n"
"\n"
"  vbox= new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);\n"
"\n"
"  FXMenuBar *bar;\n"
"  bar= new FXMenuBar(vbox, LAYOUT_FILL_X);\n"
"\n"
"  chapters_pane= new FXMenuPane(this);\n"
"  sections_pane= new FXMenuPane(this);\n"
"\n"
"  new FXMenuTitle(bar,\042Chapters\042, NULL, chapters_pane);\n"
"  sections_mtitle= new FXMenuTitle(bar,\042Sections\042, NULL, sections_pane);\n"
"\n"
"  canvas= new FYGfxCanvas(vbox, FRAME_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_Y);\n"
"  \n"
"  canvas->define_font(0, \042Dejavu Sans\042, 12, \042\042);\n"
"  canvas->define_font(4, \042Dejavu Sans\042, 24, \042B\042);\n"
"  canvas->define_font(3, \042Dejavu Sans\042, 20, \042B\042);\n"
"  canvas->define_font(2, \042Dejavu Sans\042, 16, \042B\042);\n"
"  canvas->define_font(1, \042Dejavu Sans\042, 13, \042B\042);\n"
"  canvas->define_font(5, \042Mono\042, 12, \042\042);\n"
"  canvas->define_font(6, \042Mono\042, 12, \042\042);\n"
"\n"
"  canvas->define_color(0, 0);\n"
"  canvas->define_color(1, 0xff0000);\n"
"  canvas->define_color(2, 0x00ff00);\n"
"  canvas->define_color(3, 0x0000ff);\n"
" \n"
"//uu-cons\n"
"}\n"
"\n"
"\n"
"FYHelpWin::~FYHelpWin()\n"
"{\n"
"// uu-des\n"
"}\n"
"\n"
"// uu-chapsec\n"
"void FYHelpWin::menu_add_chapter(const char *title, int linkno)\n"
"{\n"
"  FYMenuCommand *cmd;\n"
"  cmd= new FYMenuCommand(chapters_pane, title, NULL, 0, follow_link, linkno);\n"
"  cmd->create();\n"
"}\n"
"\n"
"\n"
"void FYHelpWin::set_section_menu(const sipair_t *me)\n"
"{\n"
"  int i;\n"
"  FYMenuCommand *cmd;\n"
"  if (sections_def==me) return ;\n"
"  FXWindow *ww;\n"
"  while((ww=sections_pane->getFirst()))\n"
"    delete ww;\n"
"  sections_def= me;\n"
"  for(i=0;me[i].Vs;i++)\n"
"  {\n"
"     cmd= new FYMenuCommand(sections_pane, me[i].Vs, \n"
"                            NULL, 0, follow_link, me[i].Vi);\n"
"     cmd->create();\n"
"  }\n"
"  if (i==0) \n"
"    sections_mtitle->disable();\n"
"  else\n"
"    sections_mtitle->enable();\n"
"  sections_pane->recalc();\n"
"}\n"
"\n"

;


typedef struct elt {
  int kind;
  char *txt;
  char *arg;
  struct elt *children;
  struct elt *prev,*next;
} elt_t;


typedef struct vline {
  struct vline *next;
  elt_t *elts;
} vline_t;


typedef struct {
  int col, row, colspan;
  char *fmt; elt_t *seq;
} cell_t;


typedef struct docnode
{
  int kind;
  struct docnode *prev,*next;
  elt_t *elts;
  int level; // for sections and lists

  int s_number;          // section number
  char *s_title;         // section title
  char *s_title_encoded; // above in encoded form, with quotes and everything.
        // the following two are meaningful only for subsections.
  int s_parentno;        // number of the parent section
  int s_eltstart;        // the element number starting this subsection.

  vline_t *vlines;
  char *vcmd;

  int t_ncells, t_nrows, t_ncols;
  cell_t *t_cells;

  int i_width, i_height;
  char *i_path;

  char *a_txt;         // anchor text

  int lno; char* fname;
} docnode_t;



typedef struct { int n_val; char *str; int* val; } word_record_t;



typedef struct { int y1,y2; int x; } tricor_y;


typedef struct imageref 
   { struct imageref *next; int number,width,height;
            char *path; } imageref_t;


typedef struct { int secnum; char *name; int lno; char *fname; 
                 docnode_t *node; } aanchor_t;


void parse_file(char *name);
int read_line();
void parse_top();
int classify();
void parse_section_header();
void normalize_spaces(uint8_t *str);
void remove_trailing_spc(uint8_t *str);
void remove_part(uint8_t *str, int start, int end);
void parse_paragraph();
int skipspc(uint8_t *str, int i);
int ispc(uint32_t K);
elt_t *parse_seq(uint8_t *str,int cmds);
char *get_part(char *str, int start, int end);
char *get_remove_part(char *x, int start, int end);
void remove_leading_spc(char *x);
void print_docnode(docnode_t *N);
void print_docnodelist(docnode_t *head);
void print_seq(elt_t *E);
void add_section(int level, char *txt);
void add_paragraph(char *txt);
void add_node(docnode_t *N);
void parse_verbatim();
void add_verbatim(char *cmd, vline_t *V);
elt_t *parse_verbatim_line(char *x,char *quot);
elt_t *seq_add_elt(elt_t *head, int kind);
void parse_list();
void add_list(int level,char *txt);
static int escape_seq(char *str);
static void replace_escape_seqs(char *str);
int strpfx(char *pfx, char *k);
void parse_table();
void parse_table_content(char *str);
static int get_numeric(char *txt, int V);
void section_style(int level, int *style, int *font);
void inline_style(char *cmd, int *Rstyle, int *Rfont);
void print_str(unsigned char *x);
void output_seq(int default_style, elt_t *head,docnode_t *ref);
void output_verbatim(char *cmd, vline_t *VL);
void verbatim_style(char *cmd, int *style, int *font);
void verbatim_inline_style(char *cmd,int *Rstyle, int *Rfont);
void output_list(docnode_t *N);
void print_init_words();
void page_init();
void output_elt(int style, int word);
void print_seq_buffer();
void print_command_buffer();
void coprint(const char *fmt, ...);
void print_bgrects();
void print_decor(char *varname, char *method, buffer_t **Rbuffer, int *Rn);
void record_decor(int style, int cbegin, int cend);
void record_decor_entry(buffer_t **Rbuffer, int *N, int B,int E, int C);
void output_doc();
void print_page_functions(docnode_t *secstart);
void output_node(docnode_t *N);
void print_link_table();
void print_section_menu(docnode_t *secstart);
void output_table(int nrows,int ncols, int ncells, 
                  cell_t *cells,docnode_t *ref);
const char *cell_just_func(char *fmt);
void print_table_coord_variables();
void table_draw_row_lines(int nrows, int ncols);
void table_draw_col_lines(int nrows, int ncols, int ncells, cell_t *cells);
void sort_tricor_y(tricor_y *A, int n);
int compar_tricor_y(const void *A, const void *B);
void parse_image();
char *relapath(char *base, char *name);
void print_image_file(int num, char *path);
void output_image(docnode_t *N);
void print_imagedefs();
void get_chapter_menu();
void check_first_node();
void calculate_section_titles();
char *collect_seq_str(elt_t *elts);
void print_section_menu_arrays();
void buffer_printv(buffer_t *buffer, int fmt, ...);
unsigned char *encode_str(unsigned char *src);
void remove_all_spaces(unsigned char *z);
void collect_anchors();
int anchor_compare(const void *a, const void *b);
void print_anchors();
void parse_anchor();
void output_link(elt_t *linkel,docnode_t *ref);
void record_link(int start, int end, int num);
void print_links();
aanchor_t *find_target(char *tgt);
int lookup_word(int font, const char *str);
char **get_word_list();

uint8_t *line;
size_t line_len;
int line_number;
char *file_name;
ifile_t *input;

  static int secnum;


static docnode_t *doc;
static char *classify_cmd;

    static int seqlen;
    static buffer_t *seqbuffer;

    static buffer_t *combuffer;

    static buffer_t *bgbuffer,*underbuffer,*verbox;
    static int nbg,nunder,nverbox;


static strmap_t word_list;
static int last_word;


int n_table_row_lines, n_table_col_lines;

  static int inum;

    static imageref_t *imgs, **imgtail;
  char *chapter_menu;

  aanchor_t *anchors; int n_anchors;

    buffer_t *linkbuf; int n_links;

void parse_file(char *name)
{
  if (ifile_open(&input,name))
     exit(fprintf(stderr,"%s\n", ifile_error(input)));
  file_name= name;
  if (read_line()) return ;
  parse_top();
  ifile_close(input);
}

int read_line()
{
  if (line) free(line); line= NULL;
  if (ifile_get_line(input, &line, &line_len, 1))
    return 1;
  line_number= ifile_line_number(input);
  return 0;
}

void parse_top()
{
 while(1)
 {
  switch(classify())
  {
  case 'I': parse_image(); break;
  case 'L': parse_list(); continue;
  case 'S': parse_section_header(); break;
  case 'E': break;
  case 'V': parse_verbatim(); continue;
  case 'T': parse_table(); break;
  case 'A': parse_anchor(); break;
  case '.': return ;
  default:  parse_paragraph(); continue;
  }
  if (read_line()) return ;
 }
}

int classify()
{
  int i;
  if (!line) return '.';
  i= skipspc(line, 0);
  if (!line[i]) return 'E';
  if (line[i]=='-') return 'L';
  if (line[i]=='=') return 'S';
  if (line[i]=='[')
  {
    int s;
    char *cmd;
    i++;
    s= skipspc(line,i);
    for(i=s;line[i]&&!ispc(line[i])&&line[i]!=']';i++) ;
    cmd= get_part(line, s,i);
    if (!strcmp(cmd,"code")) { classify_cmd= cmd; return 'V'; }
    if (!strcmp(cmd,"table")) { classify_cmd= cmd; return 'T'; }
    if (!strcmp(cmd,"end")) { classify_cmd= cmd; return 'Z'; }
    if (!strcmp(cmd,"image")) { classify_cmd= cmd; return 'I'; }
    if (!strcmp(cmd,"anchor")) { classify_cmd= cmd; return 'A'; }
    free(cmd);
  }
  return 'P';
}

void parse_section_header()
{
  int i;
  int seclevel;
  i= skipspc(line, 0);
  seclevel= 0;
  while(line[i]=='=') { i++; seclevel++; }
  remove_part(line, 0, i);
  normalize_spaces(line);
  if (line[0]==0) 
    exit(fprintf(stderr, "near %s:%d: empty section header\n",
                          file_name, line_number));
  add_section(seclevel, line);
}


void normalize_spaces(uint8_t *str)
{
  remove_leading_spc(str);
  remove_trailing_spc(str);
}

void remove_trailing_spc(uint8_t *str)
{
  size_t L;
  L= strlen(str);
  while(L>0 && ispc(str[L-1])) str[--L]= 0; 
}

void remove_part(uint8_t *str, int start, int end)
{
  if (start<end)
     memmove(str+start, str+end, strlen(str+end)+1);
}

void parse_paragraph()
{
  buffer_t *B;
  char *str;
  size_t len;

  B= buffer_new(128);
  while(1)
  {
    buffer_write(B, line, strlen(line));
    buffer_write(B, " ", 1);
    if (read_line()) break;
    switch(classify())
    {
    case 'A':
    case '.': case 'I': case 'L': case 'V': case 'E': case 'S': goto done;
    }
  }
done:
  buffer_write(B, "", 1);
  buffer_free(B, (void**) &str, &len);  
  normalize_spaces(str);
  add_paragraph(str);
  free(str);
}

int skipspc(uint8_t *str, int i)
{
  while(ispc(str[i])) i++;
  return i;
}

int ispc(uint32_t K)
{
  return (K>0 && K<=' ') || (K==127);
}


elt_t *parse_seq(uint8_t *str,int cmds)
{
  int i,k;
  char *cmd;
  elt_t *seq,*E;
  seq= malloc(sizeof(*seq));
  seq->prev= seq->next= seq;

again:
  remove_leading_spc(str);
  if (str[0]==0) return seq;
  if (!cmds || str[0]!='[') goto normal_word;
  if (escape_seq(str)) goto normal_word;
  i=1;
  k=1;
nextchr:
  switch(str[i])
  {
  case 0: goto unmatched;
  case '[': k++; break;
  case ']': k--; if (k==0) goto done;
            break;
  case '\\': i++; if (str[i]==0) goto unmatched;
             break;
  default:  break;
  }
  i++;
  goto nextchr;
done:
  cmd= get_part(str, 1, i);
  i++;
  remove_part(str, 0,i);
  E= calloc(1, sizeof(*E));
  E->kind= 'C';

  remove_part(cmd, 0,skipspc(cmd,0));
  if (cmd[0]==0) goto invalid;
  for(i=0;cmd[i] && !ispc(cmd[i]);i++) ;
  E->txt= get_remove_part(cmd, 0,i);
  remove_leading_spc(cmd);
  if (!strcmp(E->txt,"link"))
  {
    E->arg= cmd;
  } else {
    if (cmd[0]!=0) E->children= parse_seq(cmd, 0);
    free(cmd);
  }
  goto insert;

insert:
  E->prev= seq->prev; E->next= seq;
  seq->prev->next= E; seq->prev= E;
  goto again;
invalid:
  fprintf(stderr,"near %s:%d: invalid command sequence\n", 
                  ifile_file_name(input), ifile_line_number(input));
  exit(1);
unmatched:
  fprintf(stderr,"near %s:%d: unmatched '['\n", 
                  ifile_file_name(input), ifile_line_number(input));
  exit(1);

normal_word:
  for(i=0;str[i] && !ispc(str[i]);i++)  ;
  E= calloc(1,sizeof(*E));
  E->kind= 'W';
  E->txt= get_remove_part(str, 0,i);
  replace_escape_seqs(E->txt);
  goto insert;
}


char *get_part(char *str, int start, int end)
{
  char *T;
  int L;
  L= end-start;
  T= malloc(L+1);
  memcpy(T, str+start, L);
  T[L]= 0;
  return T;
}

char *get_remove_part(char *x, int start, int end)
{
  char *R;
  R= get_part(x,start,end);
  remove_part(x,start,end);
  return R;
}

void remove_leading_spc(char *x)
{
  remove_part(x, 0,skipspc(x,0));
}


void print_docnode(docnode_t *N)
{
  switch(N->kind)
  {
  case 'P': printf("PAR["); print_seq(N->elts); printf("]\n"); break;
  case 'S': printf("SEC(%d)[",N->level); print_seq(N->elts);
            printf("]\n"); break;
  }
}

void print_docnodelist(docnode_t *head)
{
  docnode_t *N;
  for(N=head->next;N!=head;N=N->next)
     print_docnode(N);
}

void print_seq(elt_t *E)
{
  elt_t *X;
  for(X=E->next;X!=E;X=X->next)
  {
   switch(X->kind)
   {
   case 'W': printf("W(%s)", X->txt); break;
   case 'C': printf("C(%s)", X->txt); 
             if (X->arg) printf("{%s}",X->arg);
             printf("[");
             if (X->children) print_seq(X->children);
             printf("]");
             break;
   }
  }
}


void add_section(int level, char *txt)
{
  docnode_t *N;
  N= calloc(1,sizeof(*N));
  N->kind= 'S';
  N->level= level;
  N->elts= parse_seq(txt,0);
  N->s_number= secnum++;
  add_node(N);
}
void add_paragraph(char *txt)
{
  docnode_t *N;
  N= calloc(1,sizeof(*N));
  N->kind= 'P';
  N->elts= parse_seq(txt,1);
  add_node(N);
}

void add_node(docnode_t *N)
{
  if (!doc)
  {
    doc=calloc(1,sizeof(*doc));
    doc->prev= doc->next= doc;
  }
  N->fname= file_name;
  N->lno= line_number;
  N->prev= doc->prev;
  N->next= doc;
  doc->prev->next= N;
  doc->prev= N;
}


void parse_verbatim()
{
  int i;
  vline_t **tail,*VL,*first;
  char *quot;
  char *cmd;

  remove_leading_spc(line);
  remove_part(line,0,1);  // skip the '['
  for(i=0;line[i] && !ispc(line[i]);i++) ; // skip the command, it's already
                                           // in classify_cmd
  remove_part(line, 0,i);
  cmd= strdup(classify_cmd);
  remove_leading_spc(line);
  if (line[0]==0 || line[0]==']' || line[1]==0) quot=strdup("[]");
  else { quot= strdup(line); quot[2]=0; }

  first= NULL;
  tail= &first;
  while(1)
  {
    if (read_line()) 
       exit(fprintf(stderr,"near %s:%d: unexpected end of file in verbatim\n",
                               file_name, line_number));
    if (classify()=='Z') break;
    VL= malloc(sizeof(*VL));
    VL->elts= parse_verbatim_line(line,quot);
    VL->next= NULL;
    *tail= VL;
    tail= &VL->next;
  }
  read_line(); // skip the 'Z' line.

  add_verbatim(cmd, first);
}


void add_verbatim(char *cmd, vline_t *V)
{
  docnode_t *N;
  N= calloc(1,sizeof(*N));
  N->vcmd= cmd;
  N->vlines= V;
  N->kind= 'V';
  add_node(N);
}


elt_t *parse_verbatim_line(char *x,char *quot)
{
  int i;
  char *s;
  elt_t *seq,*E;
  seq= malloc(sizeof(*seq));
  seq->prev= seq->next= seq;
  s= x;

again:
  if (*x==0 || *x=='\n') goto done;

  for(s=x;s[0]!=0 && s[0]!='\n' && s[0]!=quot[0];s++)  ;
  
  if (s!=x)
  {
     E= seq_add_elt(seq, 'W');
     E->txt= get_remove_part(x, 0, s-x);
     goto again;
  }
  if (s[0]!=quot[0]) goto done;

  remove_part(x,0,1); // remove the quot character
  remove_leading_spc(x);
  for(i=0;x[i]!=quot[1] && x[i]!=0 && !ispc(x[i]);i++) ;

  E=seq_add_elt(seq, 'C');
  E->txt= get_remove_part(x, 0, i);
  if (E->txt[0]==0) 
      exit(fprintf(stderr,"near %s:%d: empty command in verbatim\n",
                           file_name, line_number));
  E->children= malloc(sizeof(E->children[0]));
  E->children->prev= E->children->next= E->children;

  remove_leading_spc(x);
  for(i=0;x[i]!=quot[1] && x[i]!=0 && x[i]!='\n';i++) ;
  if (x[i]=='\n' || x[i]==0)
      exit(fprintf(stderr,"near %s:%d: unterminated command in verbatim\n",
                           file_name, line_number));
  
  E= seq_add_elt(E->children, 'W');
  E->txt= get_part(x,0,i);
  remove_part(x, 0,i+1);
  goto again;

done:
  return seq;
}

elt_t *seq_add_elt(elt_t *head, int kind)
{
  elt_t *E;
  E= calloc(1,sizeof(*E));
  E->kind= kind;
  E->prev= head->prev; E->next= head;
  head->prev->next= E; head->prev= E;
  return E;
}


void parse_list()
{
  int i;
  int level;
  buffer_t *B;
  char *str;
  size_t len;

  remove_leading_spc(line);
  for(level=0;line[level]=='-';level++)
    ;
  remove_part(line,0,level);
  remove_leading_spc(line);
  if (line[0]==0) 
  {
    read_line();
    return ;
  }
  
  if (level==0) level= 1;
  if (level>5)  level= 5;

  B= buffer_new(128);

  while(1)
  {
    buffer_write(B, line, strlen(line));
    buffer_write(B, " ", 1);
    if (read_line()) break;
    switch(classify())
    {
    case 'A': case '.':
    case 'I': case 'T': case 'L': case 'V': case 'E': case 'S': goto done;
    }
    if (skipspc(line,0)==0) goto done;
  }
done:
  buffer_write(B, "", 1);
  buffer_free(B, (void**) &str, &len);  
  normalize_spaces(str);
  add_list(level,str);
  free(str);
}

void add_list(int level,char *txt)
{
  docnode_t *N;
  N= calloc(1,sizeof(*N));
  N->kind= 'L';
  N->elts= parse_seq(txt,1);
  N->level= level;
  add_node(N);
}


static int escape_seq(char *str)
{
  if (strpfx("[pLs]",str)) return '[';
  if (strpfx("[pRs]",str)) return ']';
  if (strpfx("[pLr]",str)) return '(';
  if (strpfx("[pRr]",str)) return ')';
  return 0;
}

static void replace_escape_seqs(char *str)
{
  int i,k;
  while(*str)
  {
    k= escape_seq(str);
    if (k)
    { 
      for(i=0;str[i];i++) if (str[i]==']') break;
      if (!str[i]) return ;
      remove_part(str,1,i+1);
      str[0]= k;
    }
    str++;
  } 
}

int strpfx(char *pfx, char *k)
{
  int i;
  for(i=0;pfx[i];i++)
    if (k[i]!=pfx[i]) return 0;
  return 1;
}


void parse_table()
{
  buffer_t *B;
  char *str;
  size_t len;
  int start_lno;

  start_lno= line_number;
  B= buffer_new(128);
again:
  if (read_line()) goto done;
  switch(classify())
  {
  case 'Z': goto done;
  case '.': exit(fprintf(stderr,"near %s:%d: unterminated table\n",
                                 file_name, start_lno));
  }
  buffer_write(B, line, strlen(line));
  buffer_write(B, " ", 1);
  goto again;

done:
  buffer_write(B, "", 1);
  buffer_free(B, (void**) &str, &len);  
  parse_table_content(str);
  free(str);
}

void parse_table_content(char *str)
{
  buffer_t *cb;
  cell_t C; 
  docnode_t *N;
  int i,k;
  char *cellstr;

  int ncells, nrows, ncols, col;
  cb= buffer_new(128);
  ncells= nrows= ncols= col= 0;

again:
  remove_leading_spc(str);
  if (str[0]==0) goto done;
  if (str[0]=='.') { nrows++; col= 0; remove_part(str,0,1); goto again; }
  if (str[0]!='[') 
     exit(fprintf(stderr,"near %s:%d: garbage within table data\n",
                         file_name,line_number));
  i= 1;
  k= 1;
  i++;
next:
  switch(str[i])
  {
  case 0: exit(fprintf(stderr,"near %s:%d: unterminated table entry\n",
                          file_name, line_number));
  case '[': k++; i++; goto next;
  case ']': k--; if (k==0) goto found; 
  default: i++; goto next;
  }
found:
  cellstr= get_part(str, 1, i);
  remove_part(str, 0, i+1);
  for(i=0;cellstr[i];i++) if (cellstr[i]=='+') break;
  if (cellstr[i])
     C.fmt= get_remove_part(cellstr, 0,i+1);
  else
     C.fmt= NULL;
  C.seq= parse_seq(cellstr,1);
  C.col= col;
  C.row= nrows;
  C.colspan= get_numeric(C.fmt,1);
  if (C.colspan<1) C.colspan= 1;
  col+= C.colspan;
  if (col>ncols) ncols= col;
  buffer_write(cb, &C, sizeof(C));
  ncells++;
  goto again;
done:
  nrows++;
  N= malloc(sizeof(*N));
  N->kind= 'T';
  buffer_free(cb, (void**) &N->t_cells, NULL);
  N->t_ncells= ncells;
  N->t_nrows= nrows;
  N->t_ncols= ncols;
  add_node(N);
}

static int get_numeric(char *txt, int V)
{
  int i,j;
  char *z;
  if (!txt) return V;
  for(i=0;txt[i];i++) if (isdigit(txt[i])) break;
  if (!txt[i]) return V;
  for(j=i;txt[j];j++) if (!isdigit(txt[j])) break;
  z= get_remove_part(txt,i,j);
  V= atoi(z);
  free(z);
  return V;
}



void section_style(int level, int *style, int *font)
{
  if (level>4) level= 4;
  if (level<1) level= 1;
  *font= level;
  *style= level;
}

void inline_style(char *cmd, int *Rstyle, int *Rfont)
{
  int fg, bg, font,ul;
  fg= bg= font= ul= 0;
  if (!strcmp(cmd,"prg"))
  {
    font= 5;
    fg= 1;
  }
  if (!strcmp(cmd,"red"))
  {
    bg= 1;
  }
  if (!strcmp(cmd,"ul"))
  {
    ul= 1;
  }
  if (!strcmp(cmd,"link"))
  {
    ul= 1;
    fg= 3;
  }
  
  *Rfont= font;
  *Rstyle= (ul<<24) | (bg<<16) | (fg<<8) | font;
}




void print_str(unsigned char *x)
{
  printf("\"");
again:
  while(isalnum(*x)) { printf("%c",*x); x++; }
  if (!*x) goto done;
  printf("\\%03o",*x);
  x++;
  goto again;
done:
  printf("\"");
}

void output_seq(int default_style, elt_t *head,docnode_t *ref)
{
  unsigned int style, font;
  elt_t *E,*Z;
  int cbegin, cend;
  for(E=head->next;E!=head;E=E->next)
  {
    if (E->kind=='W') 
    {
      output_elt(default_style, lookup_word(default_style&0xff, E->txt));
      continue;
    }
    if (!strcmp(E->txt,"link")) { output_link(E, ref); continue; }
    if (!E->children || E->children->next==E->children) continue;
    inline_style(E->txt, &style, &font);
    cbegin= seqlen;
    for(Z=E->children->next;Z!=E->children;Z=Z->next)
        output_elt(style, lookup_word(font, Z->txt));
    cend= seqlen-1;
    record_decor(style, cbegin, cend);
  }
}


void output_verbatim(char *cmd, vline_t *VL)
{
  char buf[2000];
  int style, font;
  int def_style, def_font;
  int bs, be;
  verbatim_style(cmd, &def_style, &def_font);
  bs= seqlen;

  coprint("  canvas->verbatim_vspace();\n");
  for(;VL;VL=VL->next)
  {
     elt_t *E;
     int st,en;
     if (VL->elts->next==VL->elts) { // empty line 
         coprint("  canvas->pbreak();\n");
         continue;
     } 
     st= seqlen;
     for(E=VL->elts->next;E!=VL->elts;E=E->next)
     {
       if (E->kind=='W') 
       {
         output_elt( def_style, lookup_word(def_font, E->txt));
         continue;      
       }
       verbatim_inline_style(E->txt, &style, &font);
       if (E->children->next==E->children) continue;
       output_elt( style, lookup_word(font,E->children->next->txt));
       record_decor(style, seqlen-1, seqlen-1);
     }
     en= seqlen-1;
     sprintf(buf,"  canvas->verbatim_line(%d,%d);\n", st, en);
     buffer_write(combuffer, buf, strlen(buf));
  }
  coprint("  canvas->verbatim_vspace();\n");
  be= seqlen-1;
  if (bs<=be)
     record_decor_entry(&verbox, &nverbox, bs, be, (def_style>>16)&0xff);
}

void verbatim_style(char *cmd, int *style, int *font)
{
  *font= 6;
  *style= *font | (2<<16) ;
}

void verbatim_inline_style(char *cmd,int *Rstyle, int *Rfont)
{
  int fg, bg, font,ul;
  fg= bg= font= ul= 0;
  font= 6;
  if (!strcmp(cmd,"red"))
  {
    bg= 1;
  }
  if (!strcmp(cmd,"ul"))
  {
    ul= 1;
  }
  if (!strcmp(cmd,"link"))
  {
    ul= 1;
    fg= 3;
  }
  
  *Rfont= font;
  *Rstyle= (ul<<24) | (bg<<16) | (fg<<8) | font;
}


void output_list(docnode_t *N)
{
  int start;
  start= seqlen;
  output_elt(0,0);
  output_seq(0,N->elts,N);
  coprint("  canvas->bullet_println(%d, %d, %d);\n",
                 N->level, start, seqlen-1);
}


void print_init_words()
{
  char **words;
  int nw;

  words=get_word_list();
  printf("static void init_words(FYGfxCanvas *canvas) {\n");
  printf("static const char *words[]={\n");
  for(nw=0;words[nw];nw++) { print_str(words[nw]); printf(",\n"); }
  printf("};\n");
  printf("  canvas->set_words(words,%d);\n",nw);
  printf("}\n");
}

void page_init()
{
    seqlen= 0; seqbuffer= buffer_new(128);

    combuffer= buffer_new(1024);

    nbg= nunder= nverbox= 0;
    bgbuffer= underbuffer= verbox= NULL;

   n_table_row_lines= n_table_col_lines= 0;

    inum= 0;  

    imgs= NULL; imgtail= &imgs; 

    linkbuf= NULL; n_links= 0;

}


void output_elt(int style, int word)
{
  buffer_write(seqbuffer, &style, sizeof(style));
  buffer_write(seqbuffer, &word, sizeof(word));
  seqlen++;
}

void print_seq_buffer()
{
  uint32_t *seq;
  int i;

  buffer_free(seqbuffer, (void**) &seq, NULL); 

  printf("static const unsigned int seq[]={\n");
  for(i=0;i<seqlen*2;i++) printf("0x%x,", seq[i]);
  printf("};\n");

  printf("  canvas->set_elts(seq,%d);\n", seqlen);
  seqbuffer= NULL;
  seqlen= 0;
  free(seq);
}


void print_command_buffer()
{
  char *com;
  buffer_write(combuffer, "", 1);
  buffer_free(combuffer, (void**) &com, NULL);
  puts(com);
  free(com);
  combuffer= NULL;
}

void coprint(const char *fmt, ...)
{
  char buf[2000];
  va_list ap;
  va_start(ap,fmt);
  vsnprintf(buf, 1999, fmt, ap);
  buf[1999]= 0;
  va_end(ap);
  buffer_write(combuffer, buf, strlen(buf));
}


void print_bgrects()
{
  print_decor("bgrects", "set_bgrects", &bgbuffer, &nbg);
  print_decor("underlines", "set_underlines", &underbuffer, &nunder);
  print_decor("verboxes", "set_vboxes", &verbox, &nverbox);
}

void print_decor(char *varname, char *method, buffer_t **Rbuffer, int *Rn)
{
  buffer_t *buffer;
  int n;
  unsigned int *B;
  int i;

  buffer= *Rbuffer;
  n= *Rn;
  if (!n)
  {
     printf("  canvas->%s(NULL, 0);\n", method);
     return ;
  }

  buffer_free(buffer, (void**) &B, NULL);
  *Rbuffer= NULL;
  *Rn= 0;
  printf("static const unsigned int %s[]= {\n",varname);
  for(i=0;i<n*3;i++) printf("0x%x,", B[i]);
  printf("};\n");
  printf("  canvas->%s(%s, %d);\n", method, varname,n);
  free(B);
}


void record_decor(int style, int cbegin, int cend)
{
  if ((style>>16) &0xff)
     record_decor_entry(&bgbuffer, &nbg, cbegin, cend, (style>>16)&0xff);

  if ((style>>24) &1)
     record_decor_entry(&underbuffer, &nunder, cbegin, cend, (style>>8)&0xff);
}

void record_decor_entry(buffer_t **Rbuffer, int *N, int B,int E, int C)
{
   buffer_t *buf;
   buf= *Rbuffer;
   if (!buf) buf= buffer_new(128);
   buffer_write(buf, &B, sizeof(B));
   buffer_write(buf, &E, sizeof(E));
   buffer_write(buf, &C, sizeof(C));
   (*N)++;
   *Rbuffer= buf;
}


void output_doc()
{
  docnode_t *N,*secstart;
  elt_t *E,*Z;
  int start;
  int bg,font;
  int i;
  unsigned int style;

  check_first_node();

  N= doc->next;
next_page:
  secstart= N;
  if (N==doc) return ;
  page_init();

next_node:

  if (N->kind=='S' && N!=secstart && N->level>2)
  {
    // page ends here.
    goto print_stuff;
  }

  if (N->kind=='S' && N!=secstart)
  {
    N->s_eltstart= seqlen;
    N->s_parentno= secstart->s_number;
  }
  
  // do the regular outputting.
  output_node(N);
  N= N->next;
  if (N==doc) goto print_stuff;
  goto next_node;

print_stuff:
  print_page_functions(secstart);
  goto next_page;
}

void print_page_functions(docnode_t *secstart)
{
  printf("static void tf_%d(FYGfxCanvas* canvas) {\n",secstart->s_number);
  print_table_coord_variables();
  printf("canvas->reset_doc();\n");

  print_command_buffer();

  printf(" canvas->end_doc(tf_%d);\n",secstart->s_number);
  printf("}\n");

  printf("static void cf_%d(FYHelpWin *win) {\n",secstart->s_number);
  printf("  FYGfxCanvas *canvas= win->canvas;\n");

  print_seq_buffer();

  print_bgrects();
  print_imagedefs();
  print_links();

  print_section_menu(secstart);
  printf("  tf_%d(canvas);\n",secstart->s_number);
  printf("}\n");
}

void output_node(docnode_t *N)
{
  elt_t *E,*Z;
  int start;
  int bg,font;
  int i;
  unsigned int style;

  switch(N->kind)
  {
  case 'S': font= N->level;
            if (font>4) font= 4;
            section_style(N->level, &style, &font);
            start= seqlen;
            for(E=N->elts->next;E!=N->elts;E=E->next)
              output_elt(style, lookup_word(font, E->txt));
            coprint("  canvas->println(%d,%d);\n", start,seqlen-1);
            break;
  case 'P': 
            coprint("  canvas->pbreak();\n");
            start= seqlen;
            output_seq(0,N->elts,N);
            coprint("  canvas->println(%d,%d);\n", start,seqlen-1);
            break;
  case 'I': output_image(N); break;
  case 'L': output_list(N); break;
  case 'V': output_verbatim(N->vcmd, N->vlines); break;
  case 'T': output_table(N->t_nrows, N->t_ncols, 
                         N->t_ncells, N->t_cells,N); break;
  }
}


void print_link_table()
{
  docnode_t *N;
  printf("typedef struct { void (*func)(FYHelpWin*); int eltno; } sec_t; \n");
  printf("static sec_t sections[]= {\n");
  for(N=doc->next;N!=doc;N=N->next)
  {
    if (N->kind!='S') continue;
    if (N->level>2)
      printf(" { cf_%d, 0 },\n", N->s_number);
    else
      printf(" { cf_%d, %d},\n", N->s_parentno, N->s_eltstart);
  }
  printf("};\n");
}


void print_section_menu(docnode_t *secstart)
{
  docnode_t *N;
  if (secstart->kind!='S') return ;

  if (secstart->level==4) 
  {
     printf("  win->set_section_menu(secmen_%d);\n",secstart->s_number);
     return ;
  }

  if (secstart->level!=3) return ; // this should never happen

  for(N=secstart->prev;N!=doc;N=N->prev)
    if (N->kind=='S' && N->level==4) break;
  if (N==doc) return ;

  printf("  win->set_section_menu(secmen_%d);\n",N->s_number);
}

void output_table(int nrows,int ncols, int ncells, 
                  cell_t *cells,docnode_t *ref)
{
  int i,j,N,K;
  int *Estart, *Eend;
  int s,sr, er;
  coprint(" /// BEGIN TABLE \n {\n");

  // define width and height
  Estart= malloc(sizeof(int)*ncells);
  Eend= malloc(sizeof(int)*ncells);

  for(i=0;i<ncells;i++)
  {
    Estart[i]= seqlen;
    output_seq(0, cells[i].seq,ref);
    Eend[i]= seqlen-1;
    coprint("  int cw%d, ch%d,ca%d;\n", i,i,i);
    coprint("  canvas->get_seq_metrics(%d,%d, &cw%d, &ch%d, &ca%d);\n",
                      Estart[i], Eend[i], i,i,i);
  }

  for(i=0;i<nrows;i++)
    coprint(" int rh%d=0, ra%d=0;\n", i,i);

  for(i=0;i<nrows;i++)
    for(j=0;j<ncells;j++)
      if (cells[j].row==i)
         coprint("  rh%d= max_i(rh%d, ch%d); ra%d= max_i(ra%d, ca%d);\n",
                       i, i, j, i, i, j );
  for(i=0;i<ncols;i++) coprint("  int cow%d=0;\n", i);

  for(i=0;i<ncols;i++) 
    for(j=0;j<ncells;j++)
       if (cells[j].colspan==1 && cells[j].col==i)
          coprint("  cow%d= max_i(cow%d, cw%d);\n", i,i,j);
  coprint("  int D=0;\n");
  for(N=2;N<=ncols;N++)
     for(j=0;j<ncells;j++)
       if (cells[j].colspan==N)
       {
         coprint("  D= cw%d ", j);
         for(i=0;i<cells[j].colspan;i++)
            coprint(" - cow%d", i+cells[j].col);
         coprint(";\n");
         coprint("  if (D>0) { \n");
         for(i=0;i<cells[j].colspan-1;i++)
         {
            int K= cells[j].colspan-i;
            coprint("    cow%d += D / %d; D -= D / %d;\n",
                      cells[j].col+i, K, K);
         }
         coprint("    cow%d += D; }\n", cells[j].col+i);
       }
  for(i=0;i<=nrows;i++) coprint("  int rp%d;\n", i);
  for(i=0;i<=ncols;i++) coprint("  int cp%d;\n", i);
  coprint("  cp0= rp0= 0;\n");
  for(i=1;i<=nrows;i++) coprint("  rp%d= rp%d + 2*CELL_PAD + rh%d;\n",
                                      i, i-1, i-1);
  for(i=1;i<=ncols;i++) coprint("  cp%d= cp%d + 2*CELL_PAD + cow%d;\n",
                                      i, i-1, i-1);

  coprint("  int off_x= canvas->get_width() - cp%d;\n", ncols);
  coprint("  if (off_x<0) off_x= 0; else off_x/= 2;\n");

  for(i=0;i<ncells;i++)
    coprint("  canvas->%s(off_x+cp%d+CELL_PAD, off_x+cp%d-CELL_PAD, "
            "rp%d+CELL_PAD, rh%d, ra%d, %d, %d);\n",
                       cell_just_func(cells[i].fmt),
                                cells[i].col,
                                       cells[i].col+cells[i].colspan,
             cells[i].row,
                   cells[i].row,
                         cells[i].row,
                               Estart[i],
                                   Eend[i]);

  coprint("  int off_y= canvas->get_cursor_y();\n");

  table_draw_row_lines(nrows, ncols);
  table_draw_col_lines(nrows, ncols, ncells, cells);
  coprint("  canvas->xcoord(cp%d+3);\n", ncols);

  coprint("  canvas->advance_y(rp%d+2);\n", nrows);
  coprint(" /// END TABLE \n }\n");
  free(Estart); free(Eend);
}



const char *cell_just_func(char *fmt)
{
  if (!fmt) return "cell_left";
  if (strchr(fmt,'R')) return "cell_right";
  if (strchr(fmt,'C')) return "cell_center";
  return "cell_left";
}


void print_table_coord_variables()
{
  if (n_table_row_lines)
    printf("  int *TL_ROW= canvas->get_row_lines(%d);\n", n_table_row_lines);
  else
    printf(" canvas->get_row_lines(0); \n");
  if (n_table_col_lines)
    printf("  int *TL_COL= canvas->get_col_lines(%d);\n", n_table_col_lines);
  else
    printf(" canvas->get_col_lines(0); \n");
}


void table_draw_row_lines(int nrows, int ncols)
{
  int i,N;
  for(i=0;i<=nrows;i++)
  {
    N= n_table_row_lines;
    coprint("  TL_ROW[%d]= off_y+rp%d; "
              "TL_ROW[%d]= off_x+cp%d; "
              "TL_ROW[%d]= off_x+cp%d;\n",
                    3*N, i,       
                    3*N+1, 0,
                    3*N+2,  ncols);
    n_table_row_lines++;
  }
}


void table_draw_col_lines(int nrows, int ncols, int ncells, cell_t *cells)
{
  int i,j, s, sr,er,N;
  tricor_y *TA;
  int nT;

  TA= malloc(sizeof(*TA)*(ncols+1)*(nrows+1));
  nT= 0;

  i= -1;
nextcol:
  i++;
  if (i>ncols) goto donecol;

  s= 0;
  sr= 0;
nextseg:
  for(j=s;j<ncells;j++)
    if (i>cells[j].col && i<cells[j].col+cells[j].colspan) 
       break;
  if (j==ncells) er= nrows;
     else        er= cells[j].row;

  if (sr!=er)
  {
    TA[nT].y1= sr;
    TA[nT].y2= er;
    TA[nT].x=  i; 
    nT++;
  }
  if (j==ncells) goto nextcol;
  sr= er+1;
  s= j+1;
  goto nextseg;
donecol:
  sort_tricor_y(TA, nT);
  for(i=0;i<nT;i++)
  {
    N= n_table_col_lines;
    coprint("  TL_COL[%d]= off_y+rp%d; "
              "TL_COL[%d]= off_y+rp%d; "
              "TL_COL[%d]= off_x+cp%d;\n",
                  3*N, TA[i].y1,       
                  3*N+1, TA[i].y2, 
                  3*N+2, TA[i].x );
    n_table_col_lines++;
  }
  free(TA);
}


void sort_tricor_y(tricor_y *A, int n)
{
  qsort(A, n, sizeof(A[0]), compar_tricor_y);
}

int compar_tricor_y(const void *A, const void *B)
{
  const tricor_y *a= A, *b= B;
  int r;
  r= a->y1 - b->y1; if (r) return r;
  r= a->y2 - b->y2; if (r) return r;
  return a->x  - b->x;  
}


void parse_image()
{
  char *cmd,*e;
  int i;
  docnode_t *N;
  remove_leading_spc(line);
  if (line[0]!='[') exit(fprintf(stderr,"image: internal error 1\n"));
  remove_part(line, 0,1);
  remove_leading_spc(line);
  cmd= "image";
  if (!strpfx(cmd, line)) exit(fprintf(stderr,"image: internal error 2\n"));
  remove_part(line, 0,strlen(cmd));
  remove_leading_spc(line);
  for(i=0;line[i] && !ispc(line[i]) &&line[i]!=']';i++) ;
  line[i]= 0;
  N= calloc(1,sizeof(*N));
  N->i_path= relapath(file_name, line);
  N->kind= 'I';
  e= get_png_dimensions(N->i_path, &N->i_width, &N->i_height);
  if (e)
    exit(fprintf(stderr,"can't get dimensions "
                        "for image %s: %s\n", N->i_path,e));
  add_node(N);
}

char *relapath(char *base, char *name)
{
  int L;
  char buf[2000];
  L= strlen(base)-1;
  while(L>=0 && base[L]!='/') L--;
  if (L<0) return strdup(name);
  base= strdup(base);
  base[L]= 0;
  sprintf(buf,"%s/%s", base,name);
  free(base);
  return strdup(buf);
}


void print_image_file(int num, char *path)
{
  FILE *f;
  int k,i,j;
  unsigned char buf[2000];
  f= fopen(path,"rb");
  if (!f) exit(fprintf(stderr,"can't open %s: %s\n", path, strerror(errno)));
  j= 0;
  printf("static const unsigned char img%ddata[]={\n",num);
  while(k=fread(buf,1,sizeof(buf), f))
    for(i=0;i<k;i++,j++)
    {
      if (j%12==0) printf("\n");
      printf("%d,",buf[i]);
    }
  printf("\n};\n");
  fclose(f);
}


void output_image(docnode_t *N)
{
  imageref_t *R;

  R= malloc(sizeof(*R));
  R->number= inum++;
  R->path= N->i_path;
  R->next= NULL;
  R->width= N->i_width;
  R->height= N->i_height;
  *imgtail= R;
  imgtail= &R->next;

  coprint("  canvas->image(%d);\n",R->number);
}

void print_imagedefs()
{
  imageref_t *R;

  if (!inum)
  {
    printf("  canvas->set_images(NULL,0);\n");
    return ;
  }

  for(R=imgs;R;R=R->next)
     print_image_file(R->number, R->path);

  printf("  static imagedef_t images[]= {\n");
  for(R=imgs;R;R=R->next)
     printf(" {%d, %d, img%ddata},\n", R->width, R->height, R->number);
  printf("};\n");
  printf("  canvas->set_images(images, %d);\n",inum);
}


void get_chapter_menu()
{
  docnode_t *N;
  buffer_t *buf;
  buf= buffer_new(1024);
  for(N=doc->next;N!=doc;N=N->next)
    if (N->kind=='S' && N->level==4)
      buffer_printv(buf,'s', " win->menu_add_chapter(", 
                        's', N->s_title_encoded,
                        's', ", ",
                        'I', N->s_number,
                        's', ");\n" );
  buffer_write(buf, "",1);
  buffer_free(buf, (void**) &chapter_menu, NULL);
}


void check_first_node()
{
  docnode_t* N;
  N= doc->next;

  if (N->kind!='S' || N->level!=4)
    exit(fprintf(stderr,"Document contains text without a chapter "
                        "header at the very start. Aborting.\n"));
}


void calculate_section_titles()
{
  docnode_t *N;

  for(N=doc->next;N!=doc;N=N->next)
    if (N->kind=='S')
    {
      N->s_title= collect_seq_str(N->elts);
      N->s_title_encoded= encode_str(N->s_title);
    }
}


char *collect_seq_str(elt_t *elts)
{
  buffer_t *B;
  char *str;
  elt_t *E;

  B= buffer_new(128);
  for(E=elts->next;E!=elts;E=E->next)
  {
     if (E!=elts->next) buffer_write(B, " ",1);
     buffer_write(B, E->txt, strlen(E->txt));
  }
  buffer_write(B, "",1);
  buffer_free(B, (void**) &str, NULL);
  return str;
}



void print_section_menu_arrays()
{
  docnode_t *N;
  int st;
  st= 0;
  for(N=doc->next;N!=doc;N=N->next)
  {
    if (N->kind!='S') continue;
    switch(N->level)
    {
       case 4: if (st) printf("{NULL,0} };\n");
               st= 1;
               printf("static const sipair_t secmen_%d[]= {\n", N->s_number);
               break;
       case 3: printf("{%s, %d},\n", N->s_title_encoded, N->s_number);
               break;
    } 
  }
  if (st) printf("{NULL, 0} };\n");
}


void buffer_printv(buffer_t *buffer, int fmt, ...)
{
  int iarg; char *sarg; char buf[30];
  va_list ap;
  va_start(ap,fmt);

  while(fmt!=0)
  {
    switch(fmt)
    {
     case 's': sarg= va_arg(ap, char*);
               buffer_write(buffer, sarg, strlen(sarg));
               break;
     case 'I': iarg= va_arg(ap,int);
               sprintf(buf,"%d", iarg);
               buffer_write(buffer, buf, strlen(buf));
               break;
     default:  goto done;
    }
    fmt= va_arg(ap, int);
  }
done:
  va_end(ap);
}

unsigned char *encode_str(unsigned char *src)
{
  char *buf;
  int len,i;
  buf= NULL;

again:
  len= 1;
  for(i=0;src[i];i++)
    if (isalnum(src[i]))
    { if (buf) buf[len]= src[i];
      len++; } else { if (buf) sprintf(buf+len, "\\%03o", src[i]);
                      len+= 4; }
  if (buf) { buf[len]= '"'; buf[len+1]= 0; return buf; }
  buf= malloc(len + 2);
  buf[0]= '"';
  goto again;
}

void remove_all_spaces(unsigned char *z)
{
  unsigned char *t;
  for(t=z;*z;z++) if (!ispc(z[0])) *(t++)= *z;
  *t= 0;
}
void collect_anchors()
{
  int i;
  aanchor_t T;
  docnode_t *A,*N,*Z;
  buffer_t *buf;

  buf= buffer_new(1024);
  for(A=doc->next;A!=doc;A=Z)
  {
    Z= A->next;
    if (A->kind!='A') continue;
    
      for(N=A->next;N!=doc;N=N->next)
         if (N->kind=='S') break;
      if (N==doc)
         exit(fprintf(stderr,"error near %s:%d: anchor isn't followed by a "
                             "section heading.\n", A->fname, A->lno));
      T.secnum= N->s_number;
      T.lno= A->lno;
      T.fname= A->fname;
      T.name= A->a_txt;
      T.node= N;
      buffer_write(buf, &T, sizeof(T));
      n_anchors++;

     A->prev->next= A->next;
     A->next->prev= A->prev;
  }
  buffer_free(buf, (void**) &anchors, NULL);
  if (n_anchors)
     qsort(anchors, n_anchors, sizeof(anchors[0]), anchor_compare);

  for(i=0;i<n_anchors-1;i++)
    if (!strcmp(anchors[i].name,anchors[i+1].name))
       exit(fprintf(stderr,"multiple definition of anchor %s:\n" 
                           " 1- near %s: %d\n",
                           " 2- near %s: %d\n",
                              anchors[i].name, 
                         anchors[i].fname, anchors[i].lno,
                         anchors[i+1].fname, anchors[i+1].lno));
}

int anchor_compare(const void *a, const void *b)
{
  const aanchor_t *A= a;
  const aanchor_t *B= b;
  return strcmp(A->name, B->name);
}

void print_anchors()
{
  int i;
  printf("typedef struct { const char *name; int secno; } anchor_t; \n");
  printf("static const anchor_t anchors[]= {\n");
  for(i=0;i<n_anchors;i++)
  {
     printf("{ "); print_str(anchors[i].name); 
                   printf(", %d },\n", anchors[i].secnum);
  }
  printf("};\n");
}


void parse_anchor()
{
  int i;
  char *ach;
  docnode_t *N;
  remove_leading_spc(line);
  if (line[0]!='[') 
     exit(fprintf(stderr, "internal error at parse_anchor :1\n"));
  remove_part(line,0,1);
  remove_leading_spc(line);
  ach= "anchor";
  if (!strpfx(ach, line)) 
     exit(fprintf(stderr, "internal error at parse_anchor :2\n"));
  remove_part(line, 0, strlen(ach));
  if (!skipspc(line,0))
     exit(fprintf(stderr, "internal error at parse_anchor :3\n"));
  remove_leading_spc(line);
  for(i=0;line[i] && line[i]!=']';i++) ;
     
  if (!line[i]) 
     exit(fprintf(stderr, "near %s: %d: unterminated anchor directive\n",
                           file_name, line_number));
  N= calloc(1,sizeof(*N));
  N->kind= 'A';
  N->a_txt= get_part(line, 0,i);
  add_node(N);
}


void output_link(elt_t *linkel,docnode_t *ref)
{
  elt_t *E,*elts;
  aanchor_t *A;
  int font,style;
  int start,end;
  int q;
  char *arg;
  char *K,*L,*tgt;
  buffer_t *B;

  K= strstr(linkel->arg, "%{");
  if (!K) K= strstr(linkel->arg,"%(");
  if (!K) 
     exit(fprintf(stderr,"link contains no target near %s:%d\n",
                        ref->fname, ref->lno));

  if (K[1]=='{') q='}'; else q=')';
  for(L=K+2;L[0];L++)
    if (L[0]==q) break;

  if (!L[0])
     exit(fprintf(stderr,"unterminated link target near %s:%d\n",
                        ref->fname, ref->lno));

  tgt= get_part(linkel->arg, K+2-linkel->arg, L-linkel->arg);
  remove_all_spaces(tgt);
  if (!tgt[0])
     exit(fprintf(stderr,"empty link target near %s:%d\n",
                        ref->fname, ref->lno));

  A= find_target(tgt);
  if (!A)
     exit(fprintf(stderr,"can't find link target %s near %s:%d\n",
                        tgt, ref->fname, ref->lno));

  B= buffer_new(128);
  buffer_write(B, linkel->arg, K-linkel->arg);
  if (q!='}')
    buffer_write(B, A->node->s_title, strlen(A->node->s_title));
  buffer_write(B, L+1, strlen(L+1)+1);
  buffer_free(B, (void**) &arg, NULL);

  elts= parse_seq(arg, 0);
  if (elts->next==elts) 
     exit(fprintf(stderr,"empty link text near %s:%d\n",
                           ref->fname, ref->lno));

  inline_style("link", &style, &font);
  start= seqlen;
  for(E=elts->next;E!=elts;E=E->next)
     output_elt(style, lookup_word(font, E->txt));
  end= seqlen - 1;
  record_decor(style, start, end);
  record_link(start, end, A->secnum);
}


void record_link(int start, int end, int num)
{
  if (!linkbuf) linkbuf= buffer_new(128);
  buffer_write(linkbuf, &start, sizeof(start));
  buffer_write(linkbuf, &end, sizeof(end));
  buffer_write(linkbuf, &num, sizeof(num));
  n_links++;
}

void print_links()
{
  unsigned int *L;
  int i;
  if (!linkbuf) { printf("  canvas->set_links(NULL,0);\n"); return ; }
  buffer_free(linkbuf, (void**) &L, NULL);
  printf("static const unsigned int links[]= {\n");
  for(i=0;i<3*n_links;i++) printf("0x%x,", L[i]);
  printf("};\n");
  printf("  canvas->set_links(links, %d);\n", n_links);
  free(L);
  n_links= 0;
  linkbuf= NULL;
}


aanchor_t *find_target(char *tgt)
{
  int i,j,k,R;
  i= 0; j= n_anchors-1;
  while(i<=j)
  {
     k= (i+j)/2; 
     R= strcmp(anchors[k].name, tgt);
     if (R<0) i= k+1;
     else if (R>0) j= k-1;
     else return anchors+k;
  }
  return NULL;
}


int lookup_word(int font, const char *str)
{
   word_record_t *V;
   int *N;
   int nv,i;
   V= strmap_find(&word_list, (char*) str);
   if (!V)
   {
     V= malloc(sizeof(*V));
     V->str= strdup(str);
     V->val= malloc(sizeof(int)*2);
     strmap_insert(&word_list, strdup(str), V);
     V->n_val= 2; V->val[0]= font; V->val[1]= ++last_word;
     return V->val[1];
   }
   for(i=0;i<V->n_val;i+=2)
     if (V->val[i]==font) return V->val[i+1];
   nv= V->n_val;
   N= malloc(sizeof(int)*(nv+2));
   memcpy(N, V->val, nv*sizeof(int));
   N[nv]= font;
   N[nv+1]= ++last_word;
   free(V->val);
   V->val= N;
   V->n_val+= 2;
   return N[nv+1];
}


char **get_word_list()
{
   char **R;
   word_record_t *W;  
   int i;
  
   R= malloc((last_word+2)*sizeof(char*));
   R[0]= "";
   while((W=strmap_destroy(&word_list)))
   {
     for(i=0;i<W->n_val;i+=2)
       R[W->val[i+1]]= W->str;
     free(W->val); free(W);
   }
   R[last_word+1]= NULL;
   return R;
}




int main(int argc,char **argv)
{
  parse_file(argv[1]);
//  print_docnodelist(doc);
  puts(libhdr); puts(libcode);

  calculate_section_titles();
  collect_anchors();
  print_anchors();
  
  check_first_node();
  get_chapter_menu();
  printf(
"static void init_doc(FYHelpWin *win)\n"
"{\n"
"  %s\n"
"  win->canvas->link_func= follow_link;\n"
"}\n"

,chapter_menu);

  print_section_menu_arrays();
  output_doc();
  print_init_words();
  print_link_table();
puts(
"\n"
"/* This file defines the glue code between the application and the\n"
"   help system. It creates the help window, initializes it etc. \n"
"   The contents here are supposed to go after the document functions\n"
"   are defined. */\n"
"\n"
"static FYHelpWin *help_window;\n"
"\n"
"static void create_help_window()\n"
"{\n"
"  if (help_window) return ;  // FIXME: raise it somehow\n"
"  help_window= new FYHelpWin( FXApp::instance(),\n"
"                              \042Help\042, NULL, NULL, // FIXME: icons\n"
"                              DECOR_ALL , 640, 600);\n"
"  help_window->show(PLACEMENT_SCREEN);\n"
"  help_window->create();\n"
"  init_words(help_window->canvas);\n"
"  init_doc(help_window);\n"
"}\n"
"\n"
"static void follow_link(int linkno)\n"
"{\n"
"  int ltsize;\n"
"  // printf(\042following link %d\134n\042, linkno);\n"
"  ltsize= sizeof(sections)/sizeof(sections[0]);\n"
"  if (linkno<0 || linkno>=ltsize) return ;\n"
"\n"
"  (*(sections[linkno].func))(help_window);\n"
"  help_window->update();\n"
"}\n"
"\n"
"void help(const char *subject)\n"
"{\n"
"  int i,j,k,R;\n"
"  create_help_window(); \n"
"  i=0; j=sizeof(anchors)/sizeof(anchors[0])-1;\n"
"  while(i<=j)\n"
"  {\n"
"     k= (i+j)/2;\n"
"     R= strcmp(anchors[k].name, subject);\n"
"     if (R<0) i= k+1;\n"
"     else if (R>0) j= k-1;\n"
"     else break;\n"
"  }\n"
"  if (i>j) cf_0(help_window);\n"
"  (*sections[anchors[k].secno].func)(help_window);\n"
"}\n"
"\n"

  );
  return 0;
}
