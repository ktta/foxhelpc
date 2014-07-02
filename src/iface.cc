
/* This file defines the glue code between the application and the
   help system. It creates the help window, initializes it etc. 
   The contents here are supposed to go after the document functions
   are defined. */

static FYHelpWin *help_window;

static void create_help_window()
{
  if (help_window) return ;  // FIXME: raise it somehow
  help_window= new FYHelpWin( FXApp::instance(),
                              "Help", NULL, NULL, // FIXME: icons
                              DECOR_ALL , 640, 600);
  help_window->show(PLACEMENT_SCREEN);
  help_window->create();
  init_words(help_window->canvas);
  init_doc(help_window);
}

static void follow_link(int linkno)
{
  int ltsize;
  // printf("following link %d\n", linkno);
  ltsize= sizeof(sections)/sizeof(sections[0]);
  if (linkno<0 || linkno>=ltsize) return ;

  (*(sections[linkno].func))(help_window);
  help_window->update();
}

void help(const char *subject)
{
  int i,j,k,R;
  create_help_window(); 
  i=0; j=sizeof(anchors)/sizeof(anchors[0])-1;
  while(i<=j)
  {
     k= (i+j)/2;
     R= strcmp(anchors[k].name, subject);
     if (R<0) i= k+1;
     else if (R>0) j= k-1;
     else break;
  }
  if (i>j) cf_0(help_window);
  (*sections[anchors[k].secno].func)(help_window);
}

