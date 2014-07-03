/*
  The help window will contain the following:
   - a menu bar for navigation
   - a toolbar for displaying buttons such as prev,next, search, hprev, etc.
   - a canvas to display the help text
*/


namespace FX
{

typedef struct { const char *Vs; int Vi; } sipair_t;

class FXAPI FYHelpWin : public FXTopWindow
{
  FXDECLARE(FYHelpWin)
protected:
  FYHelpWin();
private:
  FYHelpWin(const FYHelpWin&);
  FYHelpWin &operator=(const FYHelpWin&);
public:
  FYHelpWin (FXApp* ap,const FXString& name, FXIcon *ic,FXIcon *mi,
            FXuint opts, FXint w,FXint h,
            FXint x=0,FXint y=0,
            FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,
            FXint hs=0,FXint vs=0) ;
  virtual ~FYHelpWin();

// uu-chapsec
/* Chapters and sections will be accessible thru menus. */
private:
  FXMenuPane *chapters_pane;
  FXMenuPane *sections_pane;
  FXMenuTitle *sections_mtitle;

/* The chapter menu will never change. We don't need to keep track of
   that stuff. We simply provide a method to record them into the menu. */

public:  void menu_add_chapter(const char *title, int number);

/* The section menu will be set as a whole, rather than incremental 
   method calls. */
private: const sipair_t *sections_def;
public:  void set_section_menu(const sipair_t *);

public:
  FYGfxCanvas *canvas;
};
}

using namespace FX;

static void follow_link(int);
static void show_toc2(int);
static void show_toc0(int);


FXDEFMAP(FYHelpWin) FYHelpWinMap[]=
{
};


FXIMPLEMENT(FYHelpWin,FXTopWindow,FYHelpWinMap, ARRAYNUMBER(FYHelpWinMap))

FYHelpWin::FYHelpWin(){ flags|=FLAG_ENABLED|FLAG_SHOWN; }


FYHelpWin::FYHelpWin
           (FXApp* ap,const FXString& name, FXIcon *ic,FXIcon *mi,
            FXuint opts, FXint w,FXint h,
            FXint x,FXint y,
            FXint pl,FXint pr,FXint pt,FXint pb,
            FXint hs,FXint vs) :
 FXTopWindow(ap,name,ic,mi, opts, x,y,w,h, pl,pr,pt,pb, hs,vs)
{
  FXVerticalFrame *vbox;
  flags|=FLAG_ENABLED|FLAG_SHOWN; 

  vbox= new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);

  FXMenuBar *bar;
  bar= new FXMenuBar(vbox, LAYOUT_FILL_X);

  chapters_pane= new FXMenuPane(this);
  sections_pane= new FXMenuPane(this);

  new FXMenuTitle(bar,"Chapters", NULL, chapters_pane);
  sections_mtitle= new FXMenuTitle(bar,"Sections", NULL, sections_pane);

  new FYMenuCommand(bar, "Contents", NULL, 0, show_toc2, 0);
  new FYMenuCommand(bar, "Full", NULL, 0, show_toc0, 0);

  canvas= new FYGfxCanvas(vbox, FRAME_NORMAL | LAYOUT_FILL_X | LAYOUT_FILL_Y);
  
  canvas->define_font(0, "Dejavu Sans", 12, "");
  canvas->define_font(4, "Dejavu Sans", 24, "B");
  canvas->define_font(3, "Dejavu Sans", 20, "B");
  canvas->define_font(2, "Dejavu Sans", 16, "B");
  canvas->define_font(1, "Dejavu Sans", 13, "B");
  canvas->define_font(5, "Mono", 12, "");
  canvas->define_font(6, "Mono", 12, "");

  canvas->define_color(0, 0);
  canvas->define_color(1, 0xff0000);
  canvas->define_color(2, 0x00ff00);
  canvas->define_color(3, 0x0000ff);
 
//uu-cons
}


FYHelpWin::~FYHelpWin()
{
// uu-des
}

// uu-chapsec
void FYHelpWin::menu_add_chapter(const char *title, int linkno)
{
  FYMenuCommand *cmd;
  cmd= new FYMenuCommand(chapters_pane, title, NULL, 0, follow_link, linkno);
  cmd->create();
}


void FYHelpWin::set_section_menu(const sipair_t *me)
{
  int i;
  FYMenuCommand *cmd;
  if (sections_def==me) return ;
  FXWindow *ww;
  while((ww=sections_pane->getFirst()))
    delete ww;
  sections_def= me;
  for(i=0;me[i].Vs;i++)
  {
     cmd= new FYMenuCommand(sections_pane, me[i].Vs, 
                            NULL, 0, follow_link, me[i].Vi);
     cmd->create();
  }
  if (i==0) 
    sections_mtitle->disable();
  else
    sections_mtitle->enable();
  sections_pane->recalc();
}

